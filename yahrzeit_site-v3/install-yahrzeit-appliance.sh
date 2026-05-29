#!/usr/bin/env bash
set -euo pipefail

REPO_URL="https://github.com/ams613/yahrzeit.git"
INSTALL_DIR="/home/pi/yahrzeit"
SITE_DIR="$INSTALL_DIR/yahrzeit_site-v3"

echo "Installing CBS Yahrzeit appliance software"

if ! command -v apt-get >/dev/null 2>&1; then
    echo "ERROR: this installer expects a Debian/Raspberry Pi OS system" >&2
    exit 1
fi

sudo apt-get update
sudo apt-get install -y git php-cli php-common netcat-openbsd

if [ ! -d "$INSTALL_DIR/.git" ]; then
    git clone "$REPO_URL" "$INSTALL_DIR"
else
    cd "$INSTALL_DIR"
    git pull --ff-only
fi

cd "$SITE_DIR"

mkdir -p data/backups
chmod 755 bin/yahrzeit bin/yahrzeit_scheduler bin/yahrzeit_engine.php

php -l bin/yahrzeit_scheduler
php -l bin/yahrzeit_engine.php
bash -n bin/yahrzeit

bin/yahrzeit --audit || true

cat <<'EOF'

Install complete.

Next steps:
  1. Confirm controller host/port.
  2. Test: bin/yahrzeit --notransmit
  3. Test live controller only when ready.
  4. Install cron entries.

Suggested cron:
  0 11 * * * cd /home/pi/yahrzeit/yahrzeit_site-v3 && bin/yahrzeit_scheduler --phase yizkor-on  >> data/scheduler.log 2>&1
  0 13 * * * cd /home/pi/yahrzeit/yahrzeit_site-v3 && bin/yahrzeit_scheduler --phase yizkor-off >> data/scheduler.log 2>&1
  0 16 * * * cd /home/pi/yahrzeit/yahrzeit_site-v3 && bin/yahrzeit_scheduler --phase yahrzeit   >> data/scheduler.log 2>&1
EOF
