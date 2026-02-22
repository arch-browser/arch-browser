#!/bin/bash
# Create tarball for makepkg - run from browser/ directory
# Then run: makepkg -si

set -e
cd "$(dirname "${BASH_SOURCE[0]}")"
TARBALL="arch-browser-1.0.0.tar.gz"
tmp=$(mktemp)
tar czvf "$tmp" \
    --transform 's,^,arch-browser-1.0.0/,' \
    --exclude='build' \
    --exclude='pkg' \
    --exclude='.pkg' \
    --exclude="$TARBALL" \
    --exclude='*.pkg.tar*' \
    --exclude='.git' \
    --exclude='src/arch-browser-1.0.0' \
    .
mv "$tmp" "$TARBALL"
echo "Created $TARBALL - run: makepkg -si"
