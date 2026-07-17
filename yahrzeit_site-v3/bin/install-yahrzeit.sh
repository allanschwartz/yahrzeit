#!/usr/bin/env bash
set -euo pipefail

# Install/update the CBS Yahrzeit Wall PHP appliance on a small Ubuntu/Debian host.
#
# Run locally on a fresh Ubuntu/Debian appliance:
#   curl -fsSL https://raw.githubusercontent.com/allanschwartz/yahrzeit/master/yahrzeit_site-v3/bin/install-yahrzeit.sh -o /tmp/install-yahrzeit.sh
#   chmod +x /tmp/install-yahrzeit.sh
#   /tmp/install-yahrzeit.sh
#
# This script installs packages, clones or updates the repo, configures Apache,
# runs syntax/audit checks, and installs the managed lighting schedule.
# It does not transmit to the controller.

REPO_URL="${REPO_URL:-https://github.com/allanschwartz/yahrzeit.git}"
BRANCH="${BRANCH:-master}"
SITE_SUBDIR="${SITE_SUBDIR:-yahrzeit_site-v3}"
INSTALL_PARENT="${INSTALL_PARENT:-$HOME/src}"
REPO_DIR="${REPO_DIR:-$INSTALL_PARENT/yahrzeit}"
SITE_DIR="${SITE_DIR:-$REPO_DIR/$SITE_SUBDIR}"
WEB_ALIAS="${WEB_ALIAS:-yahrzeit}"
WEB_LINK="${WEB_LINK:-/var/www/html/$WEB_ALIAS}"
CRON_USER="${YAHRZEIT_CRON_USER:-${SUDO_USER:-$(id -un)}}"
CRON_WRAPPER="/usr/local/sbin/yahrzeit-fix-crontab"
INSTALL_USER="$(id -un)"
INSTALL_GROUP="$(id -gn)"

printf 'Installing/updating CBS Yahrzeit appliance software\n\n'
printf 'Repo:       %s\n' "$REPO_URL"
printf 'Branch:     %s\n' "$BRANCH"
printf 'Repo dir:   %s\n' "$REPO_DIR"
printf 'Site dir:   %s\n' "$SITE_DIR"
printf 'Web link:   %s\n\n' "$WEB_LINK"
printf 'Cron user:  %s\n\n' "$CRON_USER"

if ! command -v apt-get >/dev/null 2>&1; then
    echo "ERROR: this installer expects an Ubuntu/Debian apt-based system" >&2
    exit 1
fi

sudo apt-get update
sudo apt-get install -y \
    git \
    curl \
    ca-certificates \
    openssh-server \
    apache2 \
    php \
    libapache2-mod-php \
    php-cli \
    php-common \
    php-mbstring \
    php-xml \
    netcat-openbsd \
    net-tools \
    tcpdump \
    coreutils \
    cron \
    logrotate \
    sudo

# Ensure SSH is enabled and running
sudo systemctl enable ssh
sudo systemctl start ssh
sudo systemctl enable cron
sudo systemctl start cron

# Disable AppArmor (it can interfere with SSH and other services)
sudo systemctl disable apparmor || true
sudo systemctl stop apparmor || true

mkdir -p "$INSTALL_PARENT"

if [ ! -d "$REPO_DIR/.git" ]; then
    git clone --filter=blob:none --no-checkout "$REPO_URL" "$REPO_DIR"
    cd "$REPO_DIR"
    git sparse-checkout init --cone
    git sparse-checkout set "$SITE_SUBDIR"
    git checkout "$BRANCH"
