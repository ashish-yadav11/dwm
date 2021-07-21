# Maintainer: Ashish Kumar Yadav <ashishkumar.yadav@students.iiserpune.ac.in>
# Based on dwm PKGBUILD

pkgname=dwm-cust
_pkgname=dwm
pkgver=6.2
pkgrel=1
pkgdesc='Custom build of dwm'
arch=(i686 x86_64)
url='https://github.com/ashish-yadav11/dwm'
license=(MIT)
depends=(libx11 libxft freetype2)
optdepends=('rofi: default app launcher and window switcher')
provides=(dwm)
conflicts=(dwm)
options=(zipman '!strip')

source=("${_pkgname}-${pkgver}.tar.gz"
        "${_pkgname}.desktop")
sha256sums=(SKIP
            SKIP)

build() {
    cd "${srcdir}/${_pkgname}-${pkgver}"
    make X11INC=/usr/include/X11 X11LIB=/usr/lib/X11 FREETYPEINC=/usr/include/freetype2
}

package() {
    cd "${srcdir}/${_pkgname}-${pkgver}"
    make PREFIX=/usr DESTDIR="${pkgdir}" install
    install -m644 -D LICENSE "${pkgdir}/usr/share/licenses/${_pkgname}/LICENSE"
    install -m644 -D README "${pkgdir}/usr/share/doc/${_pkgname}/README"
    install -m644 -D "${srcdir}/${_pkgname}.desktop" "${pkgdir}/usr/share/xsessions/${_pkgname}.desktop"
}
