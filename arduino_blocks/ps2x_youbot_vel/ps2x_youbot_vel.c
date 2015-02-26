/*
 * A fblock that generates ps2x_youbot_vel numbers.
 *
 * This is to be a well (over) documented block to serve as a good
 * example.
 */

#define DEBUG 1 

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ubx.h"
#include "../../std_blocks/youbot_driver/youbot_driver.h"
//#include "types/youbot_control_modes.h"
//#include "types/motionctrl_jnt_state.h"
/* declare and initialize a microblx type. This will be registered /
 * deregistered in the module init / cleanup at the end of this
 * file.
 *
 * Include regular header file and it's char array representation
 * (used for luajit reflection, logging, etc.)
 */
#include "types/ps2x_youbot_vel_config.h"
#include "types/ps2x_youbot_vel_config.h.hexarr"

//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
/*
struct kdl_vector {
	double x;
	double y;
	double z;
};

struct kdl_twist {
	struct kdl_vector vel;
	struct kdl_vector rot;
};
*/

#define SERIAL_IN_CHAR_LEN 128
#define DATA_OUT_ARR_SIZE 13

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t ps2x_youbot_vel_config_type = def_struct_type(struct ps2x_youbot_vel_config, &ps2x_youbot_vel_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char ps2x_youbot_vel_meta[] =
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
ubx_config_t ps2x_youbot_vel_config[] = {
	{ .name="ps2x_youbot_vel_config", .type_name = "struct ps2x_youbot_vel_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t ps2x_youbot_vel_ports[] = {
	{ .name="nData", .out_type_name="unsigned int" },
	{ .name="rnd", .out_type_name="unsigned int" },
	{ .name="string_in", .in_type_name="char", .in_data_len=SERIAL_IN_CHAR_LEN},
        { .name="data_out", .out_type_name="int", .out_data_len=DATA_OUT_ARR_SIZE},
	{ .name="base_cmd_vel", .out_type_name="int32_t", .out_data_len=4 },
	{ NULL },
};

char** str_split(char* a_str, const char a_delim, unsigned int* nSubStr)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    *nSubStr=count;
    return result;
}


/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct ps2x_youbot_vel_info {
	char delim[1];
};
//static int start_flag=0;
/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */
//def_read_fun(read_uint, unsigned int)
//def_write_fun(write_uint, unsigned int)
//def_write_arr_fun(write_char128, char, 128)
def_read_arr_fun(read_char, char, SERIAL_IN_CHAR_LEN)
def_write_arr_fun(write_int, int, DATA_OUT_ARR_SIZE)
def_write_fun(write_uint, unsigned int)
//def_write_fun(write_kdl_twist, struct kdl_twist)
def_write_arr_fun(write_vel, int32_t,4)

/**
 * ps2x_youbot_vel_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int ps2x_youbot_vel_init(ubx_block_t *b)
{
	int ret=0;

	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct ps2x_youbot_vel_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}

	struct ps2x_youbot_vel_info* inf=(struct ps2x_youbot_vel_info*) b->private_data;
	unsigned int clen;
	struct ps2x_youbot_vel_config* ps2x_youbot_vel_conf;
        ps2x_youbot_vel_conf = (struct ps2x_youbot_vel_config*) ubx_config_get_data_ptr(b, "ps2x_youbot_vel_config", &clen);
	inf->delim[0] = ps2x_youbot_vel_conf->delim[0];
 out:
	return ret;
}

/**
 * ps2x_youbot_vel_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void ps2x_youbot_vel_cleanup(ubx_block_t *b)
{
	DBG(" ");
        //struct ps2x_youbot_vel_info* inf=(struct ps2x_youbot_vel_info*) b->private_data;
	free(b->private_data);
}

/**
 * ps2x_youbot_vel_start - start the ps2x_youbot_vel block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int ps2x_youbot_vel_start(ubx_block_t *b)
{
	DBG("in");
        //struct ps2x_youbot_vel_info* inf=(struct ps2x_youbot_vel_info*) b->private_data;
	//inf=(struct ps2x_youbot_vel_info*) b->private_data;
	return 0; /* Ok */
}

