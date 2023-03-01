#ifndef TBOT__GLOBAL_PLANNER_H
#define TBOT__GLOBAL_PLANNER_H

#include "ros/ros.h"
#include "astar.h"
#include "thetastar.h"
#include "djikstra.h"
#include "bot_utils/bot_utils.h"
#include "bot_utils/map_data.h"
#include "tmsgs/Goal.h"

#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/Int32MultiArray.h>
#include <std_msgs/Bool.h>
#include <nav_msgs/Path.h>
#include <vector>



/*
GlobalPlanner class has the following roles:
- Subscribe to pose
- Subscribe to mapinfo/inflation grid
- Subscribe to mapinfo/logodds grid
- Subscribe to incoming goal
- Check status of goal
*/

class GlobalPlanner
{
private:
    //pose from motion filter
    geometry_msgs::PoseStamped robot_pose_; //ok
    bot_utils::Pos2D robot_position_; //ok
    bot_utils::Index robot_index_; //ok
    bool robot_status_;

    //All the map data and meta data. This to to make it easy to share with planner without copying so much stuff
    bot_utils::MapData mapdata;
    
    //goals
    bot_utils::Pos2D current_goal_; //ok
    bool goal_status_;

    //planning related
    bool trigger_plan;

    //path
    nav_msgs::Path path_msg_;
    std::vector<bot_utils::Pos2D> path_;

    //subscribers
    ros::Subscriber pose_sub_;
    ros::Subscriber inflation_sub_;
    ros::Subscriber lo_sub_;
    ros::Subscriber goal_sub_;
    ros::Subscriber replan_sub_;

    //publishers
    ros::Publisher update_goal_pub_;
    ros::Publisher path_pub_;

    //nodehandle
    ros::NodeHandle nh_;
    double rate_;
    const double EPS_ = 1e-6; //for float comparisons
public:
    //Class constructor for global planner
    GlobalPlanner(ros::NodeHandle &nh);
    void run();

    //callbacks
    void poseCallback(const geometry_msgs::PoseStampedConstPtr &pose_msg);

    void inflationCallback(const std_msgs::Int32MultiArrayConstPtr &inflation);

    void logoddsCallback(const std_msgs::Int32MultiArrayConstPtr &inflation);

    void goalCallback(const tmsgs::GoalConstPtr &goal);

    void replanCallback(const std_msgs::BoolConstPtr &replan);
    
    //load params
    bool loadParams();

    //helper stuff
    bot_utils::Index pos2idx(bot_utils::Pos2D &pos);

    bot_utils::Pos2D idx2pos(bot_utils::Index &idx);

    int flatten(bot_utils::Index &idx);

    bool oob(bot_utils::Index &idx);

    bool testPos(bot_utils::Pos2D idx);

    void writeToPathMsg();
};

#endif