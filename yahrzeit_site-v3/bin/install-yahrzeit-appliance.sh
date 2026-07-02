#!/usr/bin/env bash
set -euo pipefail

# Install/update the CBS Yahrzeit Wall PHP appliance on a small Ubuntu/Debian host.
#
# Run locally on a fresh Ubuntu/Debian appliance:
#   curl -fsSL https://raw.githubusercontent.com/allanschwartz/yahrzeit/master/yahrzeit_site-v3/bin/install-yahrzeit-appliance.sh -o /tmp/install-yahrzeit-appliance.sh
#   chmod +x /tmp/install-yahrzeit-appliance.sh
#   /tmp/install-yahrzeit-appliance.sh
#
# This script installs packages, clones or updates the repo, configures Apache,
# runs syntax/audit checks, and prints suggested cron entries.
# It does not install cron automatically and does not transmit to the controller.

REPO_URL="${REPO_URL:-https://github.com/allanschwartz/yahrzeit.git}"
BRANCH="${BRANCH:-master}"
SITE_SUBDIR="${SITE_SUBDIR:-yahrzeit_site-v3}"
INSTALL_PARENT="${INSTALL_PARENT:-$HOME/src}"
REPO_DIR="${REPO_DIR:-$INSTALL_PARENT/yahrzeit}"
SITE_DIR="${SITE_DIR:-$REPO_DIR/$SITE_SUBDIR}"
WEB_ALIAS="${WEB_ALIAS:-yahrzeit}"
WEB_LINK="${WEB_LINK:-/var/www/html/$WEB_ALIAS}"

printf 'Installing/updating CBS Yahrzeit appliance software\n\n'
printf 'Repo:       %s\n' "$REPO_URL"
printf 'Branch:     %s\n' "$BRANCH"
printf 'Repo dir:   %s\n' "$REPO_DIR"
printf 'Site dir:   %s\n' "$SITE_DIR"
printf 'Web link:   %s\n\n' "$WEB_LINK"

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
    coreutils

# Ensure SSH is enabled and running
sudo systemctl enable ssh
sudo systemctl start ssh

# Disable AppArmor (it can interfere with SSH and other services)
sudo systemctl disable apparmor || true
sudo systemctl stop apparmor || true

mkdir -p "$INSTALL_PARENT"

if [ ! -d "$REPO_DIR/.git" ]; then
    git clone --filter=blob:none --sparse "$REPO_URL" "$REPO_DIR"
    cd "$REPO_DIR"
    git sparse-checkout init --cone
    git sparse-checkout set "$SITE_SUBDIR"
    git checkout "$BRANCH"
else
    cd "$REPO_DIR"
    git fetch origin
    git checkout "$BRANCH"
    git sparse-checkout init --cone || true
    git sparse-checkout set "$SITE_SUBDIR"
    git pull --ff-only origin "$BRANCH"
fi

if [ ! -d "$SITE_DIR" ]; then
    echo "ERROR: site directory not found: $SITE_DIR" >&2
    exit 1
fi

cd "$SITE_DIR"

mkdir -p data/backups
: > data/scheduler.log

chmod 755 bin/yahrzeit bin/yahrzeit_scheduler bin/yahrzeit_engine.php

# Apache follows the symlink only if it can traverse the parent directories.
# This grants execute/traverse only, not broad read access to the home tree.
chmod o+x "$HOME"
chmod o+x "$INSTALL_PARENT"
chmod o+x "$REPO_DIR"
chmod o+x "$SITE_DIR"

# Ensure Apache can write to data directory for logs and backups
sudo chown -R www-data:www-data "$SITE_DIR/data" || true
sudo chmod 755 "$SITE_DIR/data" || true

# Configure Apache: ensure php module is enabled and properly configured.
# Try the generic 'php' module first, then try specific PHP versions if it doesn't exist
for php_module in php php8.5 php8.4 php8.3 php8.2 php8.1 php8.0 php7.4; do
    sudo a2enmod "$php_module" 2>/dev/null && break
done || true
sudo a2enmod rewrite || true

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
php -l 1viewpanels.php
php -l 3singlepanel.php
php -l 4viewnames.php
php -l 5singlename.php
php -l 6reports.php
php -l 7minhag.php
php -l include/yahrzeit_policy.inc.php
php -l bin/yahrzeit_scheduler
php -l bin/yahrzeit_engine.php
bash -n bin/yahrzeit

# Runtime checks. These are not exhaustive, but they catch some common misconfigurations.

if ! php -m | grep -qi '^calendar$'; then
    echo "WARNING: PHP calendar extension was not detected; Hebrew-date code may fail." >&2
fi

if ! command -v timeout >/dev/null 2>&1; then
    echo "WARNING: timeout command was not found; controller timeout handling may fail." >&2
fi

bin/yahrzeit --audit || true
bin/yahrzeit --notransmit --status || true

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

Access the site at:
  http://$FIRST_IP/$WEB_ALIAS/
  http://$FIRST_IP/$WEB_ALIAS/index.php
  http://$FIRST_IP/$WEB_ALIAS/0yahrzeit.php

Quick verification:
  cd $SITE_DIR
  bin/yahrzeit --audit
  bin/yahrzeit --notransmit --status

Suggested cron entries for this host:
  0 16 * * 5 cd $SITE_DIR && bin/yahrzeit_scheduler --phase yahrzeit   >> data/scheduler.log 2>&1
  0 11 * * * cd $SITE_DIR && bin/yahrzeit_scheduler --phase yizkor-on  >> data/scheduler.log 2>&1
  0 13 * * * cd $SITE_DIR && bin/yahrzeit_scheduler --phase yizkor-off >> data/scheduler.log 2>&1

Notes:
  - This script does not install cron automatically.
  - This script does not transmit to the physical controller during tests.
  - If the repo becomes private later, configure SSH/deploy-key access before
    relying on git pull from this appliance.
EOF2
