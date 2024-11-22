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

    // sensor_msgs::convertPointCloudToPointCloud2();

    static sensor_msgs::PointCloud2 cloud_msg2;
    static sensor_msgs::PointCloud cloud_msg;

    cloud_msg2.header.frame_id = "radar";
    cloud_msg2.header.stamp = msg.detection_array[0].header.stamp;

    cloud_msg.header.frame_id = "radar";
    cloud_msg.header.stamp = msg.detection_array[0].header.stamp;

    // 设置 PointCloud 的点和通道
    cloud_msg.points.resize(size);
    cloud_msg.channels.resize(6);
    cloud_msg.channels[0].name = "intensity";
    cloud_msg.channels[0].values.resize(size);
    cloud_msg.channels[1].name = "doppler";
    cloud_msg.channels[1].values.resize(size);
    cloud_msg.channels[2].name = "doppler_std";
    cloud_msg.channels[2].values.resize(size);
    cloud_msg.channels[3].name = "invalidflags";
    cloud_msg.channels[3].values.resize(size);
    cloud_msg.channels[4].name = "probability";
    cloud_msg.channels[4].values.resize(size);
    cloud_msg.channels[5].name = "classification";
    cloud_msg.channels[5].values.resize(size);

    // 定义PointCloud2消息的字段
    sensor_msgs::PointCloud2Modifier modifier(cloud_msg2);

    // 设置PointCloud2消息的宽度和高度
    cloud_msg2.width = size;
    cloud_msg2.height = 1;

    modifier.setPointCloud2Fields(9, "x", 1, sensor_msgs::PointField::FLOAT32,
                                  "y", 1, sensor_msgs::PointField::FLOAT32,
                                  "z", 1, sensor_msgs::PointField::FLOAT32,
                                  "intensity", 1, sensor_msgs::PointField::FLOAT32,
                                  "doppler", 1, sensor_msgs::PointField::FLOAT32,
                                  "doppler_std", 1, sensor_msgs::PointField::FLOAT32,
                                  "invalidflags", 1, sensor_msgs::PointField::FLOAT32,
                                  "probability", 1, sensor_msgs::PointField::FLOAT32,
                                  "classification", 1, sensor_msgs::PointField::FLOAT32);

    // 获取字段的偏移量
    sensor_msgs::PointCloud2Iterator<float> iter_x(cloud_msg2, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(cloud_msg2, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(cloud_msg2, "z");
    sensor_msgs::PointCloud2Iterator<float> iter_rcs(cloud_msg2, "intensity");
    sensor_msgs::PointCloud2Iterator<float> iter_doppler(cloud_msg2, "doppler");
    sensor_msgs::PointCloud2Iterator<float> iter_doppler_std(cloud_msg2, "doppler_std");
    sensor_msgs::PointCloud2Iterator<float> iter_invalidflags(cloud_msg2, "invalidflags");
    sensor_msgs::PointCloud2Iterator<float> iter_classification(cloud_msg2, "probability");
    sensor_msgs::PointCloud2Iterator<float> iter_probability(cloud_msg2, "classification");

    // 填充PointCloud和PointCloud2消息的数据
    for (uint i = 0; i < size; i++)
    {
        // 填充PointCloud
        cloud_msg.points[i].x = msg.detection_array[i].f_x;
        cloud_msg.points[i].y = msg.detection_array[i].f_y;
        cloud_msg.points[i].z = msg.detection_array[i].f_z;
        cloud_msg.channels[0].values[i] = msg.detection_array[i].s_RCS;
        cloud_msg.channels[1].values[i] = msg.detection_array[i].f_RangeRate;
        cloud_msg.channels[2].values[i] = msg.detection_array[i].f_RangeRateSTD;
        cloud_msg.channels[3].values[i] = msg.detection_array[i].u_InvalidFlags;
        cloud_msg.channels[4].values[i] = msg.detection_array[i].u_PositivePredictiveValue;
        cloud_msg.channels[5].values[i] = msg.detection_array[i].u_Classification;

        // 填充PointCloud2
        *iter_x = msg.detection_array[i].f_x;
        *iter_y = msg.detection_array[i].f_y;
        *iter_z = msg.detection_array[i].f_z;
        *iter_rcs = msg.detection_array[i].s_RCS;
        *iter_doppler = msg.detection_array[i].f_RangeRate;
        *iter_doppler_std = msg.detection_array[i].f_RangeRateSTD;
        *iter_invalidflags = msg.detection_array[i].u_InvalidFlags;
        *iter_probability = msg.detection_array[i].u_PositivePredictiveValue;
        *iter_classification = msg.detection_array[i].u_Classification;

        ++iter_x;
        ++iter_y;
        ++iter_z;
        ++iter_rcs;
        ++iter_doppler;
        ++iter_doppler_std;
        ++iter_invalidflags;
        ++iter_probability;
        ++iter_classification;
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