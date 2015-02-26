#!/usr/bin/env luajit
local ffi = require("ffi")
local ubx = require "ubx"
local ts = tostring
time=require("time")
ubx_utils = require("ubx_utils")
require"strict"

ni=ubx.node_create("testnode")

ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/testtypes/testtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
--ubx.load_module(ni, "arduino_blocks/arduino_upload/arduino_upload.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/logging/file_logger.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "arduino_blocks/serial_read/serial_read.so")
ubx.load_module(ni, "arduino_blocks/ps2x_youbot/ps2x_youbot.so")
ubx.load_module(ni, "std_blocks/youbot_driver/youbot_driver.so")
ubx.ffi_load_types(ni)

print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'youbot/youbot_driver'")
youbot1=ubx.block_create(ni, "youbot/youbot_driver", "youbot1", {ethernet_if="eth0" })

print("creating instance of 'lfds_buffers/cyclic'")
fifo1=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo1", {element_num=4,element_size=4})
print("creating instance of 'lfds_buffers/cyclic'")
fifo2=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo2", {element_num=4, element_size=128})
print("creating instance of 'lfds_buffers/cyclic'")
fifo3=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo3", {element_num=4, element_size=48})


print("creating instance of 'logging/file_logger'")

print("creating instance of 'serial_read/serial_read'")
serial_read1=ubx.block_create(ni, "serial_read/serial_read", "serial_read1", {serial_read_config={portName='/dev/ttyUSB0', brate=38400}})

print("creating instance of 'ps2x_youbot/ps2x_youbot'");
ps2x_youbot1=ubx.block_create(ni, "ps2x_youbot/ps2x_youbot", "ps2x_youbot1", {ps2x_youbot_config={delim=','}})


logger_conf=[[
{
--   { blockname='fifo1', portname="overruns", buff_len=1, },
--   { blockname='ptrig1', portname="tstats", buff_len=3, },
   { blockname='ps2x_youbot1', portname="data_out", buff_len=3,data_type="int"},
   { blockname='ps2x_youbot1', portname="nData", buff_len=3,data_type="unsigned int"},
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
			   period = {sec=0, usec=1000000 },
			   sched_policy="SCHED_OTHER", sched_priority=0,
			   trig_blocks={ 
				--	 { b=arduino_upload1, num_steps=1, measure=0 },
			   } } )


print("creating instance of 'std_triggers/ptrig'")
ptrig2=ubx.block_create(ni, "std_triggers/ptrig", "ptrig2",
                        { period={sec=0, usec=1000 }, sched_policy="SCHED_FIFO", sched_priority=80,
                          trig_blocks={ { b=youbot1, num_steps=1, measure=0 } } } )

print("creating instance of 'std_triggers/ptrig'")
ptrig3=ubx.block_create(ni, "std_triggers/ptrig", "ptrig3",
                        { period={sec=0, usec=10000 }, sched_policy="SCHED_FIFO", sched_priority=80,
                          trig_blocks={ { b=serial_read1, num_steps=1, measure=0 },
					{ b=ps2x_youbot1, num_steps=1, measure=0 }
			} } )


--- Create a table of all inversely connected ports:
local yb_pinv={}
ubx.ports_map(youbot1,
	      function(p)
		 local pname = ubx.safe_tostr(p.name)
		 yb_pinv[pname] = ubx.port_clone_conn(youbot1, pname)
	      end)


__time=ffi.new("struct ubx_timespec")
function gettime()
   ubx.clock_mono_gettime(__time)
   return {sec=tonumber(__time.sec), nsec=tonumber(__time.nsec)}
end

cm_data=ubx.data_alloc(ni, "int32_t")

--- Configure the base control mode.
-- @param mode control mode.
-- @return true if mode was set, false otherwise.
function base_set_control_mode(mode)
   ubx.data_set(cm_data, mode)
   ubx.port_write(yb_pinv.base_control_mode, cm_data)
   local res = ubx.port_read_timed(yb_pinv.base_control_mode, cm_data, 3)
   return ubx.data_tolua(cm_data)==mode
end

grip_data=ubx.data_alloc(ni, "int32_t")
function gripper(v)
   ubx.data_set(grip_data, v)
   ubx.port_write(yb_pinv.arm1_gripper, grip_data)
end

--- Configure the arm control mode.
-- @param mode control mode.
-- @return true if mode was set, false otherwise.
function arm_set_control_mode(mode)
   ubx.data_set(cm_data, mode)
   ubx.port_write(yb_pinv.arm1_control_mode, cm_data)
   local res = ubx.port_read_timed(yb_pinv.arm1_control_mode, cm_data, 3)
   return ubx.data_tolua(cm_data)==mode
end

--- Return once the youbot is initialized or raise an error.
function base_initialized()
   local res=ubx.port_read_timed(yb_pinv.base_control_mode, cm_data, 5)
   return ubx.data_tolua(cm_data)==0 -- 0=MOTORSTOP
end

--- Return once the youbot is initialized or raise an error.
function arm_initialized()
   local res=ubx.port_read_timed(yb_pinv.arm1_control_mode, cm_data, 5)
   return ubx.data_tolua(cm_data)==0 -- 0=MOTORSTOP
end


calib_int=ubx.data_alloc(ni, "int32_t")
function arm_calibrate()
   ubx.port_write(yb_pinv.arm1_calibrate_cmd, calib_int)
end

base_twist_data=ubx.data_alloc(ni, "struct kdl_twist")
base_null_twist_data=ubx.data_alloc(ni, "struct kdl_twist")

--- Move with a given twist.
-- @param twist table.
-- @param dur duration in seconds
function base_move_twist(twist_tab, dur)
   base_set_control_mode(2) -- VELOCITY
   ubx.data_set(base_twist_data, twist_tab)
   local ts_start=ffi.new("struct ubx_timespec")
   local ts_cur=ffi.new("struct ubx_timespec")

   ubx.clock_mono_gettime(ts_start)
   ubx.clock_mono_gettime(ts_cur)

   while ts_cur.sec - ts_start.sec < dur do
      ubx.port_write(yb_pinv.base_cmd_twist, base_twist_data)
      ubx.clock_mono_gettime(ts_cur)
   end
   ubx.port_write(yb_pinv.base_cmd_twist, base_null_twist_data)
end

base_vel_data=ubx.data_alloc(ni, "int32_t", 4)
base_null_vel_data=ubx.data_alloc(ni, "int32_t", 4)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel velocity
-- @param dur time in seconds to apply velocity
function base_move_vel(vel_tab, dur)
ubx.block_stop(fifo3)
   base_set_control_mode(2) -- VELOCITY
   ubx.data_set(base_vel_data, vel_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.base_cmd_vel, base_vel_data)
      ts_cur=gettime()
   end
   ubx.port_write(yb_pinv.base_cmd_vel, base_null_vel_data)
ubx.block_start(fifo3)
end

base_cur_data=ubx.data_alloc(ni, "int32_t", 4)
base_null_cur_data=ubx.data_alloc(ni, "int32_t", 4)

--- Move each wheel with an individual current value.
-- @param table of size 4 for with wheel current
-- @param dur time in seconds to apply currents.
function base_move_cur(cur_tab, dur)
   base_set_control_mode(6) -- CURRENT
   ubx.data_set(base_cur_data, cur_tab)

   local ts_start=ffi.new("struct ubx_timespec")
   local ts_cur=ffi.new("struct ubx_timespec")

   ubx.clock_mono_gettime(ts_start)
   ubx.clock_mono_gettime(ts_cur)

   while ts_cur.sec - ts_start.sec < dur do
      ubx.port_write(yb_pinv.base_cmd_cur, base_cur_data)
      ubx.clock_mono_gettime(ts_cur)
   end
   ubx.port_write(yb_pinv.base_cmd_cur, base_null_cur_data)
end


arm_vel_data=ubx.data_alloc(ni, "double", 5)
arm_null_vel_data=ubx.data_alloc(ni, "double", 5)

--- Move each joint with an individual rad/s value.
-- @param table of size for with wheel velocity
-- @param dur time in seconds to apply velocity
function arm_move_vel(vel_tab, dur)
   arm_set_control_mode(2) -- VELOCITY
   ubx.data_set(arm_vel_data, vel_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.arm1_cmd_vel, arm_vel_data)
      ts_cur=gettime()
   end
   ubx.port_write(yb_pinv.arm1_cmd_vel, arm_null_vel_data)
end

arm_eff_data=ubx.data_alloc(ni, "double", 5)
arm_null_eff_data=ubx.data_alloc(ni, "double", 5)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel effocity
-- @param dur time in seconds to apply effocity
function arm_move_eff(eff_tab, dur)
   arm_set_control_mode(6) --
   ubx.data_set(arm_eff_data, eff_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_eff=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_eff, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.arm1_cmd_eff, arm_eff_data)
      ts_eff=gettime()
   end
   ubx.port_write(yb_pinv.arm1_cmd_eff, arm_null_eff_data)
end

arm_cur_data=ubx.data_alloc(ni, "int32_t", 5)
arm_null_cur_data=ubx.data_alloc(ni, "int32_t", 5)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel curocity
-- @param dur time in seconds to apply curocity
function arm_move_cur(cur_tab, dur)
   arm_set_control_mode(6) --
   ubx.data_set(arm_cur_data, cur_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.arm1_cmd_cur, arm_cur_data)
      ts_cur=gettime()
   end
   ubx.port_write(yb_pinv.arm1_cmd_cur, arm_null_cur_data)
end


arm_pos_data=ubx.data_alloc(ni, "double", 5)
-- arm_null_pos_data=ubx.data_alloc(ni, "double", 5)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel posocity
-- @param dur time in seconds to apply posocity
function arm_move_pos(pos_tab)
   arm_set_control_mode(1) -- POS
   ubx.data_set(arm_pos_data, pos_tab)
   ubx.port_write(yb_pinv.arm1_cmd_pos, arm_pos_data)
end

function arm_tuck() arm_move_pos{2.588, 1.022, 2.248, 1.580, 2.591 } end
function arm_home() arm_move_pos{0,0,0,0,0} end


function help()
   local help_msg=
      [[
youbot test script.
 Base:
      base_set_control_mode(mode)	mode: mstop=0, pos=1, vel=2, cur=6
      base_move_twist(twist_tab, dur)  	move with twist (as Lua table) for dur seconds
      base_move_vel(vel_tab, dur)       move each wheel with individual vel [rpm] for dur seconds
      base_move_cur(cur_tab, dur)       move each wheel with individual current [mA] for dur seconds

]]

   if nr_arms>=1 then
      help_msg=help_msg..[[

 Arm: run arm_calibrate() (after each power-down) _BEFORE_ using the other arm functions!!

      arm_calibrate()			calibrate the arm. !!! DO THIS FIRST !!!
      arm_set_control_mode(mode)	see base.
      arm_move_pos(pos_tab, dur)	move to pos. pos_tab is Lua table of len=5 [rad]
      arm_move_vel(vel_tab, dur)	move joints. vel_tab is Lua table of len=5 [rad/s]
      arm_move_eff(eff_tab, dur)        move joints. eff_tab is Lua table of len=5 [Nm]
      arm_move_cur(cur_tab, dur)        move joints. cur_tab is Lua table of len=5 [mA]
      arm_tuck()                        move arm to "tuck" position
      arm_home()                        move arm to "candle" position
]]
   end
   if nr_arms>=2 then
      help_msg=help_msg..[[

	    WARNING: this script does currently not support the second youbot arm!
      ]]
   end
   print(help_msg)
end


function fmpc_run(dur_in)
   base_set_control_mode(2) -- VELOCITY
   --ubx.block_stop(ptrig1);
   --ubx.data_set(twist_data, fifo_out_youbot_msr_twist_to_fmpc)
   local dur = {sec=0, nsec=0}
   dur.sec,dur.nsec=math.modf(dur_in)
   dur.nsec = dur.nsec * 1000000000
        print(dur.sec, dur.nsec)
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}
   --ubx.block_start(ptrig2)
   while true do
      ubx.cblock_step(file_rep1)
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      --ubx.port_write(p_cmd_twist, cmd_twist)          
      --assert(ubx.cblock_step(fmpc1)==0);      
      ts_cur=gettime()
   end
   --ubx.block_stop(ptrig2)
   ubx.port_write(p_cmd_twist, null_twist_data)
end


-- ubx.ni_stat(ni)

print("running webif init", ubx.block_init(webif1))
print("running ptrig1 init", ubx.block_init(ptrig1))
print("running ptrig2 init", ubx.block_init(ptrig2))
print("running ptrig3 init", ubx.block_init(ptrig3))
--print("running arduino_upload1 init", ubx.block_init(arduino_upload1))
print("running fifo1 init", ubx.block_init(fifo1))
print("running fifo2 init", ubx.block_init(fifo2))
print("running fifo3 init", ubx.block_init(fifo3))
print("running file_log1 init", ubx.block_init(file_log1))
print("running serial_read1 init", ubx.block_init(serial_read1))
print("running ps2x_youbot1 init", ubx.block_init(ps2x_youbot1))
print("running youbot1 init",ubx.block_init(youbot1))

nr_arms=ubx.data_tolua(ubx.config_get_data(youbot1, "nr_arms"))



print("running webif start", ubx.block_start(webif1))


serial_read_out_port=ubx.port_get(serial_read1, "string_read")
ps2x_youbot_in_port=ubx.port_get(ps2x_youbot1, "string_in");
ubx.port_connect_out(serial_read_out_port, fifo2);
ubx.port_connect_in(ps2x_youbot_in_port, fifo2);

youbot_cmd_vel_port=ubx.port_get(youbot1, "base_cmd_twist");
ps2x_youbot_cmd_vel_port=ubx.port_get(ps2x_youbot1, "base_cmd_twist");
ubx.port_connect_out(ps2x_youbot_cmd_vel_port, fifo3);
ubx.port_connect_in(youbot_cmd_vel_port, fifo3);


ubx.block_start(ptrig1)
ubx.block_start(file_log1)
ubx.block_start(fifo1)
ubx.block_start(fifo2)
ubx.block_start(fifo3)
--ubx.block_start(arduino_upload1)
ubx.block_start(serial_read1)
ubx.block_start(ps2x_youbot1)
ubx.block_start(youbot1)
ubx.block_start(ptrig2)
ubx.block_start(ptrig3)

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


--print(utils.tab2str(ubx.block_totab(arduino_upload1)))
--print("--- demo app launched, browse to http://localhost:8888 and start ptrig1 block to start up")
--io.read()

--ubx.node_cleanup(ni)
--os.exit(1)
