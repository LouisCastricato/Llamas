#include <iostream>
#include "grid.h"
#define CLIENT
#include "tcputil.h"
#include <cmath>
#include <sstream>
#include <string>
#include <cstring>
#include "SDL2_gfx-1.0.1/SDL2_gfxPrimitives.h"
#include "progressbar.h"
using namespace std;
grid game_world;
bool quit = false;
point camera;
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* Sans = NULL;
#ifdef CLIENT
char *socket_data;
TCPsocket sock;
SDLNet_SocketSet set;
#endif
box_bar remaining_blocks;
//Are we currently building our fortress?
bool fortressBuildPhase = false;

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define GRID_W 35
#define GRID_H 35
#define node_size 25
#define MOVE_SPEED 40
#define block_col_size 10
#define goal_col_size 2


SDL_Palette block_col;
struct node_pos
{
    point p;
    node n;
};
//Changes the taem has made during this turn
std::vector<node_pos> team_changes;
//Changes you have made during this turn
std::vector<node_pos> changes;

//Blocks can only be placed within a fortress
shape FortShape;
struct score
{
    int changes_left;
};
//Where this player is in the scheme of things
score myScore;
void init_cols();
void draw();
void retMsg(char *msg);

int main(int argc, char *argv[])
{
    SDL_Init( SDL_INIT_EVERYTHING );

    remaining_blocks = box_bar();
    remaining_blocks.box_size = 30;
    remaining_blocks.myShape.p.x=10;

    if (TTF_Init() == -1)
    {
      printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());}


    shape grid_shape;
    grid_shape.w = GRID_W;
    grid_shape.h = GRID_H;
    camera = point(0,0);
    game_world = grid(grid_shape);
    gWindow = SDL_CreateWindow( "LLamas", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1680, 1080, SDL_WINDOW_SHOWN );
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );

    TTF_Init();
    Sans = TTF_OpenFont("/home/louis/Llamas/Neon.ttf", 28);


    remaining_blocks.myFont = Sans;
    if (Sans == nullptr){
        std::cout << TTF_GetError() << std::endl;
        return 0;
    }
#ifdef CLIENT
    char *myName = "LOUIS-1";
    IPaddress ip;


    SDLNet_Init();
    set=SDLNet_AllocSocketSet(1);
    if(SDLNet_ResolveHost(&ip,"localhost",69878)==-1)
    {
        printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        exit(5);
    };
    sock=SDLNet_TCP_Open(&ip);
    if(!sock)
        {
            printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
            SDLNet_Quit();
            SDL_Quit();
            exit(6);
        }
    SDLNet_TCP_AddSocket(set,sock);

    if(!putMsg(sock,myName))
    {
        SDLNet_TCP_Close(sock);
        SDLNet_Quit();
        SDL_Quit();
        exit(8);
    }

    //Collect the data on our world
    getMsg(sock,&socket_data);
    //Process the data we just collected
    retMsg(socket_data);
#endif
    init_cols();
    myScore.changes_left = 5;

    draw();
    SDL_Quit();
    return 0;
}
void init_cols()
{
    //Different team colors
    block_col.colors = new SDL_Color[block_col_size];
    block_col.colors[0] = {0,247,247,41};
    block_col.colors[1] = {0,255,8,0};
    block_col.colors[2] = {0,143,0,255};
}
void close()
{
    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}
