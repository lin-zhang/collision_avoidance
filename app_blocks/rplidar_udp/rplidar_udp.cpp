/*
 * A fblock that generates rplidar_udp numbers.
 *
 * This is to be a well (over) documented block to serve as a good
 * example.
 */

#define DEBUG 1 

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ubx.h"
/* declare and initialize a microblx type. This will be registered /
 * deregistered in the module init / cleanup at the end of this
 * file.
 *
 * Include regular header file and it's char array representation
 * (used for luajit reflection, logging, etc.)
 */
#include "types/rplidar_udp_config.h"
#include "types/rplidar_udp_config.h.hexarr"
#include <unistd.h>
#include <math.h>
#include <vector>
#include <float.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header
#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

/* Number of nodes detected */
#define N_DETECT 360
/* Data size = 2x number of nodes */
#define N_DATA N_DETECT*2

using namespace rp::standalone::rplidar;

/*Data union for transfering float or double type in char. */
union data{
	float f;
	int i;
};

/*UDP packets buffer size*/
#define BUF_SIZE 3200

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t rplidar_udp_config_type = def_struct_type(struct rplidar_udp_config, &rplidar_udp_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char rplidar_udp_meta[] =
	"{ doc='A block to interpret a string to data',"
	"  real-time=true,"
	"}";

/* configuration
 * upon cloning the following happens:
 *   - value.type is resolved
 *   - value.data will point to a buffer of size value.len*value.type->size
 *
 * if an array is required, then .value = { .len=<LENGTH> } can be used.
 */
ubx_config_t rplidar_udp_config[] = {
	{ .name="rplidar_udp_config", .type_name = "struct rplidar_udp_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t rplidar_udp_ports[] = {
	{ .name="rplidar_data", .out_type_name="float", .out_data_len=N_DATA},
	{ NULL },
};

u_result capture(RPlidarDriver * drv, size_t* detected_counts, rplidar_response_measurement_node_t* detected_nodes)
{
    u_result ans;
    rplidar_response_measurement_node_t nodes[N_DATA];
    size_t   count = _countof(nodes);
    ans = drv->grabScanData(nodes, count);
    for(int i=0;i<(int)count;i++)
        *(detected_nodes+i)=nodes[i];
    *detected_counts = count;
    if (IS_OK(ans) || ans == RESULT_OPERATION_TIMEOUT) {
        drv->ascendScanData(nodes, count);
    } else {
        printf("error code: %x\n", ans);
    }

    return ans;
}

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct rplidar_udp_info {

	unsigned char debug_flag;

	char *opt_com_path;
    	_u32 opt_com_baudrate;
        unsigned char* host_ip;
        char buf[BUF_SIZE];
        unsigned int port;
        int n,s;
        struct sockaddr_in server;
        struct hostent *host;
	unsigned int EnableUDP;	

	RPlidarDriver * drv;
	rplidar_response_device_health_t healthinfo;
	rplidar_response_device_info_t devinfo;
	rplidar_response_measurement_node_t nodes[N_DATA];
	u_result op_result;
	union data temp_angle, temp_dist;
	int* nr_detected_counts;
	
	float 		rp_data[N_DATA];
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */

/* Write measured data to output port */
def_write_arr_fun(write_rplidar_data, float, N_DATA);

/**
 * rplidar_udp_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int rplidar_udp_init(ubx_block_t *b)
{
	struct rplidar_udp_info* inf=(struct rplidar_udp_info*) b->private_data;
	unsigned int clen;
	struct rplidar_udp_config* rplidar_udp_conf;

    	inf->nr_detected_counts=(int*)malloc(sizeof(int));
	inf->drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);

	if(!inf->drv){
		ERR("Insufficient memory for RPLidar, exit");
		return -2;	
	}
	int ret=0;

	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct rplidar_udp_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		return ret;
	}
        rplidar_udp_conf = (struct rplidar_udp_config*) ubx_config_get_data_ptr(b, "rplidar_udp_config", &clen);
	inf->opt_com_path = rplidar_udp_conf->opt_com_path;
	inf->opt_com_baudrate = rplidar_udp_conf->opt_com_baudrate;
	inf->host_ip = rplidar_udp_conf->host_ip;
	inf->port = rplidar_udp_conf->port;
	inf->EnableUDP = rplidar_udp_conf->EnableUDP;
	inf->debug_flag = rplidar_udp_conf->debug_flag;

        if (IS_FAIL(inf->drv->connect(inf->opt_com_path, inf->opt_com_baudrate))) {
            ERR("Error, cannot bind to the specified serial port %s.\n"
                , inf->opt_com_path);
            return -2;
        }

        inf->op_result = inf->drv->getDeviceInfo(inf->devinfo);

        if (IS_FAIL(inf->op_result)) {
            if (inf->op_result == RESULT_OPERATION_TIMEOUT) {
                // you can check the detailed failure reason
                fprintf(stderr, "Error, operation time out.\n");
            } else {
                fprintf(stderr, "Error, unexpected error, code: %x\n", inf->op_result);
                // other unexpected result
            }
            goto out;
        }
        inf->op_result = inf->drv->getHealth(inf->healthinfo);
        if (IS_OK(inf->op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
            printf("RPLidar health status : ");
            switch (inf->healthinfo.status) {
            case RPLIDAR_STATUS_OK:
                printf("OK.");
                break;
            case RPLIDAR_STATUS_WARNING:
                printf("Warning.");
                break;
            case RPLIDAR_STATUS_ERROR:
                printf("Error.");
                break;
            }
            printf(" (errorcode: %d)\n", inf->healthinfo.error_code);

        } else {
            fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", inf->op_result);
            goto out;
        }

        if (inf->healthinfo.status == RPLIDAR_STATUS_ERROR) {
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // inf->drv->reset();
            goto out;
        }

    inf->host=gethostbyname((const char*)rplidar_udp_conf->host_ip);

    if ((inf->s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        return 1;
    }
    /* initialize server addr */
    memset((char *) &(inf->server), 0, sizeof(struct sockaddr_in));
    inf->server.sin_family = AF_INET;
    inf->server.sin_port = htons(inf->port);
    inf->server.sin_addr = *((struct in_addr*) inf->host->h_addr);

out:
	return ret;
}

/**
 * rplidar_udp_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void rplidar_udp_cleanup(ubx_block_t *b)
{
	DBG(" ");
        struct rplidar_udp_info* inf=(struct rplidar_udp_info*) b->private_data;
	free(b->private_data);
	RPlidarDriver::DisposeDriver(inf->drv);
}

/**
 * rplidar_udp_start - start the rplidar_udp block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int rplidar_udp_start(ubx_block_t *b)
{
	DBG("in");
        struct rplidar_udp_info* inf=(struct rplidar_udp_info*) b->private_data;
	inf->drv->startScan();
	return 0; /* Ok */
}

/**
 * rplidar_udp_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
int counter_a=0;
static void rplidar_udp_step(ubx_block_t *b) {
        unsigned short 	raw_angle_buf, raw_dist_buf;
        struct 		rplidar_udp_info* inf=(struct rplidar_udp_info*) b->private_data;
        unsigned int 	len = sizeof(struct sockaddr_in);
        ubx_port_t* 	data_port = ubx_port_get(b, "rplidar_data");

        capture(inf->drv,(size_t*)inf->nr_detected_counts,inf->nodes);
    	for(int i=1;i<*(inf->nr_detected_counts)+1;i++){
    		inf->temp_angle.f = (inf->nodes[i].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f;
    		inf->temp_dist.f = (inf->nodes[i].distance_q2/4.0f);
		if(i<N_DETECT){
		inf->rp_data[i*2]=-cos(inf->temp_angle.f*3.1415926/180)*inf->temp_dist.f*0.97;
		inf->rp_data[i*2+1]=sin(inf->temp_angle.f*3.1415926/180)*inf->temp_dist.f*0.97;
		}
	DBG("%12.3f, %12.3f\n",inf->temp_angle.f,inf->temp_dist.f);	
    	}
    	for(int i=0;i<*inf->nr_detected_counts*2;i++){


    		raw_angle_buf = inf->nodes[i].angle_q6_checkbit;
    		raw_dist_buf = inf->nodes[i].distance_q2;

    		for(int j=0;j<2;j++){
    			inf->buf[i*4+j]=(raw_angle_buf>>(j*8))&0x00FF;
    			inf->buf[i*4+j+2]=(raw_dist_buf>>(j*8))&0x00FF;
    		}
        }
	write_rplidar_data(data_port, &(inf->rp_data));
if(inf->EnableUDP==1){
        if (sendto(inf->s, inf->buf, *(inf->nr_detected_counts)*4, 0, (struct sockaddr *) &(inf->server), len) == -1) {
            perror("sendto()");
            return;
        }
}

	return;
}


/* put everything together
 *
 */
ubx_block_t rplidar_udp_comp = {
	.name = "rplidar_udp/rplidar_udp",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = rplidar_udp_meta,
	.configs = rplidar_udp_config,
	.ports = rplidar_udp_ports,

	/* ops */
	.init = rplidar_udp_init,
	.start = rplidar_udp_start,
	.step = rplidar_udp_step,
	.cleanup = rplidar_udp_cleanup,
};

/**
 * rplidar_udp_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int rplidar_udp_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &rplidar_udp_config_type);
	return ubx_block_register(ni, &rplidar_udp_comp);
}

/**
 * rplidar_udp_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void rplidar_udp_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct rplidar_udp_config");
	ubx_block_unregister(ni, "rplidar_udp/rplidar_udp");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(rplidar_udp_module_init)
UBX_MODULE_CLEANUP(rplidar_udp_module_cleanup)
