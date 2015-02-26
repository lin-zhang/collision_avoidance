/*
 * A fblock that generates arduino_upload numbers.
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
#include "types/arduino_upload_config.h"
#include "types/arduino_upload_config.h.hexarr"

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t arduino_upload_config_type = def_struct_type(struct arduino_upload_config, &arduino_upload_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char arduino_upload_meta[] =
	"{ doc='A test block for arduino Uno board, driving adxl345 accelerometer sensor',"
	"  real-time=true,"
	"}";

/* configuration
 * upon cloning the following happens:
 *   - value.type is resolved
 *   - value.data will point to a buffer of size value.len*value.type->size
 *
 * if an array is required, then .value = { .len=<LENGTH> } can be used.
 */
ubx_config_t arduino_upload_config[] = {
	{ .name="arduino_upload_config", .type_name = "struct arduino_upload_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t arduino_upload_ports[] = {
	{ .name="seed", .in_type_name="unsigned int" },
	{ .name="rnd", .out_type_name="unsigned int" },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct arduino_upload_info {
	char * avrTool;
	char * micro_controller_model;
	char * configFilePath;
	int brate;
	char * portName;
	char * hexFile;
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */
//def_read_fun(read_uint, unsigned int)
//def_write_fun(write_uint, unsigned int)

/**
 * arduino_upload_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int arduino_upload_init(ubx_block_t *b)
{
	int ret=0;
	
	char os_cmd[512];

        DBG(" ");
        if ((b->private_data = calloc(1, sizeof(struct arduino_upload_info)))==NULL) {
                ERR("Failed to alloc memory");
                ret=EOUTOFMEM;
                goto out;
        }

        struct arduino_upload_info* inf=(struct arduino_upload_info*) b->private_data;
        unsigned int clen;
        struct arduino_upload_config* arduino_upload_conf;
        arduino_upload_conf = (struct arduino_upload_config*) ubx_config_get_data_ptr(b, "arduino_upload_config", &clen);

	printf("[DEBUG]:   %s", arduino_upload_conf->avrTool);

	inf->avrTool=arduino_upload_conf->avrTool;
	inf->micro_controller_model = arduino_upload_conf->micro_controller_model;
	inf->configFilePath = arduino_upload_conf->configFilePath;
	inf->brate = arduino_upload_conf->brate;
        inf->portName = arduino_upload_conf->portName;
	inf->hexFile = arduino_upload_conf->hexFile;	

	sprintf(os_cmd,"%s -q -V -D -p %s -C %s -c arduino -b %d -P %s -U flash:w:%s:i",  inf->avrTool,  inf->micro_controller_model,  inf->configFilePath,  inf->brate,  inf->portName,  inf->hexFile);
	//sprintf(os_cmd, "avrdude -q -V -D -p atmega328p -C /usr/share/arduino/hardware/tools/avr/../avrdude.conf -c arduino -b 115200 -P /dev/ttyACM0 -U flash:w:/home/zhanglin/work/embedded/arduino/microblx/arduino_blocks/ard_adxl345/build-uno/ard_adxl345.hex:i");
	system(os_cmd);

	DBG("[arduino debug info]uploaded program");
	if ((b->private_data = calloc(1, sizeof(struct arduino_upload_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}
 out:
	return ret;
}

/**
 * arduino_upload_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void arduino_upload_cleanup(ubx_block_t *b)
{
	DBG(" ");
	free(b->private_data);
}

/**
 * arduino_upload_start - start the arduino_upload block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int arduino_upload_start(ubx_block_t *b)
{
	DBG("in");
	return 0; /* Ok */
}

/**
 * arduino_upload_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void arduino_upload_step(ubx_block_t *b) {

}


/* put everything together
 *
 */
ubx_block_t arduino_upload_comp = {
	.name = "arduino_upload/arduino_upload",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = arduino_upload_meta,
	.configs = arduino_upload_config,
	.ports = arduino_upload_ports,

	/* ops */
	.init = arduino_upload_init,
	.start = arduino_upload_start,
	.step = arduino_upload_step,
	.cleanup = arduino_upload_cleanup,
};

/**
 * arduino_upload_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int arduino_upload_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &arduino_upload_config_type);
	return ubx_block_register(ni, &arduino_upload_comp);
}

/**
 * arduino_upload_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void arduino_upload_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct arduino_upload_config");
	ubx_block_unregister(ni, "arduino_upload/arduino_upload");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(arduino_upload_module_init)
UBX_MODULE_CLEANUP(arduino_upload_module_cleanup)
