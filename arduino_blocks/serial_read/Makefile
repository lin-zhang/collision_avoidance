ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

serial_read.so: serial_read.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o serial_read.so serial_read.o $(INCLUDE_DIR)/libubx.so

serial_read.o: serial_read.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) serial_read.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
