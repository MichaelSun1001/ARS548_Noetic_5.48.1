#include <ros/ros.h>

#include <ars548_msg/objects.h>
#include <ars548_msg/ObjectList.h>
#include <ars548_msg/detections.h>
#include <ars548_msg/DetectionList.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <sensor_msgs/PointCloud.h>
#include <geometry_msgs/Point32.h>
#include <sensor_msgs/ChannelFloat32.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

ros::Publisher objects_marker_pub;
ros::Publisher detections_cloud_pub;
ros::Publisher detections_cloud2_pub;

ros::Publisher point_cloud_pub, point_cloud2_pub, detections_cloud_pub2;

void objectReceive(const ars548_msg::ObjectList &msg)
{
    uint size = msg.object_array.size();
    visualization_msgs::Marker my_marker;
    visualization_msgs::MarkerArray marker_array;
    marker_array.markers.clear();

    if (size > 0)
    {
        for (uint i = 0; i < size; i++)
        {
            my_marker.header.frame_id = "radar";
            my_marker.header.stamp = msg.object_array[i].header.stamp;
            my_marker.ns = "object_shapes";

            my_marker.id = msg.object_array[i].u_ID;
            my_marker.type = visualization_msgs::Marker::CUBE;
            my_marker.action = visualization_msgs::Marker::ADD;
            my_marker.pose.position.x = msg.object_array[i].u_Position_X;
            my_marker.pose.position.y = msg.object_array[i].u_Position_Y;
            my_marker.pose.position.z = msg.object_array[i].u_Position_Z;

            my_marker.pose.orientation.x = 0.0;
            my_marker.pose.orientation.y = 0.0;
            my_marker.pose.orientation.z = sin(msg.object_array[i].u_Position_Orientation / 2);
            my_marker.pose.orientation.w = cos(msg.object_array[i].u_Position_Orientation / 2);

            if ((msg.object_array[i].u_Shape_Length_Edge_Mean > 0.2) || (msg.object_array[i].u_Shape_Width_Edge_Mean > 0.2))
            {
                my_marker.scale.x = msg.object_array[i].u_Shape_Length_Edge_Mean;
                my_marker.scale.y = msg.object_array[i].u_Shape_Width_Edge_Mean;
                my_marker.scale.z = (msg.object_array[i].u_Shape_Length_Edge_Mean + msg.object_array[i].u_Shape_Width_Edge_Mean) / 2;
            }
            else
            {
                my_marker.scale.x = 0.2;
                my_marker.scale.y = 0.2;
                my_marker.scale.z = 0.2;
            }

            my_marker.color.r = 0.0f;
            my_marker.color.g = 1.0f;
            my_marker.color.b = 0.0f;
            my_marker.color.a = 1.0;

            my_marker.lifetime = ros::Duration(0.5);

            marker_array.markers.push_back(my_marker);
        }

        objects_marker_pub.publish(marker_array);
    }
}

void detectionReceive(const ars548_msg::DetectionList &msg)
{
    uint size = msg.detection_array.size();

    sensor_msgs::PointCloud cloud;

    geometry_msgs::Point32 p;

    if (size > 0)
    {
        cloud.header.frame_id = "radar";
        cloud.header.stamp = msg.detection_array[0].header.stamp;
        cloud.points.clear();
        for (uint i = 0; i < size; i++)
        {
            p.x = msg.detection_array[i].f_x;
            p.y = msg.detection_array[i].f_y;
            p.z = msg.detection_array[i].f_z;

            cloud.points.push_back(p);
        }

        detections_cloud_pub.publish(cloud);
        // 发布2的消息 /ars548_process/detection_point_cloud2
        sensor_msgs::PointCloud2 cloud2;
        sensor_msgs::convertPointCloudToPointCloud2(cloud, cloud2);
        detections_cloud2_pub.publish(cloud2);
    }
}

// 原始版本，确认能用

