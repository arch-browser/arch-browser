# Arch Linux Native Browser - Chromium-based web browser
#
# Build:
#   1. ./make-tarball.sh
#   2. makepkg -si

pkgname=arch-browser
pkgver=1.0.0
pkgrel=1
pkgdesc="Native Chromium-based web browser for Arch Linux with Qt GUI"
arch=('x86_64')
url="https://github.com/arch-browser/arch-browser"
license=('MIT')
depends=('qt5-base' 'qt5-webengine')
makedepends=('cmake' 'ninja')

source=("${pkgname}-${pkgver}.tar.gz")
sha256sums=('SKIP')

build() {
    cd "${srcdir}/${pkgname}-${pkgver}"
    cmake -B build -S . \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    cd "${srcdir}/${pkgname}-${pkgver}"
    DESTDIR="${pkgdir}" cmake --install build
}
