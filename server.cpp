
#ifndef CLIENT
#include "grid.cpp"
#include "tcputil.h"
#include<map>
#include <sstream>
#include <cstring>
struct node_pos
{
    point p;
    node n;
};

struct user
{
    std::string name;
    std::vector<node_pos> changes;
    TCPsocket mySocket; //Used for simple security checks
    bool isDone;
    int team;
};
std::vector<shape> TeamBoundingBoxes;
grid *game_world = 0;
std::vector<user> users;
SDLNet_SocketSet set;
TCPsocket server;
//The updates made on this iteration
std::vector<int> index_per_team;
std::vector<std::vector<node_pos>> turnChanges;
void command(char *msg, user *u);
//Pushes a copy of the game world to all clients
std::string pushGameWorld();
std::string pushGameWorld_Team(int team);
shape computeBoundingBox(int team);
SDLNet_SocketSet create_sock();
int main(int argc, char **argv)
{
    IPaddress ip;

    turnChanges = std::vector<std::vector<node_pos>>(team_count);
    index_per_team = std::vector<int>(team_count);
    char *message = NULL;
    Uint16 port;

    //initialize SDL
    if(SDL_Init(0)==-1)
    {
        printf("SDL_Init: %s\n",SDL_GetError());
        exit(1);
    }

    // initialize SDL_net
    if(SDLNet_Init()==-1)
    {
        printf("SDLNet_Init: %s\n",SDLNet_GetError());
        exit(2);
    }

    // get the port from the commandline
    port=69878; //Operate on port 9878

    // perform a byte endianess correction for the next printf
    auto ipaddr=SDL_SwapBE32(ip.host);

    // output the IP address nicely
    printf("IP Address : %d.%d.%d.%d\n",
           ipaddr>>24,
           (ipaddr>>16)&0xff,
           (ipaddr>>8)&0xff,
           ipaddr&0xff);

    // Resolve the argument into an IPaddress type
    if(SDLNet_ResolveHost(&ip,NULL,port)==-1)
    {
        printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
        exit(3);
    }

    // open the server socket
    server=SDLNet_TCP_Open(&ip);
    if(!server)
    {
        printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
        exit(4);
    }
    shape world_shape;
    world_shape.h = 35;
    world_shape.w = 35;
    game_world = new grid(world_shape);

    game_world->addNode(node(0,node::node_types::Goal,0),point(35/2,0));
    game_world->addNode(node(1,node::node_types::Goal,1),point(35/2,34));

    users = std::vector<user>();
    TeamBoundingBoxes = std::vector<shape>(team_count);
    for(int i = 0; i < team_count; i++)
    {
        TeamBoundingBoxes[i] = computeBoundingBox(i);
    }
    while(1)
    {
        set=create_sock();
        auto numready=SDLNet_CheckSockets(set, (Uint32)-1);
        if(numready==-1)
        {
            printf("SDLNet_CheckSockets: %s\n",SDLNet_GetError());
            break;
        }
        if(SDLNet_SocketReady(server))
        {
            //Allow for a new connection
            auto sock=SDLNet_TCP_Accept(server);
            if(sock)
            {
                char *userName = NULL;
                if(getMsg(sock,&userName))
                {
                    //Connect the new user
                    user newUser;
                    newUser.name = std::string(userName);
                    newUser.mySocket = sock;
                    newUser.changes = std::vector<node_pos>();
                    newUser.isDone = false;
                    newUser.team = users.size() % team_count;
                    users.push_back(newUser);
                    auto world = pushGameWorld_Team(newUser.team);
                    putMsg(newUser.mySocket,(char*)world.c_str());
                }
            }
        }
        int users_working = users.size();
        for(int i = 0; i < team_count; i++)
        {
            turnChanges[i].resize(users_working);
            index_per_team[i] = 0;
        }

        for(int i = 0; i < users.size(); i++)
        {
            if(SDLNet_SocketReady(users[i].mySocket))
            {
                if(getMsg(users[i].mySocket, &message))
                {
                    if(strcmp(message,"bckspace")==0)
                    {
                        //If they want to remove a block, let them
                        if(users[i].changes.size()!=0)
                            users[i].changes.erase(users[i].changes.begin()+users[i].changes.size() - 1);
                    }
                    else if(strcmp(message,"space")==0)
                    {
                        //If they've readied up
                        users[i].isDone = true;
                    }
                    else
                        command(message,&users[i]);
                }
            }
            if(users[i].isDone == true)
                users_working--;
        }
        for(int i = 0; i < team_count; i++)
        {
            //If any changes happened
            if(index_per_team[i] > 0){
                //Push the dynamic changes recorded to the users
                std::stringstream string_stream_dyn;
                //How many changes occured
                string_stream_dyn << "FORTDYN " << index_per_team[i] + 1 << " ";
                for(int j = 0; j < index_per_team[i] + 1; j++)
                {
                    string_stream_dyn << turnChanges[i][j].p.x <<  " " << turnChanges[i][j].p.y << " " << turnChanges[i][j].n.type << " ";
                }
                for(int j = 0; j < users.size(); j++)
                {
                    if(users[j].team == i)
                    {
                        //Put the message out to all users on the team
                        putMsg(users[j].mySocket,(char*)string_stream_dyn.str().c_str());
                    }
                }
            }

        }
        //If there are no users still working
        if((users_working <=0) &&(users.size() != 0))
        {
            //If no one has changed anything don't update
            bool doNotUpdate = true;
            //Push pending changes. Done in reverse so that earlier uses have priority
            for(int i =  users.size() - 1; i >= 0; i--)
            {
                users_working++;
                users[i].isDone = false;
                //Go through every users change.
                for(int j = 0; j < users[i].changes.size(); j++)
                {
                    //Add the node to the world
                    game_world->addNode(users[i].changes[j].n,users[i].changes[j].p);
                    doNotUpdate = false;
                }
            }
            if(!doNotUpdate){
                turnChanges.clear();
                for(int i = 0; i < team_count; i++)
                {
                    TeamBoundingBoxes[i] = computeBoundingBox(i);
                }
                //Do a new update
                game_world->updateItt();
                for(int i = 0; i < team_count; i++)
                {
                    auto world = pushGameWorld_Team(i);
                    //Send the data to the clients
                    for(int j = 0; j < users.size(); j++)
                    {
                        if(users[j].team == i){
                            putMsg(users[j].mySocket,(char*)world.c_str());
                            users[j].isDone = false;
                        }
                    }
                }
            }
        }

    }
    return 0;
}
void command(char *msg, user *u)
{
    const char delimiters[] = " .,;:!-";
    char *token, *cp;

    // cp = strdupa(msg);
    cp = (char*)malloc(strlen(msg) + 1);
    strcpy (cp, msg);
    token= strtok(cp,delimiters);

    if(!strcasecmp(token,"ADD"))
    {
        //Get our position
        int x,y,type;
        token = strtok(NULL,delimiters);
        x = atoi(token);
        token = strtok(NULL,delimiters);
        y = atoi(token);
        token = strtok(NULL,delimiters);
        type = atoi(token);
        //Add the new change to the list
        point p = point(x,y);
        node newNode;
        newNode.type = type;
        //All fortress blocks start with 20 health
        if(type == node::node_types::Fortress)
            newNode.health = 20;
        newNode.owner = u->team;
        node_pos np;
        np.n = newNode;
        np.p = p;
        //Allows for collaborative building between teams during an iteration
        turnChanges[newNode.owner][index_per_team[newNode.owner]] = np;
        index_per_team[newNode.owner]++;

        u->changes.push_back(np);
    }
    if(!strcasecmp(token,"DEL"))
    {
        //Get our position
        int x,y;
        token = strtok(NULL,delimiters);
        x = atoi(token);
        token = strtok(NULL,delimiters);
        y = atoi(token);

        //Add the new change to the list
        point p = point(x,y);
        node newNode;
        newNode.type = node::node_types::Empty;
        node_pos np;
        np.n = newNode;
        np.p = p;
        u->changes.push_back(np);
    }
}
shape computeBoundingBox(int team)
{
    int goal_pos = 0;
    point goalpt;
    //The minimum and maximum points of our bounding box
    int mindist, maxdist;
    mindist= maxdist= 0;
    point min,max;
    min.x = max.x = min.y= max.y = 0;
    //Do a first pass to find the center of the team
    for(int i = 0; i < game_world->getGridSize(); i++)
    {
        //While we're doing this, also look for any fortress blocks on the map
        if(game_world->getNode(i).type == node::node_types::Goal)
            if(game_world->getNode(i).owner == team){
                goal_pos = i;}

    }
    goalpt = game_world->convertLinearToPoint(goal_pos);
    //Do a second pass to find the actual bounding box
    for(int i = 0; i < game_world->getGridSize(); i++)
    {
        //While we're doing this, also look for any fortress blocks on the map
        if(game_world->getNode(i).type == node::node_types::Fortress)
            if(game_world->getNode(i).owner == team){
                point this_point = game_world->convertLinearToPoint(i);
                //Find the manhatten distance
                int dist = this_point.x -goalpt.x+ this_point.y - goalpt.y;
                if(dist < mindist){
                    min = this_point;
                    mindist = dist;
                }
                else if(dist > maxdist){
                    max = this_point;
                    maxdist = dist;
                }
            }

    }

    shape to_return;
    to_return.p = goalpt;
    to_return.w = max.x - goalpt.x + 3;
    to_return.h = max.y - goalpt.y + 3;
    return to_return;
}

