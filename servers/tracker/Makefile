# Makefile for tracker server
PROG=	tracker
SRCS=	main.c utility.c

DPADD+=	${LIBSYS}
LDADD+=	-lsys -lminlib -ltimers -lcompat_minix

MAN=

BINDIR?= /usr/sbin
FILES=tracker.conf
FILESNAME=tracker
FILESDIR= /etc/system.conf.d

.include <minix.service.mk>