void detectionReceive_sax(const ars548_msg::DetectionList &msg)
{
    ROS_INFO("cl2run");
    uint size = msg.detection_array.size();

    // 静态消息以在调用之间保留数据
    static sensor_msgs::PointCloud2 cloud_msg2;
    static sensor_msgs::PointCloud cloud_msg;

    // 设置头部信息
    cloud_msg2.header.frame_id = "radar";
    if (size > 0)
    {
        cloud_msg2.header.stamp = msg.detection_array[0].header.stamp;
    }

    cloud_msg.header.frame_id = "radar";
    if (size > 0)
    {
        cloud_msg.header.stamp = msg.detection_array[0].header.stamp;
    }

    // 配置 PointCloud 消息
    cloud_msg.points.resize(size);
    cloud_msg.channels.resize(2); // 仅保留 intensity 和 doppler

    // 配置 intensity 通道
    cloud_msg.channels[0].name = "intensity";
    cloud_msg.channels[0].values.resize(size);

    // 配置 doppler 通道
    cloud_msg.channels[1].name = "doppler";
    cloud_msg.channels[1].values.resize(size);

    // 定义 PointCloud2 消息的字段
    sensor_msgs::PointCloud2Modifier modifier(cloud_msg2);

    // 设置 PointCloud2 消息的宽度和高度
    cloud_msg2.width = size;
    cloud_msg2.height = 1;
    cloud_msg2.is_dense = false;

    // 设置 PointCloud2 的字段，仅包含 x, y, z, intensity, doppler
    modifier.setPointCloud2Fields(5,
                                  "x", 1, sensor_msgs::PointField::FLOAT32,
                                  "y", 1, sensor_msgs::PointField::FLOAT32,
                                  "z", 1, sensor_msgs::PointField::FLOAT32,
                                  "intensity", 1, sensor_msgs::PointField::FLOAT32,
                                  "doppler", 1, sensor_msgs::PointField::FLOAT32);

    // 获取字段的迭代器
    sensor_msgs::PointCloud2Iterator<float> iter_x(cloud_msg2, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(cloud_msg2, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(cloud_msg2, "z");
    sensor_msgs::PointCloud2Iterator<float> iter_intensity(cloud_msg2, "intensity");
    sensor_msgs::PointCloud2Iterator<float> iter_doppler(cloud_msg2, "doppler");

    // 填充 PointCloud 和 PointCloud2 消息的数据
    for (uint i = 0; i < size; i++)
    {
        // 填充 PointCloud
        cloud_msg.points[i].x = msg.detection_array[i].f_x;
        cloud_msg.points[i].y = msg.detection_array[i].f_y;
        cloud_msg.points[i].z = msg.detection_array[i].f_z;
        cloud_msg.channels[0].values[i] = msg.detection_array[i].s_RCS;       // intensity
        cloud_msg.channels[1].values[i] = msg.detection_array[i].f_RangeRate; // doppler

        // 填充 PointCloud2
        *iter_x = msg.detection_array[i].f_x;
        *iter_y = msg.detection_array[i].f_y;
        *iter_z = msg.detection_array[i].f_z;
        *iter_intensity = msg.detection_array[i].s_RCS;     // intensity
        *iter_doppler = msg.detection_array[i].f_RangeRate; // doppler

        ++iter_x;
        ++iter_y;
        ++iter_z;
        ++iter_intensity;
        ++iter_doppler;
    }

    // 发布点云消息
    point_cloud2_pub.publish(cloud_msg2);
    point_cloud_pub.publish(cloud_msg);
}

void pointcloud1Callback(const sensor_msgs::PointCloud::ConstPtr &msg)
{
    pcl::PointCloud<pcl::PointXYZRGB> cloud;

    sensor_msgs::PointCloud2 cl2;
    sensor_msgs::convertPointCloudToPointCloud2(*msg, cl2);
    pcl::fromROSMsg(cl2, cloud);

    sensor_msgs::PointCloud2 output;

    pcl::toROSMsg(cloud, output);

    ros::NodeHandle nh;
    ros::Publisher pub = nh.advertise<sensor_msgs::PointCloud2>("output", 1);
    pub.publish(output);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "info_convert_node");
    ros::NodeHandle nh;
    ros::NodeHandle private_nh("~");

    ros::Subscriber obj_list_sub = nh.subscribe("/ars548_process/object_list", 10, &objectReceive);
    ros::Subscriber det_list_sub = nh.subscribe("/ars548_process/detection_list", 10, &detectionReceive_sax);

    objects_marker_pub = nh.advertise<visualization_msgs::MarkerArray>("/ars548_process/object_marker", 10);

    point_cloud_pub = nh.advertise<sensor_msgs::PointCloud>("/ars548_process/point_cloud", 10);
    point_cloud2_pub = nh.advertise<sensor_msgs::PointCloud2>("/ars548_process/point_cloud2", 10);

    ros::Subscriber sub = nh.subscribe("input", 1, pointcloud1Callback);

    ros::spin();
}