std::string pushGameWorld_Team(int team)
{
    std::stringstream string_stream;
    string_stream << "WORLD " << game_world->getGridShape().w << " " << game_world->getGridShape().h << " ";
    for(int i = 0; i < game_world->getGridSize(); i++)
    {
        //Push all relating data into the string stream
        string_stream << game_world->getNode(i).type << " " << game_world->getNode(i).owner << " ";
    }
    auto bb = TeamBoundingBoxes[team];
    string_stream << bb.w << " " << bb.h <<" " << bb.p.x << " " << bb.p.y << " ";
    return string_stream.str();
}
std::string pushGameWorld()
{
    std::stringstream string_stream;
    string_stream << "WORLD " << game_world->getGridShape().w << " " << game_world->getGridShape().h << " ";
    for(int i = 0; i < game_world->getGridSize(); i++)
    {
        //Push all relating data into the string stream
        string_stream << game_world->getNode(i).type << " " << game_world->getNode(i).owner << " ";
    }
    return string_stream.str();
}
SDLNet_SocketSet create_sock()
{
    static SDLNet_SocketSet set=NULL;
    int i;

    if(set)
        SDLNet_FreeSocketSet(set);
    set=SDLNet_AllocSocketSet(users.size()+1);
    if(!set) {
        printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        exit(1); //most of the time this is a major error, but do what you want.
    }
    SDLNet_TCP_AddSocket(set,server);
    for(i=0;i<users.size();i++)
        SDLNet_TCP_AddSocket(set,users[i].mySocket);
    return(set);
}
#endif


