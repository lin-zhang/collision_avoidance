/*
 * path_finding
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
#include "types/path_finding_config.h"
#include "types/path_finding_config.h.hexarr"

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

typedef int vertex_t;
typedef double weight_t;

const weight_t max_weight = std::numeric_limits<double>::infinity();

struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
};

typedef std::vector<std::vector<neighbor> > adjacency_list_t;


void DijkstraComputePaths(vertex_t source,
                          const adjacency_list_t &adjacency_list,
                          std::vector<weight_t> &min_distance,
                          std::vector<vertex_t> &previous)
{
    int n = adjacency_list.size();
    min_distance.clear();
    min_distance.resize(n, max_weight);
    min_distance[source] = 0;
    previous.clear();
    previous.resize(n, -1);
    std::set<std::pair<weight_t, vertex_t> > vertex_queue;
    vertex_queue.insert(std::make_pair(min_distance[source], source));

    while (!vertex_queue.empty())
    {
        weight_t dist = vertex_queue.begin()->first;
        vertex_t u = vertex_queue.begin()->second;
        vertex_queue.erase(vertex_queue.begin());

        // Visit each edge exiting u
	const std::vector<neighbor> &neighbors = adjacency_list[u];
        for (std::vector<neighbor>::const_iterator neighbor_iter = neighbors.begin();
             neighbor_iter != neighbors.end();
             neighbor_iter++)
        {
            vertex_t v = neighbor_iter->target;
            weight_t weight = neighbor_iter->weight;
            weight_t distance_through_u = dist + weight;
	    if (distance_through_u < min_distance[v]) {
	        vertex_queue.erase(std::make_pair(min_distance[v], v));

	        min_distance[v] = distance_through_u;
	        previous[v] = u;
	        vertex_queue.insert(std::make_pair(min_distance[v], v));

	    }

        }
    }
}


std::list<vertex_t> DijkstraGetShortestPathTo(
    vertex_t vertex, const std::vector<vertex_t> &previous)
{
    std::list<vertex_t> path;
    for ( ; vertex != -1; vertex = previous[vertex])
        path.push_front(vertex);
    return path;
}

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t path_finding_config_type = def_struct_type(struct path_finding_config, &path_finding_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char path_finding_meta[] =
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
ubx_config_t path_finding_config[] = {
	{ .name="path_finding_config", .type_name = "struct path_finding_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t path_finding_ports[] = {
	{ .name="wayPoints", .in_type_name="float", .in_data_len=NUM_NODES*2},
	{ .name="costTable", .in_type_name="float", .in_data_len=NUM_NODES*NUM_NODES},
	{ .name="pathWayPoints", .out_type_name="float", .out_data_len=NUM_NODES*2},
	{ .name="nValidNodes", 	.out_type_name="int" },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct path_finding_info {
	float data[DATA_LENGTH];
        int nCenters;
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */

def_read_arr_fun(read_wayPoints, float, NUM_NODES*2)
def_read_arr_fun(read_costTable, float, NUM_NODES*NUM_NODES)
def_write_arr_fun(write_pathWayPoints, float, NUM_NODES*2)
def_write_fun(write_nValidNodes, int)
/**
 * path_finding_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int path_finding_init(ubx_block_t *b)
{
	int ret=0;
	DBG(" ");

	if ((b->private_data = calloc(1, sizeof(struct path_finding_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		return -1;
	}
	struct path_finding_info* inf=(struct path_finding_info*) b->private_data;
	
	unsigned int clen;
	struct path_finding_config* path_finding_conf;
	
        path_finding_conf = (struct path_finding_config*) ubx_config_get_data_ptr(b, "path_finding_config", &clen);
	inf->nCenters=path_finding_conf->nCenters;
	return ret;
}

/**
 * path_finding_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void path_finding_cleanup(ubx_block_t *b)
{
	DBG(" ");
	free(b->private_data);
}

/**
 * path_finding_start - start the path_finding block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int path_finding_start(ubx_block_t *b)
{
	DBG("in start");
	return 0; /* Ok */
}

/**
 * path_finding_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void path_finding_step(ubx_block_t *b) {

    	std::vector<weight_t> min_distance;
    	std::vector<vertex_t> previous;
    	std::list<vertex_t> path;
    	vertex_t *arr = new int[path.size()];
	
	int nValidNodes=0;
	float costTable[NUM_NODES*NUM_NODES];
	float wayPoints[NUM_NODES*2];	
	float pathWayPoints[NUM_NODES*2];	

        ubx_port_t* p_costTable = ubx_port_get(b, "costTable");
        ubx_port_t* p_wayPoints = ubx_port_get(b, "wayPoints");

        ubx_port_t* p_nValidNodes = ubx_port_get(b, "nValidNodes");
        ubx_port_t* p_pathWayPoints = ubx_port_get(b, "pathWayPoints");
	
	read_costTable(p_costTable, &costTable);
	read_wayPoints(p_wayPoints, &wayPoints);
	
	adjacency_list_t adjacency_list(NUM_NODES);
	for(int m=0;m<NUM_NODES;m++){
		for(int n=0;n<NUM_NODES;n++){
			if(costTable[m*NUM_NODES+n]<INFINITY)
				adjacency_list[m].push_back(neighbor(n,costTable[m*NUM_NODES+n]));
		}
	}

    	DijkstraComputePaths(0, adjacency_list, min_distance, previous);
    	std::cout << "Distance from 0 to " << NUM_NODES-1 << ": " << min_distance[NUM_NODES-1] << std::endl;
    	path = DijkstraGetShortestPathTo(NUM_NODES-1, previous);
	std::cout << "Path : ";
    	std::copy(path.begin(), path.end(), std::ostream_iterator<vertex_t>(std::cout, " "));
    	std::cout << std::endl;
    	copy(path.begin(),path.end(),arr);
	nValidNodes=0;
	for(int i=0;i<NUM_NODES;i++){

		if((unsigned int)*(arr+i)<NUM_NODES){
			nValidNodes++;
			pathWayPoints[i*2]=wayPoints[*(arr+i)*2];
                        pathWayPoints[i*2+1]=wayPoints[*(arr+i)*2+1];
		}
		DBG("%d, %d, %12.3f, %12.3f",*(arr+i), i, pathWayPoints[i*2],pathWayPoints[i*2+1]);
		if((unsigned int)*(arr+i)==NUM_NODES-1)
			break;
	
	}
	write_nValidNodes(p_nValidNodes, &nValidNodes);
	write_pathWayPoints(p_pathWayPoints, &pathWayPoints);
	
	delete(arr);
}


/* put everything together
 *
 */
ubx_block_t path_finding_comp = {
	.name = "path_finding/path_finding",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = path_finding_meta,
	.configs = path_finding_config,
	.ports = path_finding_ports,

	/* ops */
	.init = path_finding_init,
	.start = path_finding_start,
	.step = path_finding_step,
	.cleanup = path_finding_cleanup,
};

/**
 * path_finding_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int path_finding_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &path_finding_config_type);
	return ubx_block_register(ni, &path_finding_comp);
}

/**
 * path_finding_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void path_finding_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct path_finding_config");
	ubx_block_unregister(ni, "path_finding/path_finding");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(path_finding_module_init)
UBX_MODULE_CLEANUP(path_finding_module_cleanup)
