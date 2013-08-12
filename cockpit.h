#ifndef COCKPIT_H
#define COCKPIT_H

#include "viewport.h"
#include "vessel.h"


class Cockpit : public Viewport{
    private:
        Fleet vessels;
    public:
       Cockpit(unsigned int label, int pos_x, int pos_y, int width, int height);
       void Draw(void);

};



#endif //COCKPIT_H
