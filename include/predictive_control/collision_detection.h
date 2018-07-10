
#ifndef PREDICTIVE_CONTROL_COLLISION_DETECTION_H_
#define PREDICTIVE_CONTROL_COLLISION_DETECTION_H_

// ros include
#include <pluginlib/class_loader.h>
#include <ros/ros.h>
#include <ros/package.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <tf2_kdl/tf2_kdl.h>
#include <tf2_ros/static_transform_broadcaster.h>

// geometry_msgs, visualization_msgs include
#include <geometry_msgs/PointStamped.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <shape_msgs/SolidPrimitive.h>

// eigen includes
#include <Eigen/Eigen>
#include <Eigen/Core>
#include <Eigen/LU>

// c++ includes
#include <iostream>
#include <map>
#include <string>
#include <fstream>

// kdl,urdf includes
#include <urdf/model.h>
#include <kdl_parser/kdl_parser.hpp>

// predictive includes
#include <predictive_control/predictive_configuration.h>
#include <predictive_control/CollisionObject.h>
#include <predictive_control/StaticCollisionObject.h>
#include <predictive_control/StaticCollisionObjectRequest.h>
#include <predictive_control/StaticCollisionObjectResponse.h>

class CollisionRobot : public predictive_configuration
{
  /**
    * Class used to generate bounding volume around robot body,
    * - Development of distance cost function (logistic function) use to keep distance from robot body
    * - Visualize collision bounding ball aroung robot body
    * - Creat static frame for visulize center of intermidiate ball
    * Info: static member for computation of getEuclideanDistance
    */

public:
  /**
   * @brief CollisionRobot: Default constructor, allocate memory
   */
  CollisionRobot();

  /**
    *@brief ~CollisionRobot: Default distructor, free memory
    */
  ~CollisionRobot();

  /**
   * @brief initializeCollisionRobot: Initialize Collision Robot class
   * @return true with success, else false
   */
  bool initializeCollisionRobot();

  /**
   * @brief updateCollisionVolume: Update collsion matrix using forward kinematic relative to root link
   * @param FK_Homogenous_Matrix: Forward kinematic replative to root link
   * @param Transformation_Matrix: Transformation matrix between two concecutive frame
   */
  void updateCollisionVolume(const std::vector<Eigen::MatrixXd>& FK_Homogenous_Matrix,
                             const std::vector<Eigen::MatrixXd>& Transformation_Matrix);

  /**
   * @brief generateCollisionVolume: Create collsion matrix using forward kinematic relative to root link
   * @param FK_Homogenous_Matrix: Forward kinematic replative to root link
   * @param Transformation_Matrix: Transformation matrix between two concecutive frame
   */
  void generateCollisionVolume(const std::vector<Eigen::MatrixXd>& FK_Homogenous_Matrix,
                               const std::vector<Eigen::MatrixXd>& Transformation_Matrix);

  /**
   * @brief visualizeCollisionVolume: visulize collision ball on given position
   * @param center: Pose of center of volume
   * @param radius: Ball radius
   * @param ball_id: Ball id should be unique for each ball
   */
  void visualizeCollisionVolume(const geometry_msgs::PoseStamped& center, const double& radius,
                                const uint32_t& ball_id);

  /**
   * @brief computeCollisionCost: Computation collision distance cost,
   *                              Store Distance vectors represent distance from center of frame to other frame
   * @param collision_matrix: Collision matrix has information about distance between frames relative to root frame
   * @param collision_min_distance: Minimum collision distance, below that should not go
   * @param weight_factor: convergence rate
   */
  void computeCollisionCost(const std::map<std::string, geometry_msgs::PoseStamped> collision_matrix,
                            const double& collision_min_distance, const double& weight_factor);

  /**
   * @brief createStaticFrame: visulize intermidiate added frame, relative to root frame
   * @param stamped: Center position of ball
   * @param frame_name: Child frame name
   */
  void createStaticFrame(const geometry_msgs::PoseStamped& stamped, const std::string& frame_name);

  /**
   * @brief getEuclideanDistance: compute 2D distance called EuclideanDistance
   * @param pose_a: Pose of one point
   * @param pose_b: Pose of second point
   * @return distance between pose_a and pose_b
   */
  static inline double getEuclideanDistance(const geometry_msgs::Pose& pose_a, const geometry_msgs::Pose& pose_b)
  {
    ROS_DEBUG_STREAM("point_a:" << pose_a.position);
    ROS_DEBUG_STREAM("point_b:" << pose_b.position);

    double distance = (sqrt((pose_a.position.x - pose_b.position.x) * (pose_a.position.x - pose_b.position.x) +
                            (pose_a.position.y - pose_b.position.y) * (pose_a.position.y - pose_b.position.y) +
                            (pose_a.position.z - pose_b.position.z) * (pose_a.position.z - pose_b.position.z)));

    ROS_DEBUG_STREAM("getEuclideanDistance: ...  " << distance);
    return distance;
  }

  /** public data member*/
  // visulaize all volumes
  visualization_msgs::MarkerArray marker_array_;

  // collision matrix
  std::map<std::string, geometry_msgs::PoseStamped> collision_matrix_;

