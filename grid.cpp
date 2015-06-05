#include "grid.h"

grid::grid(shape g_size)
{
    int size = g_size.w * g_size.h;
    //Rescale our grid to better approporate the data
    this->grid_size = g_size;
    this->grid_data = std::vector<node>(size);
    for(int i = 0; i < size; i+=4)
    {
        grid_data[i] = node(); grid_data[i].owner = -1;
    }
}
