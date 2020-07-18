#!/bin/sh
pkgname=dwm
pkgver=6.2
tar -zcvf ${pkgname}-${pkgver}.tar.gz ${pkgname}-${pkgver}
makepkg -cfsi
