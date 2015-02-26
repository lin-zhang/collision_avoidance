ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/
RPLIDAR_INCLUDE_DIR=./sdk/include
RPLIDAR_SRC_DIR=./sdk/src
TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

rplidar_udp.so: net_serial.o timer.o thread.o rplidar_driver.o rplidar_udp.o $(INCLUDE_DIR)/libubx.so 
	${CPP} $(CFLAGS_SHARED) -o rplidar_udp.so net_serial.o timer.o thread.o rplidar_driver.o rplidar_udp.o $(INCLUDE_DIR)/libubx.so

net_serial.o: $(RPLIDAR_SRC_DIR)/arch/linux/net_serial.cpp 
	$(CPP) -fPIC -I$(RPLIDAR_INCLUDE_DIR) -I$(RPLIDAR_SRC_DIR) -c $(CFLAGS) $(RPLIDAR_SRC_DIR)/arch/linux/net_serial.cpp
	
timer.o: $(RPLIDAR_SRC_DIR)/arch/linux/timer.cpp 
	$(CPP) -fPIC -I$(RPLIDAR_INCLUDE_DIR) -I$(RPLIDAR_SRC_DIR) -c $(CFLAGS) $(RPLIDAR_SRC_DIR)/arch/linux/timer.cpp
	
thread.o: $(RPLIDAR_SRC_DIR)/hal/thread.cpp 
	$(CPP) -fPIC -I$(RPLIDAR_INCLUDE_DIR) -I$(RPLIDAR_SRC_DIR) -c $(CFLAGS) $(RPLIDAR_SRC_DIR)/hal/thread.cpp
	
rplidar_driver.o: $(RPLIDAR_SRC_DIR)/rplidar_driver.cpp 
	$(CPP) -fPIC -I$(RPLIDAR_INCLUDE_DIR) -I$(RPLIDAR_SRC_DIR) -c $(CFLAGS) $(RPLIDAR_SRC_DIR)/rplidar_driver.cpp
	
rplidar_udp.o: rplidar_udp.cpp $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CPP} -fPIC -I$(INCLUDE_DIR) -I$(RPLIDAR_INCLUDE_DIR) -c $(CFLAGS) rplidar_udp.cpp



clean:
	rm -f *.o *.so *~ core $(HEXARRS)
