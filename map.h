#ifndef MAP_H
#define MAP_H

#include "viewport.h"
#include "data.h"


class Map : public Viewport{
    private:
        static Data data;
    public:
        Map(unsigned int label, int pos_x, int pos_y, int width, int height) : Viewport(label, pos_x, pos_y, width, height){
        };
        void Draw(void);
        void DrawSection(float x0, float y0, float x1, float y1);
        void SetView(float view_x, float view_y, float zoom, int rotation);
};


#endif // MAP_H