void draw()
{

    int x, y;
    SDL_Event e;
    while(!quit)
    {
        auto numready=SDLNet_CheckSockets(set, 100);
        //Current window width and height
        int w,h;
        SDL_GetWindowSize(gWindow,&w,&h);
        if(numready==-1)
        {
            printf("SDLNet_CheckSockets: %s\n",SDLNet_GetError());
            break;
        }
        if(numready && SDLNet_SocketReady(sock)){
            getMsg(sock,&socket_data);
            //Process the data we just collected
            retMsg(socket_data);
        }
        while(SDL_PollEvent(&e) !=0)
        {
            switch(e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                /* Check the SDLKey values and move change the coords */
                switch( e.key.keysym.sym ){
                case SDLK_SPACE:
#ifdef CLIENT
                    putMsg(sock,"space");
                    //See if there is a new world to download

#else
                    for(int i = 0; i < changes.size(); i++)
                    {
                        game_world.addNode(changes[i].n, changes[i].p);
                    }
                    game_world.updateItt();
#endif
                    break;
                case SDLK_MINUS:
                    //Decrease the index of ammo we're on
                    remaining_blocks.active_ammo_index--;
                    //Make sure our index is above zero
                    if(remaining_blocks.active_ammo_index < 0)
                        remaining_blocks.active_ammo_index = remaining_blocks.ammos.size() -1;
                    break;
                case SDLK_EQUALS:
                    //Increment our current index
                    remaining_blocks.active_ammo_index++;
                    //Make sure that ammo is properly cycling
                    if(remaining_blocks.active_ammo_index == remaining_blocks.ammos.size())
                        remaining_blocks.active_ammo_index = 0;
                    break;
                case SDLK_BACKSPACE:
                    if(changes.size()!=0){
#ifdef CLIENT
                    putMsg(sock,"bckspace");
#endif
                    changes.erase(changes.begin()+ changes.size() - 1); remaining_blocks.ammos[remaining_blocks.active_ammo_index].count++;}
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:if(remaining_blocks.ammos[remaining_blocks.active_ammo_index].count > 0){
                if(e.button.button == SDL_BUTTON_LEFT){
                    x = e.button.x; y = e.button.y;
                    x -= camera.x; y -= camera.y;
                    //Are we within the playing field?
                    if((x > 0)&&(x < (GRID_W + 1) * node_size))
                        if((y > 0)&&(y < (GRID_H + 1) * node_size))
                        {

                            float x1, y1;
                            x1 = y1 = 0.0f;
                            x1 = (float)x / (float)(node_size+ 1);
                            y1 = (float)y / (float)(node_size+ 1);
                            x1 = (int)x1;
                            y1 = (int)y1;

                            point final_point = point ( x1, y1);
                            node_pos newchange;

                            //Check to see if we're within our fortress
                            if(((x1 > FortShape.p.x -FortShape.w)&&(y1 > FortShape.p.y - FortShape.h)) && ((x1 < FortShape.p.x + FortShape.w)&&(y1 < FortShape.p.y + FortShape.h)))
                            if(game_world.getNode(game_world.convertPointToLinear(final_point)).type == node::node_types::Empty){
#ifdef CLIENT
                                stringstream stream;
                                stream << "ADD " << x1 << " " << y1 << " " << remaining_blocks.ammos[remaining_blocks.active_ammo_index].type;
                                putMsg(sock,(char*)stream.str().c_str());
#endif
                                newchange.n = node(0,remaining_blocks.ammos[remaining_blocks.active_ammo_index].type,0);
                                newchange.p = final_point;
                                changes.push_back(newchange);
                                remaining_blocks.ammos[remaining_blocks.active_ammo_index].count--;
                            }
                        }
                   }
                else if(e.button.button == SDL_BUTTON_RIGHT){
                    x = e.button.x; y = e.button.y;
                    x -= camera.x; y -= camera.y;

                    if((x > 0)&&(x < (GRID_W + 1) * node_size))
                        if((y > 0)&&(y < (GRID_H + 1) * node_size))
                        {
                            float x1, y1;
                            x1 = y1 = 0.0f;
                            x1 = (float)x / (float)(node_size+ 1);
                            y1 = (float)y / (float)(node_size+ 1);
                            x1 = (int)x1;
                            y1 = (int)y1;

                            point final_point = point ( x1, y1);
                            node_pos newchange;

                            //Check to see if we're within our fortress
                            if(((x1 > FortShape.p.x -FortShape.w)&&(y1 > FortShape.p.y - FortShape.h)) && ((x1 < FortShape.p.x + FortShape.w)&&(y1 < FortShape.p.y + FortShape.h)))
                            if(game_world.getNode(game_world.convertPointToLinear(final_point)).type == node::node_types::Block){
#ifdef CLIENT
                                stringstream stream;
                                stream << "DEL " << x1 << " " << y1;
                                putMsg(sock,(char*)stream.str().c_str());
#endif
                                newchange.n = node(0,node::node_types::Empty,0);
                                newchange.p = final_point;
                                changes.push_back(newchange);
                                remaining_blocks.ammos[remaining_blocks.active_ammo_index].count--;
                            }
                        }
                 }
                 }
                break;
            case SDL_KEYDOWN:
                point accel = point(0,0);
                /* Check the SDLKey values and move change the coords */
                if(e.key.keysym.sym ==SDLK_LEFT)
                    accel.x+=MOVE_SPEED;
                if(e.key.keysym.sym ==SDLK_RIGHT)
                    accel.x-=MOVE_SPEED;
                if(e.key.keysym.sym ==SDLK_UP)
                    accel.y+=MOVE_SPEED;
                if(e.key.keysym.sym ==SDLK_DOWN)
                    accel.y-=MOVE_SPEED;
                camera.x += accel.x;
                camera.y += accel.y;
                break;
            }
        }

        //SDL_Delay(15);
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        for(int i = 0; i < game_world.getGridSize(); i++)
        {
            auto cur_node = game_world.getNode(i);
            point node_point = game_world.convertLinearToPoint(i);
            auto t= node_point;
            node_point.x *= node_size;
            node_point.y *= node_size;
            //Leave a small gap between the blocks
            node_point.x += camera.x + t.x;
            node_point.y += camera.y + t.y;

            SDL_Rect node_rect = { node_point.x, node_point.y, node_size, node_size };
            switch(cur_node.type)
            {
            case(node::node_types::Block):
                SDL_SetRenderDrawColor(gRenderer,block_col.colors[cur_node.owner].r,
                        block_col.colors[cur_node.owner].g, block_col.colors[cur_node.owner].b,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Fortress):
                SDL_SetRenderDrawColor(gRenderer,block_col.colors[cur_node.owner].r,
                        128, 128, 255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Goal):
                filledCircleRGBA(gRenderer,node_point.x + (node_size / 2), node_point.y + (node_size / 2),node_size / 2,51,204,0,255);
                break;
            }
        }
        for(int i = 0; i < team_changes.size(); i++)
        {
            auto cur_node = team_changes.at(i);
            point node_point = cur_node.p;
            auto t= node_point;
            node_point.x *= node_size;
            node_point.y *= node_size;
            //Leave a small gap between the blocks
            node_point.x += camera.x + t.x;
            node_point.y += camera.y + t.y;
            SDL_Rect node_rect = { node_point.x, node_point.y, node_size, node_size };
            //Render these in different colors so you can differentiate between teams
            switch(cur_node.n.type)
            {
            case(node::node_types::Block):
                SDL_SetRenderDrawColor(gRenderer,119,
                        136, 153,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Fortress):
                SDL_SetRenderDrawColor(gRenderer,0,
                        128, 128,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Empty):
                SDL_SetRenderDrawColor(gRenderer,255,
                        255, 255,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            }
        }
        for(int i = 0; i < changes.size(); i++)
        {
            auto cur_node = changes.at(i);
            point node_point = cur_node.p;
            auto t= node_point;
            node_point.x *= node_size;
            node_point.y *= node_size;
            //Leave a small gap between the blocks
            node_point.x += camera.x + t.x;
            node_point.y += camera.y + t.y;
            SDL_Rect node_rect = { node_point.x, node_point.y, node_size, node_size };
            switch(cur_node.n.type)
            {
            case(node::node_types::Block):
                SDL_SetRenderDrawColor(gRenderer,128,
                        128, 128,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Fortress):
                SDL_SetRenderDrawColor(gRenderer,47,
                        79, 79,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Empty):
                SDL_SetRenderDrawColor(gRenderer,255,
                        255, 255,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            }
        }
        SDL_Color darkBlue = {0,255,0};
        int text_w,text_h;
        TTF_SizeText(Sans, (char*)"BUILD_PHASE",&text_w,&text_h);
        SDL_Surface* surfaceMessage = TTF_RenderText_Blended(Sans, (char*)"BUILD PHASE", darkBlue); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

        SDL_Texture* Message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); //now you can convert it into a texture

        SDL_Rect Message_rect; //create a rect
        Message_rect.x = w - text_w;  //controls the rect's x coordinate
        Message_rect.y =  0; // controls the rect's y coordinte
        Message_rect.w = text_w; // controls the width of the rect
        Message_rect.h = text_h; // controls the height of the rect

        SDL_RenderCopy(gRenderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);

        //Are we rendering the number of cells left to build for this turn or are we rendering the number of fortress walls left that we can build?
        remaining_blocks.draw(gRenderer);

        //Draw the bounds of the fortress
        SDL_SetRenderDrawColor( gRenderer,40,50,50,250);
        SDL_Rect fortrect = SDL_Rect{(FortShape.p.x - FortShape.w) * (node_size + 1) +camera.x ,(FortShape.p.y - FortShape.h)* (node_size + 1) + camera.y, 2 * FortShape.w * (node_size + 1),2 * FortShape.h * (node_size + 1)};
        SDL_RenderDrawRect(gRenderer, &fortrect);

        //Update screen
        SDL_RenderPresent( gRenderer );
    }
    close();
}
void retMsg(char *msg)
{
    const char delimiters[] = " .,;:!-";
    char *token, *cp;
    
    // cp = strdupa(msg);
    cp = (char*)malloc(strlen(msg) + 1);
    strcpy (cp, msg);
    
    token = strtok(cp,delimiters);

    if(!strcasecmp(token,"WORLD"))
    {
        //Get our position
        int w,h;
        token = strtok(NULL,delimiters);
        w = atoi(token);
        token = strtok(NULL,delimiters);
        h = atoi(token);
        shape s = shape();
        s.w = w;
        s.h = h;
        game_world = grid(s);
        for(int i = 0; i < s.w * s.h; i++)
        {
            node tempNode;
            tempNode.type = atoi(strtok(NULL,delimiters));
            if(tempNode.type != 0)
            {
                tempNode.type = tempNode.type + 1;
                tempNode.type = tempNode.type -1;
            }
            tempNode.owner = atoi(strtok(NULL,delimiters));
            //Add our newly found node to the entire list
            game_world.addNode(tempNode,game_world.convertLinearToPoint(i));
        }
        //Get this turns new fortress bounds
        FortShape.w = atoi(strtok(NULL,delimiters));
        FortShape.h = atoi(strtok(NULL,delimiters));
        FortShape.p.x = atoi(strtok(NULL,delimiters));
        FortShape.p.y = atoi(strtok(NULL,delimiters));
        //Reset the scores and changes
        remaining_blocks.resetScores();
        changes.clear();
        team_changes.clear();
    }
    //You can view other players on your team building their fort & cells before the turn ends
    else if(!strcasecmp(token,"FORTDYN"))
    {
        //Get our position
        int length;
        token = strtok(NULL,delimiters);
        length = atoi(token);
        for(int i = 0; i < length; i++)
        {
            node_pos changed_node;
            changed_node.p.x =  atoi(strtok(NULL,delimiters));
            changed_node.p.y = atoi(strtok(NULL,delimiters));
            changed_node.n.type =atoi(strtok(NULL,delimiters));
            //Add our newly found node to the entire list
            team_changes.push_back(changed_node);
        }
    }
}

