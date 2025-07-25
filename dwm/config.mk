# dwm version
VERSION = 6.5

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/include/X11
X11LIB = /usr/lib/X11

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2

# fribidi
BIDILIBS = -lfribidi
BIDIINC = /usr/include/fribidi

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC} -I${BIDIINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} ${BIDILIBS}

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
#CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS}
CFLAGS   = -g -std=gnu99 -Wall -Wno-deprecated-declarations -Og ${INCS} ${CPPFLAGS}
#CFLAGS   = -g -std=gnu99 -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -Og ${INCS} ${CPPFLAGS}
#CFLAGS   = -std=gnu99 -Wall -Wno-deprecated-declarations -O3 ${INCS} ${CPPFLAGS}
#CFLAGS   = -std=gnu99 -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -O3 ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
#CC = cc
CC = gcc
