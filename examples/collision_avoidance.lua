#!/usr/bin/env luajit

local ffi = require("ffi")
local ubx = require "ubx"
--local ubx_utils = require("ubx_utils")
local ts = tostring

ni=ubx.node_create("testnode")

ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/testtypes/testtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic_raw.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "app_blocks/rplidar_udp/rplidar_udp.so")
ubx.load_module(ni, "app_blocks/k_means_plus/k_means_plus.so")
ubx.load_module(ni, "app_blocks/path_finding/path_finding.so")
ubx.load_module(ni, "app_blocks/path_handler/path_handler.so")
ubx.load_module(ni, "app_blocks/waypoints_gen/waypoints_gen.so")

ubx.ffi_load_types(ni)

print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'rplidar_udp/rplidar_udp'")
rplidar_udp1=ubx.block_create(ni, "rplidar_udp/rplidar_udp", "rplidar_udp1", {rplidar_udp_config={opt_com_path='/dev/ttyUSB0', opt_com_baudrate=115200, host_ip="localhost", port=51068, EnableUDP=1}})

print("creating instance of 'k_means_plus/k_means_plus'")
k_means_plus1=ubx.block_create(ni, "k_means_plus/k_means_plus", "k_means_plus1", {k_means_plus_config={n_clusters=12}})

print("creating instance of 'waypoints_gen/waypoints_gen'")
waypoints_gen1=ubx.block_create(ni, "waypoints_gen/waypoints_gen", "waypoints_gen1", {waypoints_gen_config={nCenters=12, initPos={1250, 5000}, initGoal={7500,5000}}})

print("creating instance of 'path_finding/path_finding'")
path_finding1=ubx.block_create(ni, "path_finding/path_finding", "path_finding1", {path_finding_config={nCenters=12}})

print("creating instance of 'path_handler/path_handler'")
path_handler1=ubx.block_create(ni, "path_handler/path_handler", "path_handler1", {path_handler_config={nCenters=12}})

print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo1=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo1", {element_num=4, element_size=2880})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo2=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo2", {element_num=4, element_size=96})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo3=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo3", {element_num=4, element_size=4})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo4=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo4", {element_num=4, element_size=416})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo5=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo5", {element_num=4, element_size=10816})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo6=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo6", {element_num=4, element_size=416})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo7=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo7", {element_num=4, element_size=4})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo8=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo8", {element_num=4, element_size=2})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo9=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo9", {element_num=4, element_size=2})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo10=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo10", {element_num=4, element_size=2})


print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
			{
			   period = {sec=0, usec=250000 },
			   sched_policy="SCHED_OTHER", sched_priority=0,
			   trig_blocks={ 
					 { b=rplidar_udp1, num_steps=1, measure=0 },
                                         { b=k_means_plus1, num_steps=1, measure=0 },
                                         { b=waypoints_gen1, num_steps=1, measure=0 },
                                         { b=path_finding1, num_steps=1, measure=0 },
                                         { b=path_handler1, num_steps=1, measure=0 },
			   } } )

--ubx.ni_stat(ni)

p_rplidar_udp_data = ubx.port_get(rplidar_udp1,"rplidar_data");

p_k_means_plus_data_in = ubx.port_get(k_means_plus1,"data_in");
p_k_means_plus_data_out = ubx.port_get(k_means_plus1,"data_out");
p_k_means_plus_n_clusters_out = ubx.port_get(k_means_plus1,"n_clusters_out");

p_waypoints_gen_centerCoordinates = ubx.port_get(waypoints_gen1,"centerCoordinates");
p_waypoints_gen_nCetners = ubx.port_get(waypoints_gen1,"nCenters");
p_waypoints_gen_beginPos = ubx.port_get(waypoints_gen1,"beginPos");
p_waypoints_gen_endPos = ubx.port_get(waypoints_gen1,"endPos");
p_waypoints_gen_wayPoints = ubx.port_get(waypoints_gen1,"wayPoints");
p_waypoints_gen_costTable = ubx.port_get(waypoints_gen1,"costTable");

