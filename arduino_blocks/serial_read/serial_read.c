/*
 * A fblock that generates serial_read numbers.
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
#include "types/serial_read_config.h"
#include "types/serial_read_config.h.hexarr"

//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t serial_read_config_type = def_struct_type(struct serial_read_config, &serial_read_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char serial_read_meta[] =
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
ubx_config_t serial_read_config[] = {
	{ .name="serial_read_config", .type_name = "struct serial_read_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t serial_read_ports[] = {
	{ .name="seed", .in_type_name="unsigned int" },
	{ .name="rnd", .out_type_name="unsigned int" },
	{ .name="string_read", .out_type_name="char", .out_data_len=128},
	{ NULL },
};

char** str_split(char* a_str, const char a_delim, int* nSubStr)
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
struct serial_read_info {
	int fd;
	char* portName;
	int brate;
};
static int start_flag=0;
/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */
//def_read_fun(read_uint, unsigned int)
//def_write_fun(write_uint, unsigned int)
def_write_arr_fun(write_char128, char, 128)

/**
 * serial_read_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int serial_read_init(ubx_block_t *b)
{
	int ret=0;

	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct serial_read_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}

	struct serial_read_info* inf=(struct serial_read_info*) b->private_data;
	unsigned int clen;
	struct serial_read_config* serial_read_conf;
        serial_read_conf = (struct serial_read_config*) ubx_config_get_data_ptr(b, "serial_read_config", &clen);

	inf->portName = serial_read_conf->portName;

	switch(serial_read_conf->brate){
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

  	inf->fd = open((const char*)inf->portName, O_RDWR | O_NOCTTY | O_NDELAY);
//        inf->fd = open((const char*)inf->portName, O_RDWR | O_NONBLOCK);

   	if (inf->fd == -1)
   	{                                              /* Could not open the port */
     		fprintf(stderr, "open_port: Unable to open %s - %s\n", (const char*)inf->portName,
             	strerror(errno));
   	}
	
	start_flag=0;
 out:
	return ret;
}

/**
 * serial_read_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void serial_read_cleanup(ubx_block_t *b)
{
	DBG(" ");
        struct serial_read_info* inf=(struct serial_read_info*) b->private_data;
	close(inf->fd);
	free(b->private_data);
}

/**
 * serial_read_start - start the serial_read block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int serial_read_start(ubx_block_t *b)
{
	DBG("in");

        struct serial_read_info* inf=(struct serial_read_info*) b->private_data;

	
	struct termios options;

	tcgetattr(inf->fd, &options);
//	cfsetispeed(&options, B9600);                 /* Set the baud rates to 9600 */
//	cfsetospeed(&options, B9600);
	cfsetispeed(&options, inf->brate);
	cfsetospeed(&options, inf->brate);

	                                  /* Enable the receiver and set local mode */
	//options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |=  CS8;                              /* Select 8 data bits */
	options.c_cflag &= ~CRTSCTS;               /* Disable hardware flow control */  
	
	                                /* Enable data to be processed as raw input */
	//options.c_lflag &= ~(ICANON | ECHO | ISIG);
    	options.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    	options.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    	options.c_oflag &= ~OPOST; // make raw
	 
    	options.c_cc[VMIN]  = 0;
    	options.c_cc[VTIME] = 0;     

	                                       /* Set the new options for the port */

	tcsetattr(inf->fd, TCSANOW, &options);
    	if( tcsetattr(inf->fd, TCSAFLUSH, &options) < 0) {
        	perror("init_serialport: Couldn't set term attributes");
        	return -1;
    	}
                 

	inf=(struct serial_read_info*) b->private_data;
	//fcntl(inf->fd, F_SETFL, FNDELAY);	
	return 0; /* Ok */
}

/**
 * serial_read_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
int counter_a=0;
static void serial_read_step(ubx_block_t *b) {
	char buf[128];
	//char** tokens;
	char bb[1];
	int i=0;
	//int j=0;
	int buf_max = 128;
	//int nSubStr=0;
	ubx_port_t* string_read_port=ubx_port_get(b, "string_read");


	memset(buf,0,buf_max); 
        struct serial_read_info* inf=(struct serial_read_info*) b->private_data;
	do {
        	int n = read(inf->fd, bb, 1);  // read a char at a time
	        if( n==-1) goto out;    // couldn't read
        	if( n==0 ) {
            		usleep( 1 * 1000 );  // wait 1 msec try again
            		continue;
        	}
        	buf[i] = bb[0];
        	i++;
    	} while( bb[0] != '\n' && i < buf_max );
	
	write_char128(string_read_port, &buf);
	
		//printf("[read_from_serial %8d] %s", counter_a, buf);
		
		//printf("[%8d] ",counter_a);	
		//tokens=str_split(buf,',',&nSubStr);
		/*
		for(j=0;j<nSubStr-1;j++){
		    printf("%s ",*(tokens+j));
		}
		printf("\n");
                for(j=0;j<nSubStr-1;j++){
                    printf("%d ",atoi(*(tokens+j)));
                }		
                printf("\n");
		*/	
		counter_a++;
out:        
	return;
}


/* put everything together
 *
 */
ubx_block_t serial_read_comp = {
	.name = "serial_read/serial_read",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = serial_read_meta,
	.configs = serial_read_config,
	.ports = serial_read_ports,

	/* ops */
	.init = serial_read_init,
	.start = serial_read_start,
	.step = serial_read_step,
	.cleanup = serial_read_cleanup,
};

/**
 * serial_read_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int serial_read_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &serial_read_config_type);
	return ubx_block_register(ni, &serial_read_comp);
}

/**
 * serial_read_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void serial_read_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct serial_read_config");
	ubx_block_unregister(ni, "serial_read/serial_read");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(serial_read_module_init)
UBX_MODULE_CLEANUP(serial_read_module_cleanup)
