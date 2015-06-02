#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_
#include "grid.h"
struct progressbar
{
    virtual void draw(SDL_Renderer *render){}
    shape myShape;
};
struct box_bar : public progressbar
{
public:
    virtual void draw(SDL_Renderer *render)
    {
        for(int i = 0;i < box_count; i++)
        {
            SDL_Rect fillrect = SDL_Rect{myShape.p.x + (i * (box_size+1)), myShape.p.y, box_size - 1, box_size - 1};
            SDL_SetRenderDrawColor(render,255, 0, 128,175);
            SDL_RenderFillRect( render, &fillrect );
        }
    }
    int box_size;
    int box_count;
};

#endif // PROGRESSBAR_H
