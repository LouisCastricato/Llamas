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

grid *game_world = 0;
std::vector<user> users;
SDLNet_SocketSet set;
TCPsocket server;
void command(char *msg, user *u);
//Pushes a copy of the game world to all clients
std::string pushGameWorld();
SDLNet_SocketSet create_sock();
int main(int argc, char **argv)
{
    IPaddress ip;


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
                    auto world = pushGameWorld();
                    putMsg(newUser.mySocket,(char*)world.c_str());
                }
            }
        }
        int users_working = users.size();
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
        //If there are no users still working
        if((users_working <=0) &&(users.size() != 0))
        {
            //Push pending changes. Done in reverse so that earlier uses have priority
            for(int i =  users.size() - 1; i >= 0; i--)
            {
                //Go through every users change.
                for(int j = 0; j < users[i].changes.size(); j++)
                {
                    //Add the node to the world
                    game_world->addNode(users[i].changes[j].n,users[i].changes[j].p);
                }
            }
            //Do a new update
            game_world->updateItt();
            auto world = pushGameWorld();
            //Send the data to the clients
            for(int i = 0; i < users.size(); i++)
            {
                putMsg(users[i].mySocket,(char*)world.c_str());
                users[i].isDone = false;
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
        int x,y;
        token = strtok(NULL,delimiters);
        x = atoi(token);
        token = strtok(NULL,delimiters);
        y = atoi(token);

        //Add the new change to the list
        point p = point(x,y);
        node newNode;
        newNode.type = node::node_types::Block;
        newNode.owner = u->team;
        node_pos np;
        np.n = newNode;
        np.p = p;
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
