OBJ_DIR=.
OBJECTS=$(OBJ_DIR)/k_means.o 
ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/
KMEANS_DIR=./k_means

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

k_means_plus.so: k_means_plus.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so 
	${CPP} $(CFLAGS_SHARED) -o k_means_plus.so k_means_plus.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so

k_means_plus.o: k_means_plus.cpp $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CPP} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) k_means_plus.cpp

k_means.o: $(KMEANS_DIR)/k_means.cpp
	${CPP} -fPIC -I$(KMEANS_DIR) -c $(CFLAGS) $(KMEANS_DIR)/k_means.cpp
clean:
	rm -f *.o *.so *~ core $(HEXARRS)
