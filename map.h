#ifndef MAP_H
#define MAP_H

#include "viewport.h"



class Map : public Viewport{
    public:
        Map(int pos_x, int pos_y, int width, int height) : Viewport(pos_x, pos_y, width, height){};
        void Draw(void);
};


#endif // MAP_H
