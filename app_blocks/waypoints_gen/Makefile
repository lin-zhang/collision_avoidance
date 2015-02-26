OBJ_DIR=.
OBJECTS=$(OBJ_DIR)/RoboMap.o $(OBJ_DIR)/waypoints_func.o
ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/
ROBOMAP_DIR=./RoboMap

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

waypoints_gen.so: waypoints_gen.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so 
	${CPP} $(CFLAGS_SHARED) -o waypoints_gen.so waypoints_gen.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so

waypoints_gen.o: waypoints_gen.cpp $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CPP} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) waypoints_gen.cpp

RoboMap.o: $(ROBOMAP_DIR)/RoboMap.cpp
	${CPP} -fPIC -I$(ROBOMAP_DIR) -c $(CFLAGS) $(ROBOMAP_DIR)/RoboMap.cpp

waypoints_func.o: $(ROBOMAP_DIR)/waypoints_func.cpp
	${CPP} -fPIC -I$(ROBOMAP_DIR) -c $(CFLAGS) $(ROBOMAP_DIR)/waypoints_func.cpp

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
