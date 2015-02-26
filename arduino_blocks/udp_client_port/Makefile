ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

udp_client_port.so: udp_client_port.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o udp_client_port.so udp_client_port.o $(INCLUDE_DIR)/libubx.so

udp_client_port.o: udp_client_port.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) udp_client_port.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
