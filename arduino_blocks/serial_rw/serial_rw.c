/*
 * A fblock that generates serial_rw numbers.
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
#include "types/serial_rw_config.h"
#include "types/serial_rw_config.h.hexarr"

//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t serial_rw_config_type = def_struct_type(struct serial_rw_config, &serial_rw_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char serial_rw_meta[] =
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
ubx_config_t serial_rw_config[] = {
	{ .name="serial_rw_config", .type_name = "struct serial_rw_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t serial_rw_ports[] = {
	{ .name="seed", .in_type_name="unsigned int" },
	{ .name="rnd", .out_type_name="unsigned int" },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct serial_rw_info {
	int fd;
	char* portName;
	int brate;
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */
//def_read_fun(read_uint, unsigned int)
//def_write_fun(write_uint, unsigned int)

/**
 * serial_rw_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int serial_rw_init(ubx_block_t *b)
{
	int ret=0;

	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct serial_rw_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}

	struct serial_rw_info* inf=(struct serial_rw_info*) b->private_data;
	unsigned int clen;
	struct serial_rw_config* serial_rw_conf;
        serial_rw_conf = (struct serial_rw_config*) ubx_config_get_data_ptr(b, "serial_rw_config", &clen);

	inf->portName = serial_rw_conf->portName;

	switch(serial_rw_conf->brate){
		case 9600:
		default:
			inf->brate=B9600;
		break; 
		case 38400:
                        inf->brate=B38400;
		break;
		case 57600:
                        inf->brate=B57600;
		break;
		case 115200:
                        inf->brate=B115200;
		break;

	}

 //  	inf->fd = open((const char*)inf->portName, O_RDWR | O_NOCTTY | O_NDELAY);
        inf->fd = open((const char*)inf->portName, O_RDWR | O_NONBLOCK);

   	if (inf->fd == -1)
   	{                                              /* Could not open the port */
     		fprintf(stderr, "open_port: Unable to open %s - %s\n", (const char*)inf->portName,
             	strerror(errno));
   	}

 out:
	return ret;
}

/**
 * serial_rw_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void serial_rw_cleanup(ubx_block_t *b)
{
	DBG(" ");
        struct serial_rw_info* inf=(struct serial_rw_info*) b->private_data;
	close(inf->fd);
	free(b->private_data);
}

/**
 * serial_rw_start - start the serial_rw block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int serial_rw_start(ubx_block_t *b)
{
	DBG("in");

        struct serial_rw_info* inf=(struct serial_rw_info*) b->private_data;

	
	struct termios options;

	tcgetattr(inf->fd, &options);
//	cfsetispeed(&options, B9600);                 /* Set the baud rates to 9600 */
//	cfsetospeed(&options, B9600);
	cfsetispeed(&options, inf->brate);

	                                  /* Enable the receiver and set local mode */
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |=  CS8;                              /* Select 8 data bits */
	options.c_cflag &= ~CRTSCTS;               /* Disable hardware flow control */  
	
	                                /* Enable data to be processed as raw input */
	options.c_lflag &= ~(ICANON | ECHO | ISIG);
	      
	                                       /* Set the new options for the port */
	tcsetattr(inf->fd, TCSANOW, &options);
                         

	inf=(struct serial_rw_info*) b->private_data;
	fcntl(inf->fd, F_SETFL, FNDELAY);	
	return 0; /* Ok */
}

/**
 * serial_rw_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void serial_rw_step(ubx_block_t *b) {
	char chout;
        struct serial_rw_info* inf=(struct serial_rw_info*) b->private_data;
	
	while(chout!='\n'){
		read(inf->fd, &chout, sizeof(chout));
		printf("%c", chout);
		usleep(2500);
	}
	printf("\n");
	
	write(inf->fd, "Received!\n", 10);
	usleep((10+25)*100);
	
}


/* put everything together
 *
 */
ubx_block_t serial_rw_comp = {
	.name = "serial_rw/serial_rw",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = serial_rw_meta,
	.configs = serial_rw_config,
	.ports = serial_rw_ports,

	/* ops */
	.init = serial_rw_init,
	.start = serial_rw_start,
	.step = serial_rw_step,
	.cleanup = serial_rw_cleanup,
};

/**
 * serial_rw_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int serial_rw_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &serial_rw_config_type);
	return ubx_block_register(ni, &serial_rw_comp);
}

/**
 * serial_rw_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void serial_rw_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct serial_rw_config");
	ubx_block_unregister(ni, "serial_rw/serial_rw");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(serial_rw_module_init)
UBX_MODULE_CLEANUP(serial_rw_module_cleanup)
