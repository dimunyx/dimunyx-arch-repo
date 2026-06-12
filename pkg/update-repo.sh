#!/usr/bin/env bash
set -e

REPO="dimunyx-arch-repo"

cd "$(dirname "$0")"

shopt -s nullglob
pkgs=(*.pkg.tar.zst)

if [ ${#pkgs[@]} -eq 0 ]; then
  echo "❌ no packages"
  exit 1
fi

echo "Updating repo..."

repo-add "$REPO.db.tar.gz" "${pkgs[@]}"

echo "Done"
