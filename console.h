#ifndef CONSOLE_H
#define CONSOLE_H

#include "viewport.h"
#include "vessel.h"


class Console : public Viewport{

    public:
        Console(unsigned int label, int pos_x, int pos_y, int width, int height);
        void Draw(void);

        int boatsFPS;
        int mapFPS;
};


#endif  // CONSOLE_H
