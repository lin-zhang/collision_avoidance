#!/usr/bin/env luajit

local ffi = require("ffi")
local ubx = require "ubx"
local ts = tostring

ni=ubx.node_create("testnode")

ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/testtypes/testtypes.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/logging/file_logger.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "arduino_blocks/udp_server_port/udp_server_port.so")
ubx.load_module(ni, "arduino_blocks/udp_client_port/udp_client_port.so")

ubx.ffi_load_types(ni)

print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'lfds_buffers/cyclic'")
fifo1=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo1", {buffer_len=4, type_name="unsigned int"})
print("creating instance of 'lfds_buffers/cyclic'")
fifo2=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo2", {buffer_len=4, type_name="char", data_len=24})
--fifo2=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo2", {element_num=24, element_size=24})

print("creating instance of 'logging/file_logger'")

print("creating instance of 'udp_server_port/udp_server_port'")
udp_server_port1=ubx.block_create(ni, "udp_server_port/udp_server_port", "udp_server_port1", {udp_server_port_config={port=51068}})
print("creating instance of 'udp_client_port/udp_client_port'")
udp_client_port1=ubx.block_create(ni, "udp_client_port/udp_client_port", "udp_client_port1", {udp_client_port_config={port=51068, host_ip='10.211.55.2'}})

logger_conf=[[
{
   { blockname='fifo1', portname="overruns", buff_len=1, },
   { blockname='ptrig1', portname="tstats", buff_len=3, },
--   { blockname='udp_server_port1', portname="data_out", buff_len=3, data_len=24, data_type=char,}a
}
]]

file_log1=ubx.block_create(ni, "logging/file_logger", "file_log1",
			   {filename='report.dat',
			    separator=',',
			    timestamp=1,
			    report_conf=logger_conf})

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
			{
			   period = {sec=0, usec=5000 },
			   sched_policy="SCHED_OTHER", sched_priority=0,
			   trig_blocks={ 
					 { b=udp_server_port1, num_steps=1, measure=0 },	
					 { b=udp_client_port1, num_steps=1, measure=0 },	
					 { b=file_log1, num_steps=1, measure=0 }
			   } } )

-- ubx.ni_stat(ni)

print("running webif init", ubx.block_init(webif1))
print("running ptrig1 init", ubx.block_init(ptrig1))
print("running fifo1 init", ubx.block_init(fifo1))
print("running fifo1 init", ubx.block_init(fifo2))
print("running file_log1 init", ubx.block_init(file_log1))
print("running udp_server_port1 init", ubx.block_init(udp_server_port1))
print("running udp_client_port1 init", ubx.block_init(udp_client_port1))


print("running webif start", ubx.block_start(webif1))

server_out_port=ubx.port_get(udp_server_port1, "data_out");
client_in_port=ubx.port_get(udp_client_port1, "data_in");

ubx.port_connect_out(server_out_port, fifo2);
ubx.port_connect_in(client_in_port, fifo2);


ubx.block_start(file_log1)
ubx.block_start(fifo1)
ubx.block_start(fifo2)
ubx.block_start(udp_server_port1)
ubx.block_start(udp_client_port1)

ubx.block_start(ptrig1)
--print(utils.tab2str(ubx.block_totab(arduino_upload1)))
print("--- demo app launched, browse to http://localhost:8888 and start ptrig1 block to start up")
io.read()

ubx.node_cleanup(ni)
os.exit(1)
