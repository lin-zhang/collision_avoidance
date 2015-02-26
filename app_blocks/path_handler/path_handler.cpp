/*
 * path_handler
 *
 * This is to be a well (over) documented block to serve as a good
 * example.
 */

#define DEBUG 1 

#include <stdio.h>
#include <stdlib.h>

#include "ubx.h"
/* declare and initialize a microblx type. This will be registered /
 * deregistered in the module init / cleanup at the end of this
 * file.
 *
 * Include regular header file and it's char array representation
 * (used for luajit reflection, logging, etc.)
 */
#include "types/path_handler_config.h"
#include "types/path_handler_config.h.hexarr"

#include <iostream>
#include <vector>
#include <string>
#include <list>

#include <limits> // for numeric_limits
#include <set>
#include <utility> // for pair
#include <algorithm>
#include <iterator>
#include <math.h>


//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define DATA_LENGTH 24
#define MAX_OBS 12
#define MAX_SAT 4
#define NUM_NODES (MAX_OBS*MAX_SAT+4)

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t path_handler_config_type = def_struct_type(struct path_handler_config, &path_handler_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char path_handler_meta[] =
	"{ doc='A block to read data from serial port',"
	"  real-time=true,"
	"}";

/* configuration
 * upon cloning the following happens:
 *   - value.type is resolved
 *   - value.data will point to a buffer of size value.len*value.type->size
 *
 * if an array is required, then .value = { .len=<LENGTH> } can be used.
 */
ubx_config_t path_handler_config[] = {
	{ .name="path_handler_config", .type_name = "struct path_handler_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t path_handler_ports[] = {
	{ .name="plannedPathWayPoints", .in_type_name="float", .in_data_len=NUM_NODES*2},
	{ .name="currentPos", .in_type_name="float", .in_data_len=2},
	{ .name="goalPos", .in_type_name="float", .in_data_len=2},
	{ .name="nValidNodes", 	.in_type_name="int" },
	{ .name="moveToPos", .out_type_name="float", .out_data_len=2},
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct path_handler_info {
	float data[DATA_LENGTH];
        int nCenters;
	float currPos[2];
	float moveToPos[2];
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */

def_read_arr_fun(read_currentPos, float, 2)
def_read_arr_fun(read_goalPos, float, 2)
def_read_arr_fun(read_plannedPathWayPoints, float, NUM_NODES*2)
def_read_fun(read_nValidNodes, int)
def_write_arr_fun(write_moveToPos, float, 2)
/**
 * path_handler_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int path_handler_init(ubx_block_t *b)
{
	int ret=0;
	DBG(" ");

	if ((b->private_data = calloc(1, sizeof(struct path_handler_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		return -1;
	}
	struct path_handler_info* inf=(struct path_handler_info*) b->private_data;
	
	unsigned int clen;
	struct path_handler_config* path_handler_conf;
	
        path_handler_conf = (struct path_handler_config*) ubx_config_get_data_ptr(b, "path_handler_config", &clen);
	inf->nCenters=path_handler_conf->nCenters;
	return ret;
}

/**
 * path_handler_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void path_handler_cleanup(ubx_block_t *b)
{
	DBG(" ");
	free(b->private_data);
}

/**
 * path_handler_start - start the path_handler block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int path_handler_start(ubx_block_t *b)
{
	DBG("in start");
	return 0; /* Ok */
}

/**
 * path_handler_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void path_handler_step(ubx_block_t *b) {
	int nValidNodes=0;
	float plannedPathWayPoints[NUM_NODES*2];	
	float goalPos[2];	
	float currentPos[2];
	float moveToPos[2];
        struct path_handler_info* inf=(struct path_handler_info*) b->private_data;

        ubx_port_t* p_nValidNodes = ubx_port_get(b, "nValidNodes");
        ubx_port_t* p_plannedPathWayPoints = ubx_port_get(b, "plannedPathWayPoints");
        ubx_port_t* p_currentPos = ubx_port_get(b, "currentPos");
        ubx_port_t* p_goalPos = ubx_port_get(b, "goalPos");
        ubx_port_t* p_moveToPos = ubx_port_get(b, "moveToPos");	
	
	read_currentPos			(p_currentPos, 			&currentPos);
	read_goalPos			(p_goalPos, 			&goalPos);
	read_plannedPathWayPoints	(p_plannedPathWayPoints, 	&plannedPathWayPoints);
	read_nValidNodes		(p_nValidNodes,			&nValidNodes);

	if(nValidNodes>0){
	if(plannedPathWayPoints[2]!=INFINITY&&plannedPathWayPoints[3]!=INFINITY){
		inf->moveToPos[0]=plannedPathWayPoints[2];
		inf->moveToPos[1]=plannedPathWayPoints[3];
	}
		//DBG("Next point: %12.3f, %12.3f", inf->moveToPos[0], inf->moveToPos[1]);
	}
		moveToPos[0]=inf->moveToPos[0];
		moveToPos[1]=inf->moveToPos[1];
	//if getting approach, trigger a next calculation?
	
		DBG("Next point: %12.3f, %12.3f", moveToPos[0], moveToPos[1]);
	write_moveToPos(p_moveToPos, &moveToPos);
}


/* put everything together
 *
 */
ubx_block_t path_handler_comp = {
	.name = "path_handler/path_handler",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = path_handler_meta,
	.configs = path_handler_config,
	.ports = path_handler_ports,

	/* ops */
	.init = path_handler_init,
	.start = path_handler_start,
	.step = path_handler_step,
	.cleanup = path_handler_cleanup,
};

/**
 * path_handler_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int path_handler_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &path_handler_config_type);
	return ubx_block_register(ni, &path_handler_comp);
}

/**
 * path_handler_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void path_handler_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct path_handler_config");
	ubx_block_unregister(ni, "path_handler/path_handler");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(path_handler_module_init)
UBX_MODULE_CLEANUP(path_handler_module_cleanup)
