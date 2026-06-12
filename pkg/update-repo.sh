#!/usr/bin/env bash
set -e

REPO_NAME="dimunyx-arch-repo"

echo "Updating repo database..."

repo-add "$REPO_NAME.db.tar.gz" *.pkg.tar.zst

echo "Done!"
