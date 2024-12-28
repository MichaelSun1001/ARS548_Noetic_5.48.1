这个驱动是ARS548 固件5.48.1版本的，此版本雷达表面没有丝印。

1、自己电脑的IP要配置为10.13.1.166，默认网关10.13.1.1，子网掩码255.255.255.0。
2、软件使用时，要关掉防火墙。
3、雷达IP：10.13.1.113


rm build/ devel/ -fr src/CMakeLists.txt

catkin_make --pkg ars548_msg

catkin_make -j8


source devel/setup.bash

roslaunch ars548_process ars548_process.launch

rosbag record -a

rosbag record /imu /raw_gnss /gps/fix /ars548_process/point_cloud /ars548_process/point_cloud2 /hik_cam_node/hik_camera/compressed /rtsp_camera_relay/image/compressed

rosbag play xxx.bag -r 10

rostopic echo -b xxx.bag -p /xxx >xxx.cxv
