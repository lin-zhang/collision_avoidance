/*
 * k_means_plus
 *
 * This is to be a well (over) documented block to serve as a good
 * example.
 */

#define DEBUG 1 

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ubx.h"
#include "k_means/k_means.h"

/* declare and initialize a microblx type. This will be registered /
 * deregistered in the module init / cleanup at the end of this
 * file.
 *
 * Include regular header file and it's char array representation
 * (used for luajit reflection, logging, etc.)
 */
#include "types/k_means_plus_config.h"
#include "types/k_means_plus_config.h.hexarr"

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


#define DATA_LENGTH 720
/* declare the type and give the char array type representation as the type private_data */
ubx_type_t k_means_plus_config_type = def_struct_type(struct k_means_plus_config, &k_means_plus_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char k_means_plus_meta[] =
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
ubx_config_t k_means_plus_config[] = {
	{ .name="k_means_plus_config", .type_name = "struct k_means_plus_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t k_means_plus_ports[] = {
	{ .name="data_in", .in_type_name="float", .in_data_len=DATA_LENGTH },
	{ .name="data_out", .out_type_name="float", .out_data_len=K*2 },
	{ .name="n_clusters", .in_type_name="int" },
	{ .name="n_clusters_out", .out_type_name="int" },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct k_means_plus_info {
	float data[DATA_LENGTH];
        int n_clusters;
        //int data_dim;
    	point_s cent[K];
	point_s pts[PTS];
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */

def_read_arr_fun(read_float720, float, DATA_LENGTH)
def_write_arr_fun(write_cent, float, K*2)
def_write_fun(write_n_clusters, int)
def_read_fun(read_n_clusters, int)
/**
 * k_means_plus_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int k_means_plus_init(ubx_block_t *b)
{
	int ret=0;
	DBG(" ");

	if ((b->private_data = calloc(1, sizeof(struct k_means_plus_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		return -1;
	}
	struct k_means_plus_info* inf=(struct k_means_plus_info*) b->private_data;
	
	unsigned int clen;
	struct k_means_plus_config* k_means_plus_conf;
	
        k_means_plus_conf = (struct k_means_plus_config*) ubx_config_get_data_ptr(b, "k_means_plus_config", &clen);
	
	//inf->data_dim=k_means_plus_conf->data_dim;
	inf->n_clusters=k_means_plus_conf->n_clusters;
	init_xy_s(inf->pts,PTS);
	return ret;
}

/**
 * k_means_plus_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void k_means_plus_cleanup(ubx_block_t *b)
{
	DBG(" ");
	free(b->private_data);
}

/**
 * k_means_plus_start - start the k_means_plus block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int k_means_plus_start(ubx_block_t *b)
{
	DBG("in start");
	return 0; /* Ok */
}

/**
 * k_means_plus_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void k_means_plus_step(ubx_block_t *b) {
	
	int ret=0;
	struct k_means_plus_info* inf=(struct k_means_plus_info*) b->private_data;
	float data_in_val[DATA_LENGTH];
	float data_out_val[K*2];
	ubx_port_t* data_port = ubx_port_get(b, "data_in");
	ubx_port_t* cent_port = ubx_port_get(b, "data_out");
	ubx_port_t* n_clusters_out_port = ubx_port_get(b, "n_clusters_out");
	ubx_port_t* n_clusters_in_port = ubx_port_get(b, "n_clusters");
	int n_clusters;
	ret=read_n_clusters(n_clusters_in_port, &n_clusters);
	if(ret>0)
	inf->n_clusters=n_clusters;

	ret=read_float720(data_port, &data_in_val);
	if(!(ret>0))
	  return;
	for(int i=0;i<PTS;i++){
		if(fabs(data_in_val[i*2])<3000&&fabs(data_in_val[i*2+1])<3000)
		{
			inf->pts[i].x=data_in_val[i*2];
			inf->pts[i].y=data_in_val[i*2+1];
		}
		else
		{
                        inf->pts[i].x=0;
                        inf->pts[i].y=0;
		}	
	}
	lloyd_s(inf->pts, PTS, inf->n_clusters, inf->cent);
        for(int i=0;i<inf->n_clusters;i++){
                data_out_val[i*2]=inf->cent[i].x;
                data_out_val[i*2+1]=inf->cent[i].y;
        }	
	for(int i=0;i<inf->n_clusters;i++)
	//printf("center[%d]: (%12.3f,%12.3f)\n", i, data_out_val[i*2], data_out_val[i*2+1]);
	write_cent(cent_port, &data_out_val);
	write_n_clusters(n_clusters_out_port, &(inf->n_clusters));
}


/* put everything together
 *
 */
ubx_block_t k_means_plus_comp = {
	.name = "k_means_plus/k_means_plus",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = k_means_plus_meta,
	.configs = k_means_plus_config,
	.ports = k_means_plus_ports,

	/* ops */
	.init = k_means_plus_init,
	.start = k_means_plus_start,
	.step = k_means_plus_step,
	.cleanup = k_means_plus_cleanup,
};

/**
 * k_means_plus_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int k_means_plus_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &k_means_plus_config_type);
	return ubx_block_register(ni, &k_means_plus_comp);
}

/**
 * k_means_plus_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void k_means_plus_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct k_means_plus_config");
	ubx_block_unregister(ni, "k_means_plus/k_means_plus");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(k_means_plus_module_init)
UBX_MODULE_CLEANUP(k_means_plus_module_cleanup)
