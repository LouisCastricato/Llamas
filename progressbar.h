#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_
#include "grid.h"
struct progressbar
{
    virtual void draw(SDL_Renderer *render){}
    SDL_Color myColor;
    shape myShape;
};
struct Ammo
{
    SDL_Color ammo;
    int count;
    int box_size;
    int type;
};

struct box_bar : public progressbar
{
public:
    TTF_Font *myFont;
    virtual void draw(SDL_Renderer *render)
    {
        //Render our current ammo index
        renderAmmoIndex(active_ammo_index,render,0,0,250);

        //Render what the next ammo is as well. If we're currently on the last ammo in the list, just render the first index
        if(active_ammo_index + 1 == ammos.size())
            renderAmmoIndex(0,render,0,box_size,160);
        else
            renderAmmoIndex(active_ammo_index + 1,render,0,box_size,160);
    }
    void renderAmmoIndex(int index, SDL_Renderer *render, int x_trans, int y_trans, int Alpha)
    {
        //Render the main place holder box
        SDL_Rect fillrect = SDL_Rect{myShape.p.x + x_trans, myShape.p.y + y_trans, box_size - 1, box_size - 1};
        SDL_SetRenderDrawColor(render,ammos[index].ammo.r,ammos[index].ammo.g,ammos[index].ammo.b,Alpha);
        SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

        SDL_RenderFillRect( render, &fillrect );

        //Create a stream for how many blocks we have left
        std::stringstream s_stream;
        s_stream << " x" << ammos[index].count;
        int text_w,text_h;
        TTF_SizeText(myFont, s_stream.str().c_str(),&text_w,&text_h);

        //Shift the current alpha value to temporary storage in order tor ender our text with the modified alpha
        SDL_Color temp_col = {ammos[index].ammo.r,ammos[index].ammo.g,ammos[index].ammo.b,Alpha};
        SDL_Surface* surfaceMessage = TTF_RenderText_Blended(myFont,  s_stream.str().c_str(), temp_col); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

        SDL_Texture* Message = SDL_CreateTextureFromSurface(render, surfaceMessage); //now you can convert it into a texture

        SDL_SetTextureBlendMode(Message,SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod( Message, Alpha);

        SDL_Rect Message_rect; //create a rect
        Message_rect.x = myShape.p.x + box_size + 1 + x_trans;  //controls the rect's x
        Message_rect.y =  myShape.p.y + y_trans; // controls the rect's y
        Message_rect.w = text_w; // controls the width of the rect
        Message_rect.h = text_h; // controls the height of the rect

        SDL_RenderCopy(render, Message, NULL, &Message_rect);
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);
    }
    //A quick default reset function in the event of an error
    void resetScores()
    {
        for(int i = 0; i < ammos.size(); i++)
        {
            ammos[i].count = 5;
        }
    }

    box_bar()
    {
        active_ammo_index = 0;
        ammos = std::vector<Ammo>(2);
        Ammo Cells;
        //Starting position for our cell ammo
        Cells.ammo = {255, 0, 128,175};
        Cells.box_size =30;
        Cells.count = 5;
        Cells.type = node::node_types::Block;
        ammos[0] = (Cells);

        Ammo neutral_blocks;
        neutral_blocks.ammo = {255, 128, 10,10};
        neutral_blocks.count = 10;
        neutral_blocks.box_size = 30;
        neutral_blocks.type = node::node_types::Fortress;
        ammos[1] = (neutral_blocks);
    }
    //Which ammo are we currently using?
    int active_ammo_index;
    //the ammos that are available to this user
    std::vector<Ammo> ammos;
    //How big should each individual box be?
    int box_size;
};

#endif // PROGRESSBAR_H
