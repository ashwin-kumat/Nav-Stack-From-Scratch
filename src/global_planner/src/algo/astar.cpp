#include "astar.h"
Astar::Node::Node() 
    : g(0), h(0), visited(false), idx(-1, -1), parent(-1, -1) 
    {}

Astar::Open::Open() 
    : f(0), idx(-1, -1) 
    {}

Astar::Open::Open(double f, bot_utils::Index idx) 
    : f(f), idx(idx) 
    {}

Astar::Astar(bot_utils::MapData &map) // assumes the size of the grid is always the same
    : start(-1, -1), goal(-1, -1), map_(map), nodes(map.map_size_.i * map.map_size_.j), open_list()
    {
        int k = 0;
        for (int i = 0; i < map_.map_size_.i; ++i)
        {
            for (int j = 0; j < map_.map_size_.j; ++j)
            {
                nodes[k].idx.i = i;
                nodes[k].idx.j = j;
                ++k;
            }
        }
    }

void Astar::add_to_open(Node * node)
{   // sort node into the open list
    double node_f = node->g + node->h;

    for (int n = 0; n < open_list.size(); ++n)
    {
        Open & open_node = open_list[n];
        if (open_node.f > node_f + 1e-5)
        {   // the current node in open is guaranteed to be more expensive than the node to be inserted ==> insert at current location            
            open_list.emplace(open_list.begin() + n, node_f, node->idx);
            return;
        }
    }
    // at this point, either open_list is empty or node_f is more expensive that all open nodes in the open list
    open_list.emplace_back(node_f, node->idx);
}

Astar::Node * Astar::poll_from_open()
{   
    bot_utils::Index & idx = open_list.front().idx; //ref is faster than copy
    int k = flatten(idx);
    Node * node = &(nodes[k]);

    open_list.pop_front();

    return node;
}

std::vector<bot_utils::Pos2D> Astar::plan(bot_utils::Pos2D pos_start, bot_utils::Pos2D pos_goal)
{
    std::vector<bot_utils::Index> path_idx = plan(pos2idx(pos_start), pos2idx(pos_goal));
    std::vector<bot_utils::Pos2D> path;
    for (bot_utils::Index & idx : path_idx)
    {
        path.push_back(idx2pos(idx));
    }

    std::vector<bot_utils::Pos2D> post_processed = post_process(path);
    return post_processed;
}

std::vector<bot_utils::Index> Astar::plan(bot_utils::Index idx_start, bot_utils::Index idx_goal)
{
    std::vector<bot_utils::Index> path_idx; // clear previous path

    // initialise data for all nodes
    for (Node & node : nodes)
    {
        node.h = dist_oct(node.idx, idx_goal);
        node.g = 1e5; // a reasonably large number. You can use infinity in clims as well, but clims is not included
        node.visited = false;
    }

    // set start node g cost as zero
    int k = flatten(idx_start);
    Node * node = &(nodes[k]);
    node->g = 0;

    // add start node to openlist
    add_to_open(node);

    // main loop
    while (!open_list.empty())
    {
        // (1) poll node from open
        node = poll_from_open();

        // (2) check if node was visited, and mark it as visited
        if (node->visited)
        {   // if node was already visited ==> cheapest route already found, no point expanding this anymore
            continue; // go back to start of while loop, after checking if open list is empty
        }
        node->visited = true; // mark as visited, so the cheapest route to this node is found


        // (3) return path if node is the goal
        if (node->idx.i == idx_goal.i && node->idx.j == idx_goal.j)
        {   // reached the goal, return the path
            //ROS_INFO("[Global Planner - Astar]: Goal found. Generating path!");

            path_idx.push_back(node->idx);

            while (node->idx.i != idx_start.i || node->idx.j != idx_start.j)
            {   // while node is not start, keep finding the parent nodes and add to open list
                k = flatten(node->parent);
                node = &(nodes[k]); // node is now the parent

                path_idx.push_back(node->idx);
            }

            break;
        }

        // (4) check neighbors and add them if cheaper
        bool is_cardinal = true;
        for (int dir = 0; dir < 8; ++dir)
        {   // for each neighbor in the 8 directions

            // get their index
            bot_utils::Index &idx_nb_relative = NB_LUT[dir];
            bot_utils::Index idx_nb(
                node->idx.i + idx_nb_relative.i,
                node->idx.j + idx_nb_relative.j
            );

            // check if in map and accessible
            if (!testPos(idx2pos(idx_nb)))
            {   // if not, move to next nb
                continue;
            }

            // get the cost if accessing from node as parent
            double g_nb = node->g;
            if (is_cardinal) 
                g_nb += 1;
            else
                g_nb += M_SQRT2;
            // the above if else can be condensed using ternary statements: g_nb += is_cardinal ? 1 : M_SQRT2;

            // compare the cost to any previous costs. If cheaper, mark the node as the parent
            int nb_k = flatten(idx_nb);
            Node & nb_node = nodes[nb_k]; // use reference so changing nb_node changes nodes[k]
            if (nb_node.g > g_nb + 1e-5)
            {   // previous cost was more expensive, rewrite with current
                nb_node.g = g_nb;
                nb_node.parent = node->idx;

                // add to open
                add_to_open(&nb_node); // & a reference means getting the pointer (address) to the reference's object.
            }

            // toggle is_cardinal
            is_cardinal = !is_cardinal;
        }
    }

    // clear open list
    open_list.clear();
    return path_idx; // is empty if open list is empty
}

