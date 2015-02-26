ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

arduino_2dofGadget.so: arduino_2dofGadget.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o arduino_2dofGadget.so arduino_2dofGadget.o $(INCLUDE_DIR)/libubx.so

arduino_2dofGadget.o: arduino_2dofGadget.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) arduino_2dofGadget.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
