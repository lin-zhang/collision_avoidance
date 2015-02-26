/*
 * A fblock that generates str_spliter numbers.
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
#include "types/str_spliter_config.h"
#include "types/str_spliter_config.h.hexarr"

//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define SERIAL_IN_CHAR_LEN 128
#define DATA_OUT_ARR_SIZE 13

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t str_spliter_config_type = def_struct_type(struct str_spliter_config, &str_spliter_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char str_spliter_meta[] =
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
ubx_config_t str_spliter_config[] = {
	{ .name="str_spliter_config", .type_name = "struct str_spliter_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t str_spliter_ports[] = {
	{ .name="nData", .out_type_name="unsigned int" },
	{ .name="rnd", .out_type_name="unsigned int" },
	{ .name="string_in", .in_type_name="char", .in_data_len=SERIAL_IN_CHAR_LEN},
        { .name="data_out", .out_type_name="int", .out_data_len=DATA_OUT_ARR_SIZE},
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
struct str_spliter_info {
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

/**
 * str_spliter_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int str_spliter_init(ubx_block_t *b)
{
	int ret=0;

	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct str_spliter_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}

	struct str_spliter_info* inf=(struct str_spliter_info*) b->private_data;
	unsigned int clen;
	struct str_spliter_config* str_spliter_conf;
        str_spliter_conf = (struct str_spliter_config*) ubx_config_get_data_ptr(b, "str_spliter_config", &clen);
	inf->delim[0] = str_spliter_conf->delim[0];
 out:
	return ret;
}

/**
 * str_spliter_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void str_spliter_cleanup(ubx_block_t *b)
{
	DBG(" ");
        //struct str_spliter_info* inf=(struct str_spliter_info*) b->private_data;
	free(b->private_data);
}

/**
 * str_spliter_start - start the str_spliter block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int str_spliter_start(ubx_block_t *b)
{
	DBG("in");
        //struct str_spliter_info* inf=(struct str_spliter_info*) b->private_data;
	//inf=(struct str_spliter_info*) b->private_data;
	return 0; /* Ok */
}

/**
 * str_spliter_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
int counter_a=0;
static void str_spliter_step(ubx_block_t *b) {
	int j=0;
	char** tokens;
	unsigned int nSubStr;
	char buf[SERIAL_IN_CHAR_LEN];
	int data_out[DATA_OUT_ARR_SIZE];
	ubx_port_t* string_in_port=ubx_port_get(b, "string_in");
	ubx_port_t* data_out_port=ubx_port_get(b, "data_out");
	ubx_port_t* nData_port=ubx_port_get(b, "nData");

        struct str_spliter_info* inf=(struct str_spliter_info*) b->private_data;
	read_char(string_in_port, &buf);
	
		//printf("[str_%8d] %s\n", counter_a, buf);
		//printf("[str_%8d]", counter_a);		
		tokens=str_split(buf,inf->delim[0],&nSubStr);
		printf("nSubStr=%d\n",nSubStr);
		for(j=0;j<nSubStr-1;j++){
		    //printf("%s ",*(tokens+j));
		    data_out[j]=atoi(*(tokens+j));
		}
		/*
		printf("\n");
                for(j=0;j<nSubStr-1;j++){
                    printf("%d ",atoi(*(tokens+j)));
                }		
                printf("\n");
		*/
		counter_a++;
		write_int(data_out_port, &data_out);
		write_uint(nData_port,&nSubStr);

	return;
}


/* put everything together
 *
 */
ubx_block_t str_spliter_comp = {
	.name = "str_spliter/str_spliter",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = str_spliter_meta,
	.configs = str_spliter_config,
	.ports = str_spliter_ports,

	/* ops */
	.init = str_spliter_init,
	.start = str_spliter_start,
	.step = str_spliter_step,
	.cleanup = str_spliter_cleanup,
};

/**
 * str_spliter_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int str_spliter_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &str_spliter_config_type);
	return ubx_block_register(ni, &str_spliter_comp);
}

/**
 * str_spliter_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void str_spliter_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct str_spliter_config");
	ubx_block_unregister(ni, "str_spliter/str_spliter");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(str_spliter_module_init)
UBX_MODULE_CLEANUP(str_spliter_module_cleanup)
