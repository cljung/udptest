DEFS = -D__LINUX__ -DLINUX -DNOWIN32
INC = -I.
CFLAGS = -W

all:	app

httpd.o: udptest.cpp socketdef.h
	cc $(DEFS) $(INC) $(CFLAGS) udptest.cpp -o udptest
	chmod +x udptest

app: udptest.o


