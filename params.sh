export WORLD="world20" ##options: world20 world21 world23proj1 world0. ##Note that world23proj1 will not create a gazebo, cus theres no sim world. Note that world0 is for tuning
export TASK="proj2" ##options: labs proj1 proj2 soloflight


#The following describes the possible combinations of TASK and WORLD params. Any other combination of params is a breaking change.
# ___________________________________________________________________________________________________________________________________________________
#    TASK     |             WORLD              |                                          TASK DESCRIPTION                                          |  
#             |                                |                                                                                                    |
#   "labs"    |      "world20", "world21"      |       "labs"  will run exclusively the turtlebot from project 1 in simulation                      | 
#             |                                |                                                                                                    |
#   "proj1"   |        "world23proj1"          |       "proj1" will run the live test of the turlebot. DO NOT SELECT THIS AT ALL!                   |
#             |                                |                                                                                                    |
#   "proj2"   |      "world20", "world21"      |       "proj2" will run both the turtebot and the hector in simulation in world20 or world21        |  
#             |                                |                                                                                                    |
# "soloflight"|           "world0"             |       "soloflight" will spawn only the hector in world0, which is a training world that can        |
#             |                                |         be used for testing only the hector. If you use this option, you must ensure that waypoints| 
#             |                                |        are set in the goal file and set co_op param to false in drone_commander.yaml.              |
#             |                                |        See README for more information                                                             |
# ____________|________________________________|____________________________________________________________________________________________________|
