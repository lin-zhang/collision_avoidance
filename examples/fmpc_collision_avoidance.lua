#!/usr/bin/env luajit

local ffi = require("ffi")
local ubx = require "ubx"
--local ubx_utils = require("ubx_utils")
local time = require("time")
local ts = tostring

require "strict"

ni=ubx.node_create("testnode")

ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/testtypes/testtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic_raw.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "app_blocks/rplidar_udp/rplidar_udp.so")
ubx.load_module(ni, "app_blocks/k_means_plus/k_means_plus.so")
ubx.load_module(ni, "app_blocks/path_finding/path_finding.so")
ubx.load_module(ni, "app_blocks/path_handler/path_handler.so")
ubx.load_module(ni, "app_blocks/waypoints_gen/waypoints_gen.so")
ubx.load_module(ni, "std_blocks/youbot_driver/youbot_driver.so")
ubx.load_module(ni, "std_blocks/fmpc_c2/fmpc.so")

ubx.ffi_load_types(ni)

print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'rplidar_udp/rplidar_udp'")
rplidar_udp1=ubx.block_create(ni, "rplidar_udp/rplidar_udp", "rplidar_udp1", {rplidar_udp_config={opt_com_path='/dev/ttyUSB0', opt_com_baudrate=115200, host_ip="localhost", port=51068, EnableUDP=1}})

print("creating instance of 'k_means_plus/k_means_plus'")
k_means_plus1=ubx.block_create(ni, "k_means_plus/k_means_plus", "k_means_plus1", {k_means_plus_config={n_clusters=12}})

print("creating instance of 'waypoints_gen/waypoints_gen'")
waypoints_gen1=ubx.block_create(ni, "waypoints_gen/waypoints_gen", "waypoints_gen1", {waypoints_gen_config={nCenters=12, initPos={0, 5}, initGoal={5,5}}})

print("creating instance of 'path_finding/path_finding'")
path_finding1=ubx.block_create(ni, "path_finding/path_finding", "path_finding1", {path_finding_config={nCenters=12}})

print("creating instance of 'path_handler/path_handler'")
path_handler1=ubx.block_create(ni, "path_handler/path_handler", "path_handler1", {path_handler_config={nCenters=12}})

print("creating instance of 'youbot/youbot_driver'")
youbot1=ubx.block_create(ni, "youbot/youbot_driver", "youbot1", {ethernet_if="eth0" })

print("creating instance of 'fmpc/fmpc'")
fmpc1=ubx.block_create(ni, "fmpc/fmpc", "fmpc1", {fmpc_config={
param_kappa=5e-5,
param_iteration=12,
param_fence={0,0,0,0},
param_states_max={10,10,0.4,0.4},
param_states_min={-10,-10,-0.4,-0.4},
param_states_init={-10,0,0,0},
param_inputs_max={3.9195,3.9195,3.9195,3.9195},
param_inputs_min={-3.9195,-3.9195,-3.9195,-3.9195},
param_inputs_init={0,0,0,0},
param_obstacle={-3.5,0.1,0.5}
}})

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
fifo8=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo8", {element_num=4, element_size=8})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo9=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo9", {element_num=4, element_size=8})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifo10=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifo10", {element_num=4, element_size=2})


print("creating instance of 'lfds_buffers/cyclic_raw'")
fifoS1=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifoS1", {element_num=4, element_size=48})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifoS2=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifoS2", {element_num=4, element_size=96})
print("creating instance of 'lfds_buffers/cyclic_raw'")
fifoS3=ubx.block_create(ni, "lfds_buffers/cyclic_raw", "fifoS3", {element_num=4, element_size=48})

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
                        { period={sec=0, usec=1000 }, sched_policy="SCHED_FIFO", sched_priority=80,
                          trig_blocks={ { b=youbot1, num_steps=1, measure=0 } } } )

print("creating instance of 'std_triggers/ptrig'")
ptrig2=ubx.block_create(ni, "std_triggers/ptrig", "ptrig2",
                        { period={sec=0, usec=100000 },
                          trig_blocks={
                                        { b=fmpc1, num_steps=1, measure=0} } } )
	
print("creating instance of 'std_triggers/ptrig'")
ptrig3=ubx.block_create(ni, "std_triggers/ptrig", "ptrig3",
			{
			   period = {sec=0, usec=200000 },
			   sched_policy="SCHED_OTHER", sched_priority=0,
			   trig_blocks={ 
					 { b=rplidar_udp1, num_steps=1, measure=0 },
                                         { b=k_means_plus1, num_steps=1, measure=0 },
                                         --{ b=waypoints_gen1, num_steps=1, measure=0 },
                                         --{ b=path_finding1, num_steps=1, measure=0 },
                                         --{ b=path_handler1, num_steps=1, measure=0 },
			   } } )