std::vector<bot_utils::Pos2D> Astar::post_process(std::vector<bot_utils::Pos2D> &path)
{
    // LOS los;
    // (1) obtain turning points
    if (path.size() <= 2)
    { // path contains 0 to elements. Nothing to process
        return path;
    }

    // add path[0] (goal) to turning_points
    std::vector<bot_utils::Pos2D> turning_points = {path.front()}; 
    // add intermediate turning points
    for (int n = 2; n < path.size(); ++n)
    {
        bot_utils::Pos2D &pos_next = path[n];
        bot_utils::Pos2D &pos_cur = path[n - 1];
        bot_utils::Pos2D &pos_prev = path[n - 2];

        double Dx_next = pos_next.x - pos_cur.x;
        double Dy_next = pos_next.y - pos_cur.y;
        double Dx_prev = pos_cur.x - pos_prev.x;
        double Dy_prev = pos_cur.y - pos_prev.y;

        // use 2D cross product to check if there is a turn around pos_cur
        if (abs(Dx_next * Dy_prev - Dy_next * Dx_prev) > 1e-5)
        {   // cross product is small enough ==> straight
            turning_points.push_back(pos_cur);
        }
    }
    // add path[path.size()-1] (start) to turning_points
    turning_points.push_back(path.back());
    std::vector<bot_utils::Pos2D> post_process_path;
    post_process_path.push_back(turning_points.front()); //the front of the array is th end
    int x = 0;
    bot_utils::Index curr = pos2idx(post_process_path.at(x));

    for (int i = 1 ; i < turning_points.size()-1 ; ++i)
    {
        bot_utils::Index to_test = pos2idx(turning_points.at(i+1));
        std::vector<bot_utils::Index> ray = bresenham_los(curr , to_test);
        bool islos = true;

        for (auto &id : ray)
        {
            if (!testPos(idx2pos(id)))
            {
                islos = false;
                break;
            }
        }
        if (!islos)
        {
            x++;
            post_process_path.push_back(turning_points.at(i));
            curr = pos2idx(post_process_path.at(x));
        }
    }
    post_process_path.push_back(turning_points.back());
    return post_process_path;
}


int Astar::flatten(bot_utils::Index &idx)
{
    return idx.i * map_.map_size_.j + idx.j;
}

bool Astar::oob(bot_utils::Index &idx)
{
    return (idx.i <=0 || idx.i >= map_.map_size_.i || idx.j < 0 && idx.j >= map_.map_size_.j);
}


bot_utils::Index Astar::pos2idx(bot_utils::Pos2D &pos)
{
    int i = round((pos.x - map_.origin_.x) / map_.cell_size_);
    int j = round((pos.y - map_.origin_.y) / map_.cell_size_);
    return bot_utils::Index(i,j);
}

bot_utils::Pos2D Astar::idx2pos(bot_utils::Index &idx)
{
    double x = idx.i * map_.cell_size_ + map_.origin_.x;
    double y = idx.j * map_.cell_size_ + map_.origin_.y;
    return bot_utils::Pos2D(x,y);
}

bool Astar::testPos(bot_utils::Pos2D pos)
{
    bot_utils::Index idx = pos2idx(pos);
    int key = flatten(idx);

    if (oob(idx))
    {
        return false; //test fail
    }

    if (map_.grid_inflation_.at(key) > 0)
    {
        return false; //in map, but on inflated
    }
    else if (map_.grid_logodds_.at(key) > map_.lo_thresh_)
    {
        return false; //in map, not inflated but log odds occupied
    }
    else
    {
        return true; //in map, not inflated not lo occupied
    }
}