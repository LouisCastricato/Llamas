#include <iostream>
#include "grid.h"
#include <cmath>
#include <sstream>
#include <string>
#include "SDL2_gfx-1.0.1/SDL2_gfxPrimitives.h"
#include "progressbar.h"
using namespace std;
grid game_world;
bool quit = false;
point camera;
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* Sans = NULL;
box_bar remaining_blocks;
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define GRID_W 35
#define GRID_H 35
#define node_size 25

#define block_col_size 10
#define goal_col_size 2
SDL_Palette block_col;
struct node_pos
{
    point p;
    node n;
};

std::vector<node_pos> changes;
struct score
{
    int changes_left;
};
//Where this player is in the scheme of things
score myScore;
void init_cols();
void draw();
int main()
{
    SDL_Init( SDL_INIT_EVERYTHING );


    remaining_blocks.box_size = 30;
    remaining_blocks.myShape.p.x=10;
    remaining_blocks.box_count = 5;

    if (TTF_Init() == -1)
    {
      printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());}


    shape grid_shape;
    grid_shape.h = GRID_W;
    grid_shape.w = GRID_H;
    camera = point(0,0);
    game_world = grid(grid_shape);
    gWindow = SDL_CreateWindow( "LLamas", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1680, 1080, SDL_WINDOW_SHOWN );
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );

    TTF_Init();
    Sans = TTF_OpenFont("/home/louis/LlamaGame/Neon.ttf", 28);

    init_cols();
    myScore.changes_left = 5;
    game_world.addNode(node(0,node::node_types::Goal,0),point(30,30));
    draw();
    SDL_Quit();
    return 0;
}
void init_cols()
{
    block_col.colors = new SDL_Color[block_col_size];
    block_col.colors[0] = {0,247,247,41};
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
void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
    // Note that there is more to altering the bitrate of this
    // method than just changing this value.  See how pixels are
    // altered at the following web page for tips:
    //   http://www.libsdl.org/intro.en/usingvideo.html
    static const int BPP = 4;

    double r = (double)radius;

    for (double dy = 1; dy <= r; dy += 1.0)
    {
        // This loop is unrolled a bit, only iterating through half of the
        // height of the circle.  The result is used to draw a scan line and
        // its mirror image below it.

        // The following formula has been simplified from our original.  We
        // are using half of the width of the circle because we are provided
        // with a center and we need left/right coordinates.

        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        int x = cx - dx;

        // Grab a pointer to the left-most pixel for each half of the circle
        Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
        Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;

        for (; x <= cx + dx; x++)
        {
            *(Uint32 *)target_pixel_a = pixel;
            *(Uint32 *)target_pixel_b = pixel;
            target_pixel_a += BPP;
            target_pixel_b += BPP;
        }
    }
}
void draw_circle(int n_cx, int n_cy, int radius, Uint32 pixel)
{
    // if the first pixel in the screen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:

    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
    SDL_SetRenderDrawColor( gRenderer,51, 204, 0, 0);
    while (x >= y)
    {
        SDL_RenderDrawPoint(gRenderer, (int)(cx + x), (int)(cy + y));
        SDL_RenderDrawPoint(gRenderer, (int)(cx + y), (int)(cy + x));

        if (x != 0)
        {
            SDL_RenderDrawPoint(gRenderer, (int)(cx - x), (int)(cy + y));
            SDL_RenderDrawPoint(gRenderer, (int)(cx + y), (int)(cy - x));
        }

        if (y != 0)
        {
            SDL_RenderDrawPoint(gRenderer, (int)(cx + x), (int)(cy - y));
            SDL_RenderDrawPoint(gRenderer, (int)(cx - y), (int)(cy + x));
        }

        if (x != 0 && y != 0)
        {
            SDL_RenderDrawPoint(gRenderer, (int)(cx - x), (int)(cy - y));
            SDL_RenderDrawPoint(gRenderer, (int)(cx - y), (int)(cy - x));
        }

        error += y;
        ++y;
        error += y;

        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
}
void draw()
{
    int x, y;
    SDL_Event e;
    while(!quit)
    {
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
                    for(int i = 0; i < changes.size(); i++)
                    {
                        game_world.addNode(changes[i].n, changes[i].p);
                    }
                    game_world.updateItt();
                    myScore.changes_left = 5;
                    changes.clear();
                    break;
                case SDLK_BACKSPACE:
                    if(changes.size()!=0){
                    changes.erase(changes.begin()+ changes.size() - 1); myScore.changes_left++;}
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:if(myScore.changes_left > 0){
                if(e.button.button == SDL_BUTTON_LEFT){
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
                            newchange.n = node(0,node::node_types::Block,0);
                            newchange.p = final_point;
                            changes.push_back(newchange);
                            myScore.changes_left--;
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
                            newchange.n = node(0,node::node_types::Empty,0);
                            newchange.p = final_point;
                            changes.push_back(newchange);
                            myScore.changes_left--;
                        }
                 }
                 }
                break;
            case SDL_KEYDOWN:
                point accel = point(0,0);
                /* Check the SDLKey values and move change the coords */
                if(e.key.keysym.sym ==SDLK_LEFT)
                    accel.x+=10;
                if(e.key.keysym.sym ==SDLK_RIGHT)
                    accel.x-=10;
                if(e.key.keysym.sym ==SDLK_UP)
                    accel.y+=10;
                if(e.key.keysym.sym ==SDLK_DOWN)
                    accel.y-=10;
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
                SDL_SetRenderDrawColor(gRenderer,block_col.colors[cur_node.col].r,
                        block_col.colors[cur_node.col].g, block_col.colors[cur_node.col].b,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            case(node::node_types::Goal):
                filledCircleRGBA(gRenderer,node_point.x + (node_size / 2), node_point.y + (node_size / 2),node_size / 2,51,204,0,255);
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
            case(node::node_types::Empty):
                SDL_SetRenderDrawColor(gRenderer,255,
                        255, 255,255);
                SDL_RenderFillRect( gRenderer, &node_rect );
                break;
            }
        }
        /*SDL_Surface* surfaceMessage = TTF_RenderText_Blended(Sans, remaining_moves.c_str(), DarkBlue); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

        SDL_Texture* Message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); //now you can convert it into a texture

        SDL_Rect Message_rect; //create a rect
        Message_rect.x = 10;  //controls the rect's x coordinate
        Message_rect.y = 0; // controls the rect's y coordinte
        Message_rect.w = 350; // controls the width of the rect
        Message_rect.h = 40; // controls the height of the rect

        SDL_RenderCopy(gRenderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);*/
        remaining_blocks.box_count = myScore.changes_left;
        remaining_blocks.draw(gRenderer);
        //Update screen
        SDL_RenderPresent( gRenderer );
    }
    close();
}


