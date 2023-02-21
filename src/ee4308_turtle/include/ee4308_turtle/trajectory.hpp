#include "common.hpp"
#include "grid.hpp"
#include <algorithm>
#ifndef TRAJECTORY_HPP
#define TRAJECTORY_HPP
std::vector<Position> post_process(std::vector<Position> path, Grid &grid); // returns the turning points
std::vector<Position> generate_trajectory(Position pos_begin, Position pos_end, double average_speed, double target_dt, Grid & grid, double robot_angle , double initial_vel);
bool is_safe_trajectory(std::vector<Position> trajectory, Grid & grid);

#endif