print("creating instance of 'std_triggers/ptrig'")
ptrig4=ubx.block_create(ni, "std_triggers/ptrig", "ptrig4",
                        { period={sec=0, usec=50000 },
                          trig_blocks={
                                         { b=waypoints_gen1, num_steps=1, measure=0 },
                                         { b=path_finding1, num_steps=1, measure=0 },
                                        { b=path_handler1, num_steps=1, measure=0 },
			  } } )

--ubx.ni_stat(ni)




--- Create a table of all inversely connected ports:
local yb_pinv={}
ubx.ports_map(youbot1,
	      function(p)
		 local pname = ubx.safe_tostr(p.name)
		 yb_pinv[pname] = ubx.port_clone_conn(youbot1, pname)
	      end)

local fmpc_pinv={}
ubx.ports_map(fmpc1,
              function(p)
                 local pname = ubx.safe_tostr(p.name)
                 fmpc_pinv[pname] = ubx.port_clone_conn(fmpc1, pname)
              end)

local waypoints_gen_pinv={}
ubx.ports_map(waypoints_gen1,
              function(p)
                 local pname = ubx.safe_tostr(p.name)
                 waypoints_gen_pinv[pname] = ubx.port_clone_conn(waypoints_gen1, pname)
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

p_fmpc_cmd_vel=ubx.port_get(fmpc1, "cmd_vel")
p_fmpc_cmd_twist=ubx.port_get(fmpc1, "cmd_twist")
p_fmpc_odom_input=ubx.port_get(fmpc1, "fmpc_odom_port")
p_fmpc_twist_input=ubx.port_get(fmpc1, "fmpc_twist_port")

p_youbot_curr_input=ubx.port_get(youbot1, 'base_cmd_cur');
p_youbot_twist_input=ubx.port_get(youbot1, 'base_cmd_twist');
p_youbot_msr_odom=ubx.port_get(youbot1, 'base_msr_odom');
p_youbot_msr_twist=ubx.port_get(youbot1, 'base_msr_twist');

p_fmpc_msr_odom=ubx.port_get(fmpc1, 'youbot_info_port');

p_fmpc_obstacle=ubx.port_get(fmpc1, 'fmpc_obstacle');
p_fmpc_goal_pose=ubx.port_get(fmpc1, 'fmpc_goal_pose');
p_fmpc_virtual_fence=ubx.port_get(fmpc1, 'fmpc_virtual_fence');
p_fmpc_wm_info_in=ubx.port_get(fmpc1, 'fmpc_wm_info_in');
p_fmpc_robot_pose=ubx.port_get(fmpc1, 'fmpc_robot_pose');

ubx.port_connect_out(p_fmpc_cmd_twist, fifoS1);
ubx.port_connect_in(p_youbot_twist_input, fifoS1);

ubx.port_connect_out(p_youbot_msr_odom,fifoS2);
ubx.port_connect_in(p_fmpc_odom_input,fifoS2);

ubx.port_connect_out(p_youbot_msr_twist,fifoS3);
ubx.port_connect_in(p_fmpc_twist_input,fifoS3);




-- FMPC controller
-- @param dur: time in seconds to run the controller
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
   ubx.block_start(ptrig2)
--   ubx.block_start(ptrig3)
   while true do
      --ubx.cblock_step(file_log1)
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      --ubx.port_write(p_cmd_twist, cmd_twist)          
      --assert(ubx.cblock_step(fmpc1)==0);      
      ts_cur=gettime()
   end
   ubx.block_stop(ptrig2)
--   ubx.block_stop(ptrig3)
   ubx.port_write(p_fmpc_cmd_twist, base_null_twist_data)
end

goal_arr_data=ubx.data_alloc(ni, "float", 2)
obs_arr_data=ubx.data_alloc(ni, "float", 3)


function fmpc_move(dur_in, goal_arr, obs_arr)
   base_set_control_mode(2) -- VELOCITY

   ubx.data_set(goal_arr_data, goal_arr)
   ubx.data_set(obs_arr_data, obs_arr)

   ubx.port_write(fmpc_pinv.fmpc_obstacle, obs_arr_data)
   ubx.port_write(fmpc_pinv.fmpc_goal_pose, goal_arr_data)
   --ubx.block_stop(ptrig1);
   --ubx.data_set(twist_data, fifo_out_youbot_msr_twist_to_fmpc)
   local dur = {sec=0, nsec=0}
   dur.sec,dur.nsec=math.modf(dur_in)
   dur.nsec = dur.nsec * 1000000000
        print(dur.sec, dur.nsec)
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}
   ubx.block_start(ptrig2)
--   ubx.block_start(ptrig3)
   while true do
      --ubx.cblock_step(file_log1)
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      --ubx.port_write(p_cmd_twist, cmd_twist)          
      --assert(ubx.cblock_step(fmpc1)==0);      
      ts_cur=gettime()
   end
   ubx.block_stop(ptrig2)
--   ubx.block_stop(ptrig3)
   ubx.port_write(p_fmpc_cmd_twist, base_null_twist_data)
