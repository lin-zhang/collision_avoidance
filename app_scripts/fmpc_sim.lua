#!/usr/bin/luajit

ffi = require("ffi")
ubx = require("ubx")
time = require("time")
ubx_utils = require("ubx_utils")
ts = tostring

require"strict"
-- require"trace"

-- prog starts here.
ni=ubx.node_create("youbot")

-- load modules
ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/youbot_driver/youbot_driver.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/logging/file_logger.so")
ubx.load_module(ni, "std_blocks/fmpc/fmpc.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
-- create necessary blocks
print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

--print("creating instance of 'youbot/youbot_driver'")
--youbot1=ubx.block_create(ni, "youbot/youbot_driver", "youbot1", {ethernet_if="eth0" })

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
			{ period={sec=0, usec=1000 }, sched_policy="SCHED_FIFO", sched_priority=80,
			  trig_blocks={ 
				-- {b=youbot1, num_steps=1, measure=0 } 
			} } )

print("creating instance of 'fmpc/fmpc'")
fmpc1=ubx.block_create(ni, "fmpc/fmpc", "fmpc1" )

print("creating instance of 'lfds_buffers/cyclic'")
fifo1=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo1", {element_num=4, element_size=48})
print("creating instance of 'lfds_buffers/cyclic'")
fifo2=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo2", {element_num=4, element_size=96})
print("creating instance of 'lfds_buffers/cyclic'")
fifo3=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo3", {element_num=4, element_size=48})



print("creating instance of 'logging/file_logger'")

rep_conf=[[
{
   --{ blockname='youbot1', portname="base_motorinfo", buff_len=2, },
   --{ blockname='youbot1', portname="base_msr_twist", buff_len=2, },
   --{ blockname='youbot1', portname="base_msr_odom", buff_len=2, },
   --{ blockname='fmpc1', portname="cmd_twist", buff_len=2, },    
   --{ blockname='fmpc1', portname="cmd_vel", buff_len=2, }	
}
]]

file_log1=ubx.block_create(ni, "logging/file_logger", "file_log1",
			   {filename='report.dat',
			    separator=',',
			    report_conf=rep_conf})

print("creating instance of 'std_triggers/ptrig'")
ptrig2=ubx.block_create(ni, "std_triggers/ptrig", "ptrig2",
			{ period={sec=0, usec=25000 },
			  trig_blocks={ 
					--{ b=file_log1, num_steps=1, measure=0 },
					{ b=fmpc1, num_steps=1, measure=0} } } )

p_fmpc_cmd_vel=ubx.port_get(fmpc1, "cmd_vel")
p_fmpc_cmd_twist=ubx.port_get(fmpc1, "cmd_twist")
p_fmpc_odom_input=ubx.port_get(fmpc1, "fmpc_odom_port")
p_fmpc_twist_input=ubx.port_get(fmpc1, "fmpc_twist_port")

--p_youbot_curr_input=ubx.port_get(youbot1, 'base_cmd_cur');
--p_youbot_twist_input=ubx.port_get(youbot1, 'base_cmd_twist');
--p_youbot_msr_odom=ubx.port_get(youbot1, 'base_msr_odom');
--p_youbot_msr_twist=ubx.port_get(youbot1, 'base_msr_twist');

ubx.port_connect_out(p_fmpc_cmd_twist, fifo1);
ubx.port_connect_in(p_fmpc_twist_input, fifo1);

--ubx.port_connect_out(p_youbot_msr_odom,fifo2);
--ubx.port_connect_in(p_fmpc_odom_input,fifo2);

--ubx.port_connect_out(p_youbot_msr_twist,fifo3);
--ubx.port_connect_in(p_fmpc_twist_input,fifo3);

-- start and init webif and youbot
ubx.block_init(fifo1);
ubx.block_init(fifo2);
ubx.block_init(fifo3);
assert(ubx.block_init(ptrig1))
assert(ubx.block_init(ptrig2))
assert(ubx.block_init(file_log1))
assert(ubx.block_init(webif1)==0)
--assert(ubx.block_init(youbot1)==0)
assert(ubx.block_init(fmpc1)==0)

--nr_arms=ubx.data_tolua(ubx.config_get_data(youbot1, "nr_arms"))

ubx.block_start(fifo1);
ubx.block_start(fifo2);
ubx.block_start(fifo3);
assert(ubx.block_start(webif1)==0)
assert(ubx.block_start(file_log1)==0)
--assert(ubx.block_start(youbot1)==0)
assert(ubx.block_start(fmpc1)==0)
assert(ubx.block_start(ptrig1)==0)

-- make sure youbot is running ok.
base_initialized()

if nr_arms==1 then
   arm_initialized()
elseif nr_arms==2 then
   print("WARNING: this script does not yet support a two arm youbot (the driver does however)")
end

twst={vel={x=0.05,y=0,z=0},rot={x=0,y=0,z=0.1}}
vel_tab={1,1,1,1}
arm_vel_tab={0.002,0.002,0.003,0.002, 0.002}




print('Please run "help()" for information on available functions')
-- ubx.node_cleanup(ni)
