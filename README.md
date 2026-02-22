# Arch Browser

A native Chromium-based web browser for **Arch Linux only**, built with C++, Qt5, and QtWebEngine (Chromium). Uses a native GUI toolkit—no HTML, CSS, or JavaScript for the browser chrome.

## Features

- **Tabs**: Open, close, and switch between tabs (Ctrl+T, Ctrl+W)
- **Navigation**: Back, forward, refresh, stop loading, home
- **Address bar**: URL entry with basic validation (adds `https://`, search via DuckDuckGo); Ctrl+L to focus
- **Find in page**: Ctrl+F to search and highlight text on the current page
- **Zoom**: Zoom in (Ctrl++), zoom out (Ctrl+-), reset (Ctrl+0)
- **Bookmarks**: Add bookmark (Ctrl+D), set homepage, persistent storage
- **Cookies & sessions**: Persistent storage—log in to sites (e.g. YouTube) and stay signed in after closing the browser
- **History**: Browsing history (Ctrl+H), clear history, double-click to revisit
- **HTTPS**: Full support via Chromium
- **Multiple windows**: File → New Window
- **Downloads**: Save dialog with default location

## Requirements

- Arch Linux (x86_64)
- Qt5 with WebEngine: `qt5-base`, `qt5-webengine`

## Dependencies

Install build dependencies:

```bash
sudo pacman -S --needed base-devel cmake ninja qt5-base qt5-webengine
```

## Build

### From source (CMake)

```bash
cd /path/to/browser
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

Or with Ninja:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

### Using PKGBUILD (install as system package)

```bash
cd /path/to/browser
./make-tarball.sh
makepkg -si
```

- `-s` installs dependencies with pacman
- `-i` installs the built package

## Uninstall (remove package + all config/data)

```bash
./uninstall-arch-browser.sh
```

Or manually:
```bash
sudo pacman -Rns arch-browser
rm -rf ~/.config/ArchBrowser ~/.local/share/Arch\ Browser
```

## Launch

After install, Arch Browser appears in your application menu and can be launched with:
- **Application launcher**: search for "Arch Browser"
- **Terminal**: `arch-browser`
- **URL handling**: `arch-browser https://example.com`

## Website

Static site in `website/`:
- `index.html` — main landing page
- `install.html` — installation instructions
- `style.css` — shared styles

Open locally or serve with any static host (e.g. `python -m http.server` in `website/`).

## Project Structure

```
browser/
├── CMakeLists.txt         # CMake build configuration
├── PKGBUILD               # Arch Linux package recipe
├── arch-browser.desktop   # Application launcher entry
├── make-tarball.sh        # Creates tarball for makepkg
├── uninstall-arch-browser.sh  # Removes package + all config/data
├── website/                  # Main site + install page
├── README.md              # This file
└── src/
    ├── main.cpp        # Application entry point, Qt/WebEngine init
    ├── MainWindow.hpp  # Main window (tabs, toolbar, bookmarks, zoom)
    ├── MainWindow.cpp  # Implementation
    ├── WebView.hpp     # Web content widget wrapper
    ├── WebView.cpp     # WebView implementation
    ├── FindBar.hpp     # Find-in-page bar
    └── FindBar.cpp     # FindBar implementation
```

## Key Components

| File       | Role                                                                  |
|------------|-----------------------------------------------------------------------|
| `main.cpp` | Initializes Qt, enables WebEngine settings, creates first window      |
| `MainWindow` | Tabs, toolbar, address bar, bookmarks, zoom, find, downloads        |
| `WebView`  | Wraps `QWebEngineView`, handles `target="_blank"` (new tab)           |
| `FindBar`  | Find-in-page UI (Previous/Next, match case)                           |

## License

MIT
