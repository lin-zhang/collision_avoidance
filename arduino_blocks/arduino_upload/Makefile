ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

arduino_upload.so: arduino_upload.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o arduino_upload.so arduino_upload.o $(INCLUDE_DIR)/libubx.so

arduino_upload.o: arduino_upload.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) arduino_upload.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
