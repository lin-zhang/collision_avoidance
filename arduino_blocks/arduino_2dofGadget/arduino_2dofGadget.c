/*
 * A fblock that generates arduino_2dofGadget numbers.
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
#include "types/arduino_2dofGadget_config.h"
#include "types/arduino_2dofGadget_config.h.hexarr"

//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <math.h>


#define BUFF_LEN 5
#define MA_FILTER 1
#ifdef MA_FILTER
float IMU_buffer[BUFF_LEN][2];
int buffer_i;
#endif

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t arduino_2dofGadget_config_type = def_struct_type(struct arduino_2dofGadget_config, &arduino_2dofGadget_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char arduino_2dofGadget_meta[] =
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
ubx_config_t arduino_2dofGadget_config[] = {
	{ .name="arduino_2dofGadget_config", .type_name = "struct arduino_2dofGadget_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t arduino_2dofGadget_ports[] = {
	{ .name="seed", .in_type_name="unsigned int" },
	{ .name="rnd", .out_type_name="unsigned int" },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct arduino_2dofGadget_info {
	int fd;
	char* portName;
	int brate;
};

	float ang[2];
	int meas[3];

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

	
	//printf("%s", a_str);
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

    return result;
}


/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */
//def_read_fun(read_uint, unsigned int)
//def_write_fun(write_uint, unsigned int)

/**
 * arduino_2dofGadget_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int arduino_2dofGadget_init(ubx_block_t *b)
{
	int ret=0;

	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct arduino_2dofGadget_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}

	struct arduino_2dofGadget_info* inf=(struct arduino_2dofGadget_info*) b->private_data;
	unsigned int clen;
	struct arduino_2dofGadget_config* arduino_2dofGadget_conf;
        arduino_2dofGadget_conf = (struct arduino_2dofGadget_config*) ubx_config_get_data_ptr(b, "arduino_2dofGadget_config", &clen);

	inf->portName = arduino_2dofGadget_conf->portName;

	switch(arduino_2dofGadget_conf->brate){
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
 * arduino_2dofGadget_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void arduino_2dofGadget_cleanup(ubx_block_t *b)
{
	DBG(" ");
        struct arduino_2dofGadget_info* inf=(struct arduino_2dofGadget_info*) b->private_data;
	close(inf->fd);
	free(b->private_data);
}

/**
 * arduino_2dofGadget_start - start the arduino_2dofGadget block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int arduino_2dofGadget_start(ubx_block_t *b)
{
	DBG("in");

        struct arduino_2dofGadget_info* inf=(struct arduino_2dofGadget_info*) b->private_data;

	
	struct termios options;

	tcgetattr(inf->fd, &options);
	cfsetispeed(&options, inf->brate);
	cfsetospeed(&options, inf->brate);

	                                  /* Enable the receiver and set local mode */
	options.c_cflag |= (CLOCAL | CREAD);
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
	/* Set the new options for the port */
	tcsetattr(inf->fd, TCSANOW, &options);
	inf=(struct arduino_2dofGadget_info*) b->private_data;
	//fcntl(inf->fd, F_SETFL, FNDELAY);	
	return 0; /* Ok */
}

/**
 * arduino_2dofGadget_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void arduino_2dofGadget_step(ubx_block_t *b) {
	float float_result[2];
	char** tokens;
	char buf[128];
	char bb[1];
	char str2ard[11];
	int i=0;
	int buf_max = 128;
        struct arduino_2dofGadget_info* inf=(struct arduino_2dofGadget_info*) b->private_data;
	do {
        	int n = read(inf->fd, bb, 1);  // read a char at a time
	        if( n==-1) goto out;    // couldn't read
        	if( n==0 ) {
            		usleep( 1 * 10 );  // wait 1 msec try again
            		continue;
        	}
        	buf[i] = bb[0];
        	i++;
    	} while( bb[0] != '\n' && i < buf_max );
	
	tokens=str_split(buf,',');
	if(strcmp(*tokens,"[R]")==0){
	//printf("Received from Arduino: %s,%s,%s,%s,%s,%s,%s\n", *tokens, *(tokens+1),*(tokens+2), *(tokens+3),*(tokens+4),*(tokens+5), *(tokens+6));
	//printf("%s", buf);
	if((*tokens+1)&&(*tokens+2)&&(*tokens+3)){
		for(i=0;i<3;i++)
			meas[i]=atoi(*(tokens+i+1));
	ang[0]=atan2((float)meas[2],(float)meas[0])*180/3.1415926;
	ang[1]=atan2((float)meas[2],(float)meas[1])*180/3.1415926;
	}
	}

#ifdef MA_FILTER
        if(buffer_i<BUFF_LEN-1) buffer_i++; else buffer_i=0;
        IMU_buffer[buffer_i][0]=ang[0];
        IMU_buffer[buffer_i][1]=ang[1];

        for(i=0;i<BUFF_LEN;i++){
 	       float_result[0]+=IMU_buffer[i][0];
 	       float_result[1]+=IMU_buffer[i][1];
        }
	
	float_result[0]/=BUFF_LEN;
	float_result[1]/=BUFF_LEN;
#else
	float_result[0]=ang[0];
	float_result[1]=ang[1];
#endif
	printf("calculated angles: %f,%f\n", float_result[0],float_result[1]);
	sprintf(str2ard, "[C],%3d,%3d\n", (int)float_result[0],(int)float_result[1]);
	write(inf->fd, str2ard, 11);
out:        
	return;
}


/* put everything together
 *
 */
ubx_block_t arduino_2dofGadget_comp = {
	.name = "arduino_2dofGadget/arduino_2dofGadget",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = arduino_2dofGadget_meta,
	.configs = arduino_2dofGadget_config,
	.ports = arduino_2dofGadget_ports,

	/* ops */
	.init = arduino_2dofGadget_init,
	.start = arduino_2dofGadget_start,
	.step = arduino_2dofGadget_step,
	.cleanup = arduino_2dofGadget_cleanup,
};

/**
 * arduino_2dofGadget_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int arduino_2dofGadget_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &arduino_2dofGadget_config_type);
	return ubx_block_register(ni, &arduino_2dofGadget_comp);
}

/**
 * arduino_2dofGadget_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void arduino_2dofGadget_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct arduino_2dofGadget_config");
	ubx_block_unregister(ni, "arduino_2dofGadget/arduino_2dofGadget");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(arduino_2dofGadget_module_init)
UBX_MODULE_CLEANUP(arduino_2dofGadget_module_cleanup)
