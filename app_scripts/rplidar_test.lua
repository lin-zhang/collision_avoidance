#!/usr/bin/env luajit

local ffi = require("ffi")
local ubx = require "ubx"
--local ubx_utils = require("ubx_utils")
local ts = tostring

ni=ubx.node_create("testnode")

ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/testtypes/testtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "app_blocks/rplidar_udp/rplidar_udp.so")
ubx.ffi_load_types(ni)

print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'rplidar_udp/rplidar_udp'")
rplidar_udp1=ubx.block_create(ni, "rplidar_udp/rplidar_udp", "rplidar_udp1", {rplidar_udp_config={opt_com_path='/dev/ttyUSB0', opt_com_baudrate=115200, host_ip="localhost", port=51068, EnableUDP=1}})

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
			{
			   period = {sec=0, usec=200000 },
			   sched_policy="SCHED_OTHER", sched_priority=0,
			   trig_blocks={ 
					 { b=rplidar_udp1, num_steps=1, measure=0 }
			   } } )

--ubx.ni_stat(ni)

print("running webif init", ubx.block_init(webif1))
print("running rplidar1 init", assert(ubx.block_init(rplidar_udp1))==0)
print("running ptrig1 init", ubx.block_init(ptrig1))

print("running webif start", ubx.block_start(webif1))
print("running rplidar_udp1 start", ubx.block_start(rplidar_udp1))
ubx.block_start(ptrig1)

--print(utils.tab2str(ubx.block_totab(random1)))
print("--- demo app launched, browse to http://localhost:8888 and start ptrig1 block to start up")
io.read()

print("stopping and cleaning up blocks --------------------------------------------------------")
print("running ptrig1 unload", ubx.block_unload(ni, "ptrig1"))
print("running webif1 unload", ubx.block_unload(ni, "webif1"))
print("running rplidar_udp1 unload", ubx.block_unload(ni, "rplidar_udp1"))

-- ubx.ni_stat(ni)
-- l1=ubx.ubx_alloc_data(ni, "unsigned long", 1)
-- if l1~=nil then print_data(l1) end

ubx.unload_modules(ni)
-- ubx.ni_stat(ni)
os.exit(1)
