#ifndef MAP_H
#define MAP_H

#include "viewport.h"



class Map : public Viewport{
    private:
    public:
        Map(unsigned int label, int pos_x, int pos_y, int width, int height) : Viewport(label, pos_x, pos_y, width, height){
        };
        void Draw(void);
        void SetView(float view_x, float view_y, float zoom, int rotation);
};


#endif // MAP_H
