DEFS = -D__LINUX__ -DLINUX -DNOWIN32
INC = -I.
CFLAGS = -W

all:	app

udptest: udptest.cpp socketdef.h
	cc $(DEFS) $(INC) $(CFLAGS) udptest.cpp -o udptest
	chmod +x udptest

clean:
	rm -f ./udptest

app: udptest
