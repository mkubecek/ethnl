INCS = $(shell pkg-config --cflags libmnl)
LIBS = $(shell pkg-config --libs libmnl)
CFLAGS = -O0 -ggdb
LDFLAGS = 

%.o: %.c ethtool_netlink.h
	$(CC) -c -o $@ $(CFLAGS) $(INCS) $<

ethnl-test: ethnl-test.o
	$(CC) -o $@ $< $(LIBS)
