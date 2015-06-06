#ifndef GRID_H_
#define GRID_H_
#define SPRITE_SIZE_MULT 1.0f
#include "SDL2/SDL.h"
#define PI 3.14159265
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <algorithm>
#define team_count 2

#include <cstdio>  /* defines FILENAME_MAX */
#include <ctime>
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
struct node
{
    node(int col = 0, int t = 0, int o = 0)
    {
        this->col =col;
        type = t;
        owner = o;
        //By default goals can take 10 hits
        health = 10;
    }
    node(const node &t)
    {
        col = t.col;
        type = t.type;
        owner = t.owner;
        health = t.health;
    }
    //Defines which color this node will be placed in
    int col;
    //Defines what type of block this node currently is
    int type;
    //Who owns this object
    int owner;
    //Thea health of the current block
    int health;
    enum node_types{
        Empty = 0,
        Block,
        PowerUp,
        Goal,
        Fortress,
    };
};
struct point{ public: int x,y; point(int X = 0, int Y=0) { this->x = X; this->y = Y;}};
//The shape of our grid
struct shape{ public: shape(){} int w,h; point p; };

class grid
{
public:
    grid(){}
    //Checks to see if a node is alive
    int check(int index)
    {
        if(index > 0 && index < (grid_size.w * grid_size.h))
            if(grid_data[index].type == node::node_types::Block)
                return 1;
        return 0;
    }
    //A more indepth neighbour check
    int lcheck(int tocheck, int newNode)
    {
        if(newNode > 0 && newNode < (grid_size.w * grid_size.h)){
        if(grid_data[newNode].type == node::node_types::Block){
            //if its empty, don't check if the owners are equal
            if(grid_data[tocheck].type == node::node_types::Empty)
                return 1;
            //If tocheck is a block (The only other viable solution), owners must be checked
            if(grid_data[tocheck].owner == grid_data[newNode].owner)
                return 1;
            //If tocheck is a block (The only other viable solution), colors must be checked
            if(grid_data[tocheck].col == grid_data[newNode].col)
                return 1;
        }}
        return 0;
    }

    grid(shape g_size);
    int convertPointToLinear(point p)
    {
        return (p.y * grid_size.w) + p.x;
    }
    point convertLinearToPoint(int linear)
    {
        int x = linear % grid_size.w;
        return point(x, linear / grid_size.w);
    }
    void addNode(node n, point p)
    {
        int index = convertPointToLinear(p);
        grid_data[index] = n;
    }
    void removeNode(point p)
    {
        int index = convertPointToLinear(p);
        grid_data[index].type = node::node_types::Empty;
        grid_data[index].owner = -1;
        grid_data[index].col = -1;
    }
    node getNode(int i)
    { return grid_data[i]; }
    void updateItt()
    {

        for(int i = 0; i < grid_data.size(); i++)
        {
            if(grid_data[i].type == node::node_types::Empty)
            {
                grid_data[i].type = node::node_types::Empty;
                grid_data[i].owner = -1;
                grid_data[i].col = -1;
            }
            if(grid_data[i].health <= 0){
                grid_data[i].type = node::node_types::Empty;
                grid_data[i].owner = -1;
            }
        }
        std::vector<node> updated_grid = grid_data;

        for(int i = 0; i < updated_grid.size(); i++)
        {
            //Retrieve the point we're sampling around
            auto cur_point = convertLinearToPoint(i);
            std::vector<short> neighbour_itt = std::vector<short>(team_count);
            for(int j = 0; j < team_count; j++)
            {
                neighbour_itt[j] = 0;
            }


            //When a fortress is played we don't care about its neighbours
            if((getNode(i).type!= node::node_types::Fortress) && ((getNode(i).type!= node::node_types::Goal))){
                int n_count =lcheck(i,i - 1) + lcheck(i,i + 1) +
                        lcheck(i,i - grid_size.w) + lcheck(i,i + grid_size.w)
                        + lcheck(i,i - grid_size.w - 1) + lcheck(i,i + grid_size.w + 1)
                        + lcheck(i,i - grid_size.w + 1) + lcheck(i,i + grid_size.w - 1);


                for(int j = -1; j <= 1; j++)//Find our current x translation
                    for(int k = -1; k <= 1; k++)//Find our current y translation
                    {

                        //Our new point to check
                        if(!((j == 0) && (k == 0))){
                            auto new_point = point(j + cur_point.x,k + cur_point.y);
                            if((new_point.x >= 0) && (new_point.y >=0) && (new_point.y < grid_size.h)
                                    && (new_point.x < grid_size.w)){
                                node new_node = getNode(convertPointToLinear(new_point));

                                if(new_node.type == node::node_types::Block){
                                    if( new_node.owner != -1){
                                        neighbour_itt[new_node.owner]++;
                                    }
                                }
                                else if(new_node.type == node::node_types::Fortress){
                                    //Anything that comes in contact with a fortress has its health taken away
                                    if((new_node.owner != grid_data[i].owner) && (new_node.owner != -1)) //Assuming its a different owner of course
                                    {
                                        updated_grid[i].health--;
                                        //This also does damage to our fortress
                                        updated_grid[convertPointToLinear(new_point)].health--;
                                    }
                                }
                                else if((new_node.type == node::node_types::Goal) &&(grid_data[i].type == node::node_types::Block))
                                    if((new_node.owner != grid_data[i].owner) && ( new_node.owner != -1)){
                                        updated_grid[convertPointToLinear(new_point)].health--;
                                    }
                            }
                        }
                    }

                //What owner are we most similiar to?
                int best_owner = std::distance(&neighbour_itt[0], std::max_element(&neighbour_itt[0], &neighbour_itt[0] + neighbour_itt.size()));
                if(n_count == 2)
                    updated_grid[i] = grid_data[i];
                else if(n_count == 3)
                {
                    updated_grid[i].type = node::node_types::Block;
                    updated_grid[i].owner = best_owner;
                }
                else if((n_count < 2) || (n_count > 3))
                {
                    updated_grid[i].type = node::node_types::Empty;
                    updated_grid[i].owner = -1;
                    updated_grid[i].col = -1;
                }
            }
            else{
                updated_grid[i].type = grid_data[i].type;
                updated_grid[i].owner = grid_data[i].owner;
                updated_grid[i].health = 20;
            }
            neighbour_itt.clear();
        }
        //Reset the grid
        reset(grid_size);
        for(int i = 0; i < grid_data.size(); i++)
        {
            //Add nodes back into the grid
            if(updated_grid[i].health <= 0){
                updated_grid[i].type = node::node_types::Empty;
                updated_grid[i].owner = -1;
                updated_grid[i].col = -1;
            }
            addNode(updated_grid[i], convertLinearToPoint(i));
        }
        updated_grid.clear();
    }
    void reset(shape g_size)
    {
        int size = g_size.w * g_size.h;
        //Rescale our grid to better approporate the data
        this->grid_size = g_size;
        this->grid_data = std::vector<node>(size);
        for(int i = 0; i < size; i++)
        {
            grid_data[i] = node(); grid_data[i].owner = -1;
        }
    }

    int getGridSize()
    {return grid_data.size();}
    shape getGridShape()
    { return grid_size; }
private:
    //All of the data that is currently present within the grid
    std::vector<node> grid_data;
    //How big our current grid size is
    shape grid_size;
    //How big are the blocks?
    shape block_size;

};

#endif // GRID_H
