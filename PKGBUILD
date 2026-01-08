# Maintainer: MaskedSyntax <aftaab2507@gmail.com>
pkgname=calendarrr
pkgver=1.0.2
pkgrel=1
pkgdesc="A minimal calendar application built using GTK for XFCE"
arch=('x86_64')
url="https://github.com/maskedsyntax/calendarrr"
license=('GPL3')
depends=('gtk3')
source=("$pkgname-$pkgver.tar.gz")  # Or use a GitHub URL
sha256sums=('SKIP')

package() {
    # Install binary
    install -Dm755 "$srcdir/$pkgname-$pkgver/bin/calendarrr" "$pkgdir/usr/bin/calendarrr"
    
    # Install icons
    install -Dm644 "$srcdir/$pkgname-$pkgver/docs/assets/calendarrr-outlined.svg" \
        "$pkgdir/usr/share/icons/hicolor/scalable/apps/calendarrr-outlined.svg"
    install -Dm644 "$srcdir/$pkgname-$pkgver/docs/assets/calendarrr.svg" \
        "$pkgdir/usr/share/icons/hicolor/scalable/apps/calendarrr.svg"

    # Install desktop file
    install -Dm644 "$srcdir/$pkgname-$pkgver/calendarrr.desktop" \
        "$pkgdir/usr/share/applications/calendarrr.desktop"
}