  // collision cost vector
  Eigen::VectorXd collision_cost_vector_;

private:
  // marker publisher
  ros::Publisher marker_pub_;

  // static frame broadcaster
  tf2_ros::StaticTransformBroadcaster static_broadcaster_;

  /**
   * @brief transformKDLToEigenMatrix: transform KDL Frame to Eigen Matrix
   * @param frame KDL::Frame which containts Rotation Matrix and Traslation vector
   * @param matrix transformation matrix
   */
  void transformKDLToEigenMatrix(const KDL::Frame& frame, Eigen::MatrixXd& matrix);

  /**
   * @brief transformEigenMatrixToKDL: transform Eigen Matrix to KDL Frame
   * @param matrix transformation matrix
   * @param frame KDL::Frame which containts Rotation Matrix and Traslation vector
   */
  void transformEigenMatrixToKDL(const Eigen::MatrixXd& matrix, KDL::Frame& frame);

  /**
   * @brief clearDataMember: clear vectors means free allocated memory
   */
  void clearDataMember();
};

//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------- Static Collision Object Avoidance
//-----------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
class StaticCollision : public predictive_configuration
{
  /**
    * Class used to generate bounding volume around static object,
    * - Development of distance cost function (logistic function) use to keep distance from robot body
    * - Visualize collision bounding ball around StaticCollision
    * - Creat static frame for visulize center of intermidiate ball
    */

public:
  /**
   * @brief CollisionRobot: Default constructor, allocate memory
   */
  StaticCollision();

  /**
    *@brief ~CollisionRobot: Default distructor, free memory
    */
  ~StaticCollision();

  /**
   * @brief initializeStaticCollisionObject: Initialize Collision Robot class
   * @return true with success, else false
   */
  bool initializeStaticCollisionObject();

  /**
   * @brief updateStaticCollisionVolume: update static collision volume function at every runtime
   * @param robot_critical_points
   */
  void updateStaticCollisionVolume(const std::map<std::string, geometry_msgs::PoseStamped>& robot_critical_points);

  /**
   * @brief generateStaticCollisionVolume: generate static collision oject, fill collision_matrix
   */
  void generateStaticCollisionVolume();

  /**
   * @brief visualizeStaticCollisionVoulme: visulize static collision object
   * @param stamped: collsion object represented by size, pose, primitive_type, operation
   */
  void visualizeStaticCollisionVoulme(const geometry_msgs::PoseStamped& stamped);

  /**
   * @brief computeStaticCollisionCost: Computation collision distance cost,
   *                              Store Distance vectors represent distance from center of frame to other frame
   * @param collision_matrix: Collision matrix has information about distance between frames relative to root frame
   * @param collision_min_distance: Minimum collision distance, below that should not go
   * @param weight_factor: convergence rate
   */
  void computeStaticCollisionCost(const std::map<std::string, geometry_msgs::PoseStamped> static_collision_matrix,
                                  const std::map<std::string, geometry_msgs::PoseStamped> robot_collision_matrix,
                                  const double& collision_threshold_distance, const double& weight_factor);

  /** public data member*/
  // visulaize all volumes
  visualization_msgs::MarkerArray marker_array_;

  // collision matrix
  std::map<std::string, geometry_msgs::PoseStamped> collision_matrix_;

  // collision cost vector
  Eigen::VectorXd collision_cost_vector_;

private:
  // marker publisher
  ros::Publisher marker_pub_;

  // service
  ros::ServiceServer add_static_object_;
  ros::ServiceServer remove_static_object_;
  ros::ServiceServer remove_all_static_objects_;

  // transform listerner
  tf::TransformListener tf_listener_;

  // static frame broadcaster
  tf2_ros::StaticTransformBroadcaster static_broadcaster_;

  /**
   * @brief getTransform: Find transformation stamed rotation is in the form of quaternion
   * @param from: source frame from find transformation
   * @param to: target frame till find transformation
   * @param stamped_pose: Resultant poseStamed between source and target frame
   * @return: true if transform else false
   */
  bool getTransform(const std::string& from, const std::string& to, geometry_msgs::PoseStamped& stamped_pose);

  bool addStaticObjectServiceCB(predictive_control::StaticCollisionObjectRequest& request,
                                predictive_control::StaticCollisionObjectResponse& response);

  bool removeStaticObjectServiceCB(predictive_control::StaticCollisionObjectRequest& request,
                                   predictive_control::StaticCollisionObjectResponse& response);

  bool removeAllStaticObjectsServiceCB(predictive_control::StaticCollisionObjectRequest& request,
                                       predictive_control::StaticCollisionObjectResponse& response);

  /**
   * @brief createStaticFrame: visulize intermidiate added frame, relative to root frame
   * @param stamped: Center position of ball
   * @param frame_name: Child frame name
   */
  void createStaticFrame(const geometry_msgs::PoseStamped& stamped, const std::string& frame_name);

  /**
   * @brief clearDataMember: clear vectors means free allocated memory
   */
  void clearDataMember();
};

#endif  // PREDICTIVE_CONTROL_COLLISION_DETECTION_H_