else
    # Older installer versions made data/ entirely www-data-owned. Restore
    # repository ownership before Git attempts to replace tracked data files.
    sudo chown -R "$INSTALL_USER:$INSTALL_GROUP" "$REPO_DIR"

    cd "$REPO_DIR"

    # Remember only the previously selected controller address. The tracked
    # configuration and program defaults should otherwise update with Git.
    PREVIOUS_CONTROLLER_HOST=""
    for relative_path in \
        "$SITE_SUBDIR/bin/yahrzeit-controller.conf" \
        "$SITE_SUBDIR/bin/yahrzeit-controller"; do
        controller_config="$REPO_DIR/$relative_path"
        if [ -r "$controller_config" ]; then
            PREVIOUS_CONTROLLER_HOST="$(awk -F= '$1 == "CONTROLLER_HOST" { print substr($0, index($0, "=") + 1); exit }' "$controller_config")"
            if [ -n "$PREVIOUS_CONTROLLER_HOST" ]; then
                break
            fi
        fi
    done

    # The appliance checkout is a deployment, not a development worktree.
    # Preserve its live tracked data, repair interrupted/dirty code updates by
    # matching origin exactly, and restore the appliance-specific data files.
    LOCAL_FILES_TMP="$(mktemp -d)"
    restore_local_files() {
        for relative_path in \
            "$SITE_SUBDIR/data/minhag.ini" \
            "$SITE_SUBDIR/data/yahrzeits-rev4.csv"; do
            backup_path="$LOCAL_FILES_TMP/$relative_path"
            if [ -f "$backup_path" ]; then
                mkdir -p "$(dirname "$REPO_DIR/$relative_path")"
                cp -p "$backup_path" "$REPO_DIR/$relative_path"
            fi
        done
    }
    trap 'restore_local_files' EXIT

    for relative_path in \
        "$SITE_SUBDIR/data/minhag.ini" \
        "$SITE_SUBDIR/data/yahrzeits-rev4.csv"; do
        if [ -f "$REPO_DIR/$relative_path" ]; then
            mkdir -p "$(dirname "$LOCAL_FILES_TMP/$relative_path")"
            cp -p "$REPO_DIR/$relative_path" "$LOCAL_FILES_TMP/$relative_path"
        fi
    done

    git fetch origin "$BRANCH"
    if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
        git checkout -f "$BRANCH"
    else
        git checkout -f -b "$BRANCH" "origin/$BRANCH"
    fi
    git sparse-checkout init --cone || true
    git sparse-checkout set "$SITE_SUBDIR"
    git reset --hard "origin/$BRANCH"

    restore_local_files
    trap - EXIT
    rm -rf -- "${LOCAL_FILES_TMP:?}"
fi

if [ ! -d "$SITE_DIR" ]; then
    echo "ERROR: site directory not found: $SITE_DIR" >&2
    exit 1
fi

cd "$SITE_DIR"

CONTROLLER_CONFIG_FILE="bin/yahrzeit-controller.conf"
# shellcheck source=bin/yahrzeit-controller.conf
source "$CONTROLLER_CONFIG_FILE"
RECORDED_CONTROLLER_HOST="${PREVIOUS_CONTROLLER_HOST:-${CONTROLLER_HOST:-}}"
CONTROLLER_HOST_INPUT="${YAHRZEIT_CONTROLLER_HOST:-}"

if [ -z "$CONTROLLER_HOST_INPUT" ] && [ -t 0 ]; then
    printf 'CONTROLLER_HOST is recorded as %s.\n' "$RECORDED_CONTROLLER_HOST"
    printf 'Press Enter to keep it, or enter a new address: '
    IFS= read -r CONTROLLER_HOST_INPUT
fi

CONTROLLER_HOST_INPUT="${CONTROLLER_HOST_INPUT:-$RECORDED_CONTROLLER_HOST}"
if [[ ! "$CONTROLLER_HOST_INPUT" =~ ^[A-Za-z0-9._:-]+$ ]]; then
    echo "ERROR: invalid controller hostname or address: $CONTROLLER_HOST_INPUT" >&2
    exit 1
fi

if ! grep -q '^CONTROLLER_HOST=' "$CONTROLLER_CONFIG_FILE"; then
    echo "ERROR: CONTROLLER_HOST is missing from $CONTROLLER_CONFIG_FILE" >&2
    exit 1
fi

sed -i "s/^CONTROLLER_HOST=.*/CONTROLLER_HOST=$CONTROLLER_HOST_INPUT/" "$CONTROLLER_CONFIG_FILE"
chmod 644 "$CONTROLLER_CONFIG_FILE"
printf 'Controller: %s\n\n' "$CONTROLLER_HOST_INPUT"

mkdir -p data/backups
if [ -f data/scheduler.log ] && [ ! -e data/automation.log ]; then
    mv data/scheduler.log data/automation.log
fi
touch data/automation.log

chmod 755 bin/yahrzeit bin/yahrzeit_scheduler bin/yahrzeit_engine.php bin/fix-up-crontab

if ! id "$CRON_USER" >/dev/null 2>&1; then
    echo "ERROR: configured cron account does not exist: $CRON_USER" >&2
    exit 1
fi

# Record one authoritative crontab owner.
printf '%s\n' "$CRON_USER" | sudo tee /etc/yahrzeit-cron-user >/dev/null
sudo chmod 644 /etc/yahrzeit-cron-user

# The web server may invoke only this root-owned wrapper. It immediately drops
# privileges to the configured account, so the repository helper can modify
# only that account's own crontab.
WRAPPER_TMP="$(mktemp)"
trap 'rm -f "$WRAPPER_TMP"' EXIT
cat > "$WRAPPER_TMP" <<EOF_WRAPPER
#!/bin/sh
exec /usr/bin/sudo -u '$CRON_USER' '$SITE_DIR/bin/fix-up-crontab'
EOF_WRAPPER
sudo install -o root -g root -m 0755 "$WRAPPER_TMP" "$CRON_WRAPPER"
rm -f "$WRAPPER_TMP"
trap - EXIT

SUDOERS_TMP="$(mktemp)"
trap 'rm -f "$SUDOERS_TMP"' EXIT
printf 'www-data ALL=(root) NOPASSWD: %s\n' "$CRON_WRAPPER" > "$SUDOERS_TMP"
sudo visudo -cf "$SUDOERS_TMP"
sudo install -o root -g root -m 0440 "$SUDOERS_TMP" /etc/sudoers.d/yahrzeit-fix-crontab
rm -f "$SUDOERS_TMP"
trap - EXIT

# Apache follows the symlink only if it can traverse the parent directories.
# This grants execute/traverse only, not broad read access to the home tree.
chmod o+x "$HOME"
chmod o+x "$INSTALL_PARENT"
chmod o+x "$REPO_DIR"
chmod o+x "$SITE_DIR"

# Keep the installation account as owner so future Git updates can replace
# tracked files. Apache receives group-write access only to runtime data.
sudo chown -R "$INSTALL_USER":www-data "$SITE_DIR/data"
sudo find "$SITE_DIR/data" -type d -exec chmod 2775 {} +
sudo find "$SITE_DIR/data" -type f -exec chmod 664 {} +
sudo touch "$SITE_DIR/data/automation.log"
sudo chown "$CRON_USER":www-data "$SITE_DIR/data/automation.log"
sudo chmod 664 "$SITE_DIR/data/automation.log"

# Keep approximately three months of weekly automation history. Cron opens the
# log for each invocation, so normal rename/create rotation is safe.
LOGROTATE_TMP="$(mktemp)"
LOGROTATE_CONFIG="/etc/logrotate.d/cbs-yahrzeit-automation"
trap 'rm -f "$LOGROTATE_TMP"' EXIT
cat > "$LOGROTATE_TMP" <<EOF_LOGROTATE
"$SITE_DIR/data/automation.log" {
    weekly
    rotate 13
    compress
    dateext
    missingok
    notifempty
    create 0664 $CRON_USER www-data
    su $CRON_USER www-data
}
EOF_LOGROTATE
sudo install -o root -g root -m 0644 "$LOGROTATE_TMP" "$LOGROTATE_CONFIG"
sudo rm -f /etc/logrotate.d/cbs-yahrzeit-scheduler
rm -f "$LOGROTATE_TMP"
trap - EXIT
sudo logrotate --debug "$LOGROTATE_CONFIG" >/dev/null 2>&1

# Ubuntu hardens Apache with ProtectHome=read-only and hides sudoers files.
# Permit writes only to this application's runtime data, and let sudo read its
# configuration so the one explicitly authorized cron-repair wrapper works.
APACHE_OVERRIDE_DIR="/etc/systemd/system/apache2.service.d"
APACHE_OVERRIDE="$APACHE_OVERRIDE_DIR/yahrzeit.conf"
sudo mkdir -p "$APACHE_OVERRIDE_DIR"
sudo tee "$APACHE_OVERRIDE" >/dev/null <<EOF_APACHE_OVERRIDE
[Service]
ReadWritePaths=$SITE_DIR/data

# Reset the distribution list, then retain every inaccessible path except
# /etc/sudoers and /etc/sudoers.d, which sudo must read.
InaccessiblePaths=
InaccessiblePaths=/boot
InaccessiblePaths=/root
InaccessiblePaths=-/etc/ssh
InaccessiblePaths=-/etc/apt
InaccessiblePaths=-/etc/.git
InaccessiblePaths=-/etc/.svn
EOF_APACHE_OVERRIDE
sudo chmod 644 "$APACHE_OVERRIDE"
sudo systemctl daemon-reload

# Configure Apache: ensure php module is enabled and properly configured.
# Try the generic 'php' module first, then try specific PHP versions if it doesn't exist
for php_module in php php8.5 php8.4 php8.3 php8.2 php8.1 php8.0 php7.4; do
    sudo a2enmod "$php_module" 2>/dev/null && break
done || true
sudo a2enmod rewrite || true

# This appliance does not depend on public DNS. A stable global name prevents
# Apache from guessing an IPv4/IPv6 address and logging AH00558 on each restart.
sudo tee /etc/apache2/conf-available/yahrzeit-servername.conf >/dev/null <<'EOF_SERVERNAME'
ServerName localhost
EOF_SERVERNAME
sudo a2enconf yahrzeit-servername

# Create a simple index.html at root to redirect to /yahrzeit/
sudo tee /var/www/html/index.html > /dev/null <<REDIRECT_HTML
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="refresh" content="0;url=/$WEB_ALIAS/" />
    <title>Redirecting...</title>
