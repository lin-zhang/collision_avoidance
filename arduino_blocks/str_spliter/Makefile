ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

str_spliter.so: str_spliter.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o str_spliter.so str_spliter.o $(INCLUDE_DIR)/libubx.so

str_spliter.o: str_spliter.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) str_spliter.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