/**
 * ps2x_youbot_vel_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
int counter_a=0;
static void ps2x_youbot_vel_step(ubx_block_t *b) {
	int j=0;
	char** tokens;
	unsigned int nSubStr;
	char buf[SERIAL_IN_CHAR_LEN];
	int data_out[DATA_OUT_ARR_SIZE];
	int32_t cmd_val[4];
	ubx_port_t* string_in_port=ubx_port_get(b, "string_in");
	ubx_port_t* data_out_port=ubx_port_get(b, "data_out");
	ubx_port_t* nData_port=ubx_port_get(b, "nData");
	ubx_port_t* cmd_vel_port=ubx_port_get(b, "base_cmd_vel");
	struct kdl_twist cmd_twist;
	int fast_forward=1.0;
	cmd_twist.vel.x=0;
	cmd_twist.vel.y=0;
	cmd_twist.vel.z=0;
	cmd_twist.rot.x=0;
	cmd_twist.rot.y=0;
	cmd_twist.rot.z=0;

	
        struct ps2x_youbot_vel_info* inf=(struct ps2x_youbot_vel_info*) b->private_data;
	read_char(string_in_port, &buf);
	
		//printf("[str_%8d] %s\n", counter_a, buf);
		//printf("[str_%8d]", counter_a);		
		tokens=str_split(buf,inf->delim[0],&nSubStr);
		//printf("nSubStr=%d\n",nSubStr);
		for(j=0;j<nSubStr-1;j++){
		//   printf("%s ",*(tokens+j));
		    data_out[j]=atoi(*(tokens+j));
		}
		/*printf("\n");
                for(j=0;j<nSubStr-1;j++){
                    printf("[%d] %d %d ",j, data_out[j], atoi(*(tokens+j)));
                }		
                printf("\n");*/

                if((unsigned char)data_out[0]==0xFF&&(unsigned char)data_out[1]==0x79&&(unsigned char)data_out[2]==0x5A){
                //printf("with PS2X!\n");
		
		if(((unsigned char)data_out[4]&0x40)==0x00){
			fast_forward=2.0;
		}
		
		if(((unsigned char)data_out[3]&0x10)==0x00){
		cmd_twist.vel.x+=0.2*fast_forward;
		printf("forward\n");
		}
		if(((unsigned char)data_out[3]&0x40)==0x00){
		cmd_twist.vel.x-=0.2*fast_forward;
		printf("backward\n");
		}

		if(((unsigned char)data_out[3]&0x20)==0x00){
		cmd_twist.vel.y+=0.2*fast_forward;
		printf("left\n");
		}
		if(((unsigned char)data_out[3]&0x80)==0x00){
		cmd_twist.vel.y-=0.2*fast_forward;
		printf("right\n");
		}

		cmd_twist.vel.z=0;
		
		cmd_twist.rot.x=0;
		cmd_twist.rot.y=0;
	
		if((unsigned char)data_out[5]>0x90){
		cmd_twist.rot.z+=0.1;
		printf("steer left\n");
		}
		if((unsigned char)data_out[5]<0x70){
		cmd_twist.vel.z-=0.1;
		printf("steer right\n");
		}
		}
		
		cmd_val[0] = ( -cmd_twist.vel.x + cmd_twist.vel.y + cmd_twist.rot.z *
		            YOUBOT_ANGULAR_TO_WHEEL_VELOCITY ) * YOUBOT_CARTESIAN_VELOCITY_TO_RPM;
		
		cmd_val[1] = ( cmd_twist.vel.x + cmd_twist.vel.y + cmd_twist.rot.z *
		            YOUBOT_ANGULAR_TO_WHEEL_VELOCITY ) * YOUBOT_CARTESIAN_VELOCITY_TO_RPM;
		
		cmd_val[2] = ( -cmd_twist.vel.x - cmd_twist.vel.y + cmd_twist.rot.z  *
		            YOUBOT_ANGULAR_TO_WHEEL_VELOCITY ) * YOUBOT_CARTESIAN_VELOCITY_TO_RPM;
		
		cmd_val[3] = ( cmd_twist.vel.x - cmd_twist.vel.y + cmd_twist.rot.z *
                                       YOUBOT_ANGULAR_TO_WHEEL_VELOCITY ) * YOUBOT_CARTESIAN_VELOCITY_TO_RPM;

		

		counter_a++;
		write_int(data_out_port, &data_out);
		write_uint(nData_port,&nSubStr);
		//write_kdl_twist(cmd_twist_port,&cmd_twist);
		write_vel(cmd_vel_port,&cmd_val);
		return;
}


/* put everything together
 *
 */
ubx_block_t ps2x_youbot_vel_comp = {
	.name = "ps2x_youbot_vel/ps2x_youbot_vel",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = ps2x_youbot_vel_meta,
	.configs = ps2x_youbot_vel_config,
	.ports = ps2x_youbot_vel_ports,

	/* ops */
	.init = ps2x_youbot_vel_init,
	.start = ps2x_youbot_vel_start,
	.step = ps2x_youbot_vel_step,
	.cleanup = ps2x_youbot_vel_cleanup,
};

/**
 * ps2x_youbot_vel_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int ps2x_youbot_vel_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &ps2x_youbot_vel_config_type);
	return ubx_block_register(ni, &ps2x_youbot_vel_comp);
}

/**
 * ps2x_youbot_vel_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void ps2x_youbot_vel_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct ps2x_youbot_vel_config");
	ubx_block_unregister(ni, "ps2x_youbot_vel/ps2x_youbot_vel");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(ps2x_youbot_vel_module_init)
UBX_MODULE_CLEANUP(ps2x_youbot_vel_module_cleanup)
