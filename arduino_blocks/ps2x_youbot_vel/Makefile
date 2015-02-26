ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

ps2x_youbot_vel.so: ps2x_youbot_vel.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o ps2x_youbot_vel.so ps2x_youbot_vel.o $(INCLUDE_DIR)/libubx.so

ps2x_youbot_vel.o: ps2x_youbot_vel.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) ps2x_youbot_vel.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
