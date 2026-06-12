#!/usr/bin/env bash
set -e

REPO_NAME="dimunyx-arch-repo"

# перейти в директорию скрипта
cd "$(dirname "$0")"

echo "Working dir: $(pwd)"

echo "Updating repo database..."

shopt -s nullglob
packages=(*.pkg.tar.zst)

if [ ${#packages[@]} -eq 0 ]; then
    echo "❌ No packages found (*.pkg.tar.zst)"
    exit 1
fi

repo-add "$REPO_NAME.db.tar.gz" "${packages[@]}"

echo "Done!"
