ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

ps2x_youbot.so: ps2x_youbot.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o ps2x_youbot.so ps2x_youbot.o $(INCLUDE_DIR)/libubx.so

ps2x_youbot.o: ps2x_youbot.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) ps2x_youbot.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
