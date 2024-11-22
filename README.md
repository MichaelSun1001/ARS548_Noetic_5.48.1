1、自己电脑的IP要配置为10.13.1.166，默认网关10.13.1.1，子网掩码255.255.255.0。
2、软件使用时，要关掉防火墙。
3、雷达IP：10.13.1.113


rm build/ devel/ -fr

catkin_make --pkg ars548_msg

catkin_make -j8


source devel/setup.bash

roslaunch ars548_process ars548_process.launch

rosbag record -a

rosbag play xxx.bag -r 10

rostopic echo -b xxx.bag -p /xxx >xxx.cxv



第一次使用：

sudo apt install ros-xxx-serial

# 根据自己的ros版本而定

sudo chmod 777 wheeltec_udev.sh
sudo ./wheeltec_udev.sh
这些udev_rules.sh脚本，将确保当插入相应的USB串口设备时，系统会自动为其创建符号链接，并设置权限，方便用户在应用程序中访问串口设备。

不是第一次使用：

sudo chmod a+rw /dev/ttyUSB0

# 启动前一定要修改串口权限，否则会报错:"Permission denied" error

### 使用

source devel/setup.bash
roslaunch fdilink_ahrs ahrs_data.launch

其他命令：

# 查看设备

ls /dev/ttyUSB0
ll /dev

rostopic list
rostopic echo /imu

# 遗留问题

正确连接方式：
先连usb后上电

错误连接方式：
场景一：先上电后连usb，通信指示灯快速闪烁，rostopic无法显示imu信息
场景二：正确连接后，拔掉usb，再连接usb，通信指示灯慢速闪烁，rostopic无法显示imu信息。
usb指示灯有两个，一个应该是usb电源指示灯，一个应该是通信指示灯，当通信指示灯常亮时使用rostopic echo /imu
可以看到imu的信息，当通信指示灯一直快速闪烁或者慢速闪烁时，无法显示imu信息。
