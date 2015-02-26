ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

serial_rw.so: serial_rw.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o serial_rw.so serial_rw.o $(INCLUDE_DIR)/libubx.so

serial_rw.o: serial_rw.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) serial_rw.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