</head>
<body>
    <p>Redirecting to <a href="/$WEB_ALIAS/">Yahrzeit Wall</a>...</p>
</body>
</html>
REDIRECT_HTML

# Create or update the symlink idempotently.
# First check if link exists and points to the correct location.
if [ -L "$WEB_LINK" ]; then
    CURRENT_TARGET="$(readlink "$WEB_LINK")"
    if [ "$CURRENT_TARGET" != "$SITE_DIR" ]; then
        printf 'Updating symlink %s from %s to %s\n' "$WEB_LINK" "$CURRENT_TARGET" "$SITE_DIR"
        sudo rm -f "$WEB_LINK"
        sudo ln -sfn "$SITE_DIR" "$WEB_LINK"
    else
        printf 'Symlink %s already correct\n' "$WEB_LINK"
    fi
elif [ -e "$WEB_LINK" ]; then
    # It exists but is not a symlink. Backup and replace.
    printf 'WARNING: %s exists but is not a symlink. Backing up and replacing.\n' "$WEB_LINK" >&2
    sudo mv "$WEB_LINK" "$WEB_LINK.backup-$(date +%s)"
    sudo ln -sfn "$SITE_DIR" "$WEB_LINK"
else
    # Doesn't exist, create it.
    printf 'Creating symlink %s -> %s\n' "$WEB_LINK" "$SITE_DIR"
    sudo ln -sfn "$SITE_DIR" "$WEB_LINK"
fi

# Ensure Apache is running and will restart on reboot.
sudo systemctl enable apache2 || true
sudo systemctl restart apache2 || true

# Verify Apache can read the site.
if [ ! -r "$WEB_LINK/index.php" ]; then
    echo "ERROR: Apache cannot read $WEB_LINK/index.php" >&2
    exit 1
fi

# Disable PCRE JIT. Some hardened/server environments deny executable
# memory allocation for PCRE JIT; the app does not need regex JIT speed.
sudo sed -i.bak 's/^;*pcre.jit=.*/pcre.jit=0/' /etc/php/*/cli/php.ini || true
sudo sed -i.bak 's/^;*pcre.jit=.*/pcre.jit=0/' /etc/php/*/apache2/php.ini || true
sudo systemctl reload apache2 || true

# Syntax checks. These are not exhaustive, but they catch some mistakes.

php -l 0yahrzeit.php
php -l 1yizkor.php
php -l 2panels.php
php -l 3singlepanel.php
php -l 4names.php
php -l 5singlename.php
php -l 6reports.php
php -l 7minhag.php
php -l include/yahrzeit_policy.inc.php
php -l bin/yahrzeit_scheduler
php -l bin/yahrzeit_engine.php
php -l bin/fix-up-crontab
bash -n bin/yahrzeit

# Runtime checks. These are not exhaustive, but they catch some common misconfigurations.

if ! php -m | grep -qi '^calendar$'; then
    echo "WARNING: PHP calendar extension was not detected; Hebrew-date code may fail." >&2
fi

if ! command -v timeout >/dev/null 2>&1; then
    echo "WARNING: timeout command was not found; controller timeout handling may fail." >&2
fi

bin/yahrzeit --audit || true
bin/yahrzeit --dry-run || true
php tests/yahrzeit_engine_policy_test.php

# Install or repair only the marked Yahrzeit block in the intended appliance
# account's crontab. This command does not transmit to the controller.
sudo "$CRON_WRAPPER"

FIRST_IP="$(hostname -I | awk '{print $1}')"

# Test Apache configuration for syntax errors.
sudo apache2ctl -t || {
    echo "ERROR: Apache configuration has syntax errors" >&2
    sudo apache2ctl -t 2>&1 || true
    exit 1
}

cat <<EOF2

Install/update complete.

✓ Apache configured to serve PHP
✓ Yahrzeit site linked to: $WEB_LINK
✓ Apache configuration verified
✓ Scheduled lighting installed for: $CRON_USER
✓ Automation log rotation installed: 13 compressed weekly logs
✓ Controller address: $CONTROLLER_HOST_INPUT

Access the site at:
  http://$FIRST_IP/$WEB_ALIAS/
  http://$FIRST_IP/$WEB_ALIAS/index.php
  http://$FIRST_IP/$WEB_ALIAS/0yahrzeit.php

Quick verification:
  cd $SITE_DIR
  bin/yahrzeit --audit
  bin/yahrzeit --dry-run

Notes:
  - Cron is generated from data/minhag.ini. Save the Minhag screen or run
    sudo $SITE_DIR/bin/fix-up-crontab to repair it after an OS migration.
  - This script does not transmit to the physical controller during tests.
  - If the repo becomes private later, configure SSH/deploy-key access before
    relying on git pull from this appliance.
EOF2
