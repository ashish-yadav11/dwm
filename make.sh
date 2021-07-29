#!/bin/sh
pkgname=dwm
tar -zcvf "$pkgname.tar.gz" "$pkgname"
makepkg -cfsi
