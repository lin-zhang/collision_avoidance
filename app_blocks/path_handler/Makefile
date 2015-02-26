ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

path_handler.so: path_handler.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so 
	${CPP} $(CFLAGS_SHARED) -o path_handler.so path_handler.o $(INCLUDE_DIR)/libubx.so

path_handler.o: path_handler.cpp $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CPP} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) path_handler.cpp

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
