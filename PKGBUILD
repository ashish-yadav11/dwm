# Maintainer: Ashish Kumar Yadav <ashishkumar.yadav@students.iiserpune.ac.in>
# Based on dwm PKGBUILD

pkgname=dwm-cust
_pkgname=dwm
pkgver=6.2
pkgrel=1
pkgdesc="A dynamic window manager for X"
url="https://dwm.suckless.org"
arch=('i686' 'x86_64')
license=('MIT')
options=(zipman)
provides=(dwm)
conflicts=(dwm)
depends=('libx11' 'libxft' 'freetype2')
source=($_pkgname-$pkgver.tar.gz
	config.h
        dwm.desktop)
sha256sums=('SKIP'
            'SKIP'
            'SKIP')

prepare() {
  cd "$srcdir/$_pkgname-$pkgver"
  cp "$srcdir/config.h" config.h
}

build() {
  cd "$srcdir/$_pkgname-$pkgver"
  make X11INC=/usr/include/X11 X11LIB=/usr/lib/X11 FREETYPEINC=/usr/include/freetype2
}

package() {
  cd "$srcdir/$_pkgname-$pkgver"
  make PREFIX=/usr DESTDIR="$pkgdir" install
  install -m644 -D LICENSE "$pkgdir/usr/share/licenses/$_pkgname/LICENSE"
  install -m644 -D README "$pkgdir/usr/share/doc/$_pkgname/README"
  install -m644 -D "$srcdir/dwm.desktop" "$pkgdir/usr/share/xsessions/dwm.desktop"
}
