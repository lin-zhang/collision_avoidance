/*
 * waypoints_gen
 *
 * This is to be a well (over) documented block to serve as a good
 * example.
 */

#define DEBUG 1 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ubx.h"
#include "RoboMap/RoboMap.h"
#include "RoboMap/waypoints_func.h"
/* declare and initialize a microblx type. This will be registered /
 * deregistered in the module init / cleanup at the end of this
 * file.
 *
 * Include regular header file and it's char array representation
 * (used for luajit reflection, logging, etc.)
 */
#include "types/waypoints_gen_config.h"
#include "types/waypoints_gen_config.h.hexarr"

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

using namespace RM;
#define ROBOT_WIDTH 400

/*
#define MAP_WIDTH 1000
#define MAP_HEIGHT 1000

RoboMap map1(MAP_WIDTH,MAP_HEIGHT);
*/
#define DATA_LENGTH 12
#define NUM_NODES (MAX_OBS*MAX_SAT+4)
/* declare the type and give the char array type representation as the type private_data */
ubx_type_t waypoints_gen_config_type = def_struct_type(struct waypoints_gen_config, &waypoints_gen_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char waypoints_gen_meta[] =
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
ubx_config_t waypoints_gen_config[] = {
	{ .name="waypoints_gen_config", .type_name = "struct waypoints_gen_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t waypoints_gen_ports[] = {
	{ .name="centerCoordinates", .in_type_name="float", .in_data_len=DATA_LENGTH*2},
	{ .name="nCenters", .in_type_name="int" },
	{ .name="beginPos", .in_type_name="float", .in_data_len=2},
	{ .name="endPos", .in_type_name="float", .in_data_len=2},
	{ .name="wayPoints", .out_type_name="float", .out_data_len=NUM_NODES*2},
	{ .name="costTable", .out_type_name="float", .out_data_len=NUM_NODES*NUM_NODES},
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct waypoints_gen_info {
	float data[DATA_LENGTH*2];
        int nCenters;
	float initGoal[2];
	float initPos[2];
	Point2D beginPos;
	Point2D endPos;
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */

def_read_arr_fun(read_centerCoordinates, float, DATA_LENGTH*2)
def_write_arr_fun(write_wayPoints, float, NUM_NODES*2)
def_write_arr_fun(write_costTable, float, NUM_NODES*NUM_NODES)
def_read_arr_fun(read_pos, float, 2);
def_read_fun(read_nCenters, int);
/**
 * waypoints_gen_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int waypoints_gen_init(ubx_block_t *b)
{
	int ret=0;
	DBG(" ");

	if ((b->private_data = calloc(1, sizeof(struct waypoints_gen_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		return -1;
	}
	struct waypoints_gen_info* inf=(struct waypoints_gen_info*) b->private_data;
	
	unsigned int clen;
	struct waypoints_gen_config* waypoints_gen_conf;
	
        waypoints_gen_conf = (struct waypoints_gen_config*) ubx_config_get_data_ptr(b, "waypoints_gen_config", &clen);
	inf->nCenters=waypoints_gen_conf->nCenters;
	inf->initGoal[0]=waypoints_gen_conf->initGoal[0];
	inf->initGoal[1]=waypoints_gen_conf->initGoal[1];
	inf->initPos[0]=waypoints_gen_conf->initPos[0];
	inf->initPos[1]=waypoints_gen_conf->initPos[1];
	return ret;
}

/**
 * waypoints_gen_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void waypoints_gen_cleanup(ubx_block_t *b)
{
	DBG(" ");
	free(b->private_data);
}

/**
 * waypoints_gen_start - start the waypoints_gen block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int waypoints_gen_start(ubx_block_t *b)
{
	DBG("in start");
	return 0; /* Ok */
}

/**
 * waypoints_gen_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void waypoints_gen_step(ubx_block_t *b) {
	int ret_read_pos=0;
	struct waypoints_gen_info* inf=(struct waypoints_gen_info*) b->private_data;
	Point2D p0[MAX_OBS];
	Point2D endPos;
	Point2D beginPos;
	Point2D assistingNodesPairs[MAX_OBS*MAX_SAT+4];
	Point2D ppd[MAX_OBS*MAX_SAT];
	float distPedal[MAX_OBS];
	Point2D obsPos[MAX_OBS];
	int assistingNodeRange=ROBOT_WIDTH;
	float inBeginPos[2];
	float inEndPos[2];
	int inNCenters;
	float outPoints[NUM_NODES*2];
	float costTable[NUM_NODES*NUM_NODES];
	float inCenterCoordinates[DATA_LENGTH*2];
	Point2D NodesPairs[NUM_NODES];

        ubx_port_t* p_costTable = ubx_port_get(b, "costTable");
        ubx_port_t* p_wayPoints = ubx_port_get(b, "wayPoints");

        ubx_port_t* p_nCenters = ubx_port_get(b, "nCenters");
        ubx_port_t* p_beginPos = ubx_port_get(b, "beginPos");
        ubx_port_t* p_endPos = ubx_port_get(b, "endPos");
        ubx_port_t* p_centerCoordinates = ubx_port_get(b, "centerCoordinates");
		
	
	read_centerCoordinates(p_centerCoordinates, &inCenterCoordinates);
	ret_read_pos=read_pos(p_beginPos, &inBeginPos);
	//DBG("%d,ret\n",ret_read_pos);
/*	if(ret_read_pos==0){
		inBeginPos[0]=inf->initPos[0];
		inBeginPos[1]=inf->initPos[1];
	}*/
	ret_read_pos=read_pos(p_endPos, &inEndPos);
	//DBG("%d,ret\n",ret_read_pos);
/*        if(ret_read_pos==0){
                inEndPos[0]=inf->initGoal[0];
                inEndPos[1]=inf->initGoal[1];
        }*/

	//DBG("beginPos, endPos [%8.3f,%8.3f,%8.3f,%8.3f]", inBeginPos[0], inBeginPos[1],  inEndPos[0], inEndPos[1]);

	read_nCenters(p_nCenters, &inNCenters);
	if(ret_read_pos>0){
	inf->beginPos.setX(inBeginPos[0]*1000);
	inf->beginPos.setY(inBeginPos[1]*1000);
	inf->endPos.setX(inEndPos[0]*1000);
	inf->endPos.setY(inEndPos[1]*1000);
/*	DBG("beginPos, endPos [%8.3f,%8.3f,%8.3f,%8.3f]", 
	inf->beginPos.getX(), 
	inf->beginPos.getY(),
	inf->endPos.getX(), 
	inf->endPos.getY());*/
}

//#define OFFSET_X 5000
//#define OFFSET_Y 5000
#define OFFSET_X inf->beginPos.getX()
#define OFFSET_Y inf->beginPos.getY()

	for(int i=0;i<DATA_LENGTH;i++){
		obsPos[i].setX(inCenterCoordinates[i*2]+OFFSET_X);
		obsPos[i].setY(inCenterCoordinates[i*2+1]+OFFSET_Y);
	}


	//DBG("inNCenters %d\n",inNCenters);
	for(int i=0;i<DATA_LENGTH;i++){
		assistingNodesPairs[i].setX(inCenterCoordinates[i*2]);
		assistingNodesPairs[i].setY(inCenterCoordinates[i*2+1]);
	}

	for(int i=0;i<MAX_OBS;i++){
		p0[i]=PedalCoordinates(obsPos[i],Line2D(inf->beginPos,inf->endPos));
		distPedal[i]=distPointToLine(obsPos[i], Line2D(inf->beginPos,inf->endPos));
	}

	//DBG("test A\n");
	satelliteNode(inf->endPos,p0,obsPos,assistingNodesPairs,ppd,ROBOT_WIDTH/2+assistingNodeRange,distPedal);

	//DBG("test post A\n");
	for (int m=0;m<NUM_NODES;m++){
		for(int n=0;n<NUM_NODES;n++){
			costTable[m*NUM_NODES+n]=INFINITY;
			//printf("%d,%d,%d,%d\n", m,n,m*NUM_NODES+n,NUM_NODES*NUM_NODES);
		}
	}

	//DBG("test A\n");
	NodesPairs[0].setX(inf->beginPos.getX());
	NodesPairs[0].setY(inf->beginPos.getY());

	NodesPairs[1].setX(inf->beginPos.getX()+assistingNodeRange*2);
	NodesPairs[1].setY(inf->beginPos.getY());

	NodesPairs[2].setX(inf->beginPos.getX()-assistingNodeRange*2);
	NodesPairs[2].setY(inf->beginPos.getY());

	NodesPairs[3].setX(inf->beginPos.getX());
	NodesPairs[3].setY(inf->beginPos.getY()+assistingNodeRange*2);

	NodesPairs[4].setX(inf->beginPos.getX());
	NodesPairs[4].setY(inf->beginPos.getY()-assistingNodeRange*2);

	//DBG("test A\n");
	for (int m=0;m<NUM_NODES;m++){
		for(int n=0;n<NUM_NODES;n++){
			costTable[m*NUM_NODES+n]=INFINITY;
		}
		NodesPairs[m+5].setX(assistingNodesPairs[m].getX());
		NodesPairs[m+5].setY(assistingNodesPairs[m].getY());
	}

	NodesPairs[NUM_NODES-1].setX(inf->endPos.getX());
	NodesPairs[NUM_NODES-1].setY(inf->endPos.getY());
	
	//DBG("test B\n");

	for (int m=0;m<NUM_NODES;m++){
		for(int n=m+1;n<NUM_NODES;n++){
			if(m%2==0&&n==(m+1)){
				costTable[m*NUM_NODES+n]=INFINITY;
			}
			else if(checkBlockingObstacles(NodesPairs[m],NodesPairs[n],obsPos,ROBOT_WIDTH/2)>0){
				costTable[m*NUM_NODES+n]=INFINITY;
			}
			else{
				costTable[m*NUM_NODES+n]=(float)distPoints(NodesPairs[m],NodesPairs[n]);
			}
		}
	}

	for(int i=0;i<NUM_NODES;i++){
		outPoints[i*2]=NodesPairs[i].getX();
		outPoints[i*2+1]=NodesPairs[i].getY();
	//DBG("[%12.3f,%12.3f] waypoint %d",NodesPairs[i].getX(),NodesPairs[i].getY(),i);
	}
	
	write_wayPoints(p_wayPoints, &outPoints);
	write_costTable(p_costTable, &costTable);

}


/* put everything together
 *
 */
ubx_block_t waypoints_gen_comp = {
	.name = "waypoints_gen/waypoints_gen",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = waypoints_gen_meta,
	.configs = waypoints_gen_config,
	.ports = waypoints_gen_ports,

	/* ops */
	.init = waypoints_gen_init,
	.start = waypoints_gen_start,
	.step = waypoints_gen_step,
	.cleanup = waypoints_gen_cleanup,
};

/**
 * waypoints_gen_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int waypoints_gen_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &waypoints_gen_config_type);
	return ubx_block_register(ni, &waypoints_gen_comp);
}

/**
 * waypoints_gen_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void waypoints_gen_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct waypoints_gen_config");
	ubx_block_unregister(ni, "waypoints_gen/waypoints_gen");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(waypoints_gen_module_init)
UBX_MODULE_CLEANUP(waypoints_gen_module_cleanup)
