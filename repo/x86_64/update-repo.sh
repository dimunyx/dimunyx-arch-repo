#!/usr/bin/env bash
set -euo pipefail
shopt -s nullglob

REPO="dimunyx-arch-repo"
DIR="$(dirname "$0")"
KEY_ID="09D90FC946B01EFE"

cd "$DIR"

pkgs=(*.pkg.tar.zst)
db="$REPO.db.tar.gz"

# Clean up old signatures
rm -f "$REPO.db.tar.gz.sig" *.pkg.tar.zst.sig

# Wipe old db for a clean state
rm -f "$REPO.db" "$REPO.db.tar.gz" "$REPO.files" "$REPO.files.tar.gz" \
      "$REPO.db.tar.gz.old" "$REPO.files.tar.gz.old"

if [ ${#pkgs[@]} -eq 0 ]; then
  echo "No packages found"
  exit 1
fi

echo "Adding ${#pkgs[@]} packages..."
repo-add -R "$db" "${pkgs[@]}"

# Replace symlinks with real copies so raw GitHub URLs work
for f in "$REPO.db" "$REPO.files"; do
  if [ -L "$f" ]; then
    cp --remove-destination "$(readlink "$f")" "$f"
  fi
done

# Sign packages and database
echo "Signing packages and database..."
for pkg in "${pkgs[@]}"; do
  gpg --detach-sign "$pkg"
done
gpg --detach-sign "$REPO.db.tar.gz"

# Export public key
echo "Exporting public key..."
gpg --export --armor "$KEY_ID" > "$REPO.pubkey"

echo "Done"
