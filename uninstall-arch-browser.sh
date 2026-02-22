#!/bin/bash
# Completely remove Arch Browser: package + all config, cookies, history, cache

set -e

echo "=== Removing Arch Browser ==="

# 1. Uninstall package (if installed via pacman/makepkg)
if pacman -Q arch-browser &>/dev/null; then
    echo "Uninstalling arch-browser package..."
    sudo pacman -Rns arch-browser
else
    echo "arch-browser package not installed (or not found)"
fi

# 2. Remove all user config and data
echo "Removing config and user data..."

rm -rf ~/.config/ArchBrowser
rm -rf ~/.local/share/Arch\ Browser
rm -rf "$HOME/.local/share/Arch Browser"

echo "Done. Arch Browser and all its data have been removed."