p_path_finding_wayPoints = ubx.port_get(path_finding1,"wayPoints");
p_path_finding_costTable = ubx.port_get(path_finding1,"costTable");
p_path_finding_pathWayPoints = ubx.port_get(path_finding1,"pathWayPoints");
p_path_finding_nValidNodes = ubx.port_get(path_finding1,"nValidNodes");


p_path_handler_plannedPathWayPoints = ubx.port_get(path_handler1,"plannedPathWayPoints");
p_path_handler_currentPos 	= ubx.port_get(path_handler1,"currentPos");
p_path_handler_goalPos		= ubx.port_get(path_handler1,"goalPos");
p_path_handler_nValidNodes 	= ubx.port_get(path_handler1,"nValidNodes");
p_path_handler_moveToPos 	= ubx.port_get(path_handler1,"moveToPos");

ubx.port_connect_in (p_k_means_plus_data_in,			fifo1);
ubx.port_connect_out (p_rplidar_udp_data, 			fifo1);

ubx.port_connect_in (p_waypoints_gen_centerCoordinates, 	fifo2);
ubx.port_connect_out(p_k_means_plus_data_out,			fifo2);

ubx.port_connect_in (p_waypoints_gen_nCetners, 			fifo3);
ubx.port_connect_out(p_k_means_plus_n_clusters_out,		fifo3);

ubx.port_connect_in (p_path_finding_wayPoints, 			fifo4);
ubx.port_connect_out(p_waypoints_gen_wayPoints,			fifo4);

ubx.port_connect_in (p_path_finding_costTable, 			fifo5);
ubx.port_connect_out(p_waypoints_gen_costTable,			fifo5);

ubx.port_connect_in (p_path_handler_plannedPathWayPoints,	fifo6);
ubx.port_connect_out(p_path_finding_pathWayPoints,		fifo6);

ubx.port_connect_in (p_path_handler_nValidNodes,		fifo7);
ubx.port_connect_out(p_path_finding_nValidNodes,		fifo7);


print("running webif init", ubx.block_init(webif1))
print("running rplidar_udp1 init", assert(ubx.block_init(rplidar_udp1))==0)
print("running k_means_plus init", ubx.block_init(k_means_plus1))
print("running waypoints_gen1 init", ubx.block_init(waypoints_gen1))
print("running path_finding1 init", ubx.block_init(path_finding1))
print("running path_handler1 init", ubx.block_init(path_handler1))
print("running ptrig1 init", ubx.block_init(ptrig1))

ubx.block_init(fifo1);
ubx.block_init(fifo2);
ubx.block_init(fifo3);
ubx.block_init(fifo4);
ubx.block_init(fifo5);
ubx.block_init(fifo6);
ubx.block_init(fifo7);


print("running webif start", ubx.block_start(webif1))
print("running rplidar_udp1 start", ubx.block_start(rplidar_udp1))
print("running k_means_plus start", ubx.block_start(k_means_plus1))
print("running waypoints_gen1 start", ubx.block_start(waypoints_gen1))
print("running path_finding1 start", ubx.block_start(path_finding1))
print("running path_handler1 start", ubx.block_start(path_handler1))
print("running ptrig1 start", ubx.block_start(ptrig1))

ubx.block_start(fifo1);
ubx.block_start(fifo2);
ubx.block_start(fifo3);
ubx.block_start(fifo4);
ubx.block_start(fifo5);
ubx.block_start(fifo6);
ubx.block_start(fifo7);
--print(utils.tab2str(ubx.block_totab(random1)))
print("--- demo app launched, browse to http://localhost:8888 and start ptrig1 block to start up")
io.read()

print("stopping and cleaning up blocks --------------------------------------------------------")
print("running ptrig1 unload", ubx.block_unload(ni, "ptrig1"))
print("running webif1 unload", ubx.block_unload(ni, "webif1"))
print("running rplidar_udp1 unload", ubx.block_unload(ni, "rplidar_udp1"))

ubx.unload_modules(ni)
-- ubx.ni_stat(ni)
os.exit(1)