end

final_goal_arr_data=ubx.data_alloc(ni, "float", 2)

function fmpc_lidar(dur_in, final_goal_arr)
   base_set_control_mode(2) -- VELOCITY

   ubx.data_set(final_goal_arr_data, final_goal_arr)
   --ubx.data_set(obs_arr_data, obs_arr)

   --ubx.port_write(fmpc_pinv.fmpc_obstacle, obs_arr_data)
   --ubx.port_write(fmpc_pinv.fmpc_goal_pose, goal_arr_data)
   ubx.port_write(waypoints_gen_pinv.endPos, final_goal_arr_data)
   --ubx.block_stop(ptrig1);
   --ubx.data_set(twist_data, fifo_out_youbot_msr_twist_to_fmpc)
   local dur = {sec=0, nsec=0}
   dur.sec,dur.nsec=math.modf(dur_in)
   dur.nsec = dur.nsec * 1000000000
        print(dur.sec, dur.nsec)
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}
   ubx.block_start(ptrig2)
--   ubx.block_start(ptrig3)
   while true do
      --ubx.cblock_step(file_log1)
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      --ubx.port_write(p_cmd_twist, cmd_twist)          
      --assert(ubx.cblock_step(fmpc1)==0);      
      ts_cur=gettime()
   end
   ubx.block_stop(ptrig2)
--   ubx.block_stop(ptrig3)
   ubx.port_write(p_fmpc_cmd_twist, base_null_twist_data)
end

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

ubx.port_connect_out(p_fmpc_robot_pose,				fifo8);
ubx.port_connect_in(p_waypoints_gen_beginPos,			fifo8);

ubx.port_connect_out(p_path_handler_moveToPos,			fifo9);
ubx.port_connect_in(p_fmpc_goal_pose,				fifo9);

print("running webif init", ubx.block_init(webif1))
print("running rplidar_udp1 init", assert(ubx.block_init(rplidar_udp1))==0)
print("running k_means_plus init", ubx.block_init(k_means_plus1))
print("running waypoints_gen1 init", ubx.block_init(waypoints_gen1))
print("running path_finding1 init", ubx.block_init(path_finding1))
print("running path_handler1 init", ubx.block_init(path_handler1))
print("running ptrig1 init", ubx.block_init(ptrig1))
print("running ptrig2 init", ubx.block_init(ptrig2))
print("running ptrig3 init", ubx.block_init(ptrig3))
print("running ptrig4 init", ubx.block_init(ptrig4))

print("running fmpc1 init", ubx.block_init(fmpc1))
print("running youbot1 init", ubx.block_init(youbot1))

ubx.block_init(fifo1);
ubx.block_init(fifo2);
ubx.block_init(fifo3);
ubx.block_init(fifo4);
ubx.block_init(fifo5);
ubx.block_init(fifo6);
ubx.block_init(fifo7);
ubx.block_init(fifo8);
ubx.block_init(fifo9);
ubx.block_init(fifoS1);
ubx.block_init(fifoS2);
ubx.block_init(fifoS3);


print("running webif start", ubx.block_start(webif1))
print("running rplidar_udp1 start", ubx.block_start(rplidar_udp1))
print("running k_means_plus start", ubx.block_start(k_means_plus1))
print("running waypoints_gen1 start", ubx.block_start(waypoints_gen1))
print("running path_finding1 start", ubx.block_start(path_finding1))
print("running path_handler1 start", ubx.block_start(path_handler1))
print("running fmpc1 start", ubx.block_start(fmpc1))
print("running youbot1 start", ubx.block_start(youbot1))
print("running ptrig1 start", ubx.block_start(ptrig1))
--print("running ptrig2 start", ubx.block_start(ptrig2))
print("running ptrig3 start", ubx.block_start(ptrig3))
print("running ptrig4 start", ubx.block_start(ptrig4))


ubx.block_start(fifo1);
ubx.block_start(fifo2);
ubx.block_start(fifo3);
ubx.block_start(fifo4);
ubx.block_start(fifo5);
ubx.block_start(fifo6);
ubx.block_start(fifo7);
ubx.block_start(fifo8);
ubx.block_start(fifo9);
ubx.block_start(fifoS1);
ubx.block_start(fifoS2);
ubx.block_start(fifoS3);

base_initialized();
--print(utils.tab2str(ubx.block_totab(random1)))
print("--- demo app launched, browse to http://localhost:8888 and start ptrig1 block to start up")
--io.read()

--print("stopping and cleaning up blocks --------------------------------------------------------")
--print("running ptrig1 unload", ubx.block_unload(ni, "ptrig1"))
--print("running webif1 unload", ubx.block_unload(ni, "webif1"))
--print("running rplidar_udp1 unload", ubx.block_unload(ni, "rplidar_udp1"))

--ubx.unload_modules(ni)
-- ubx.ni_stat(ni)
--os.exit(1)
