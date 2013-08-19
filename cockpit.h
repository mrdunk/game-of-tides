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
        Icon ButtonLeft(int but_pos_x, int but_pos_y, int mouse_pos_x, int mouse_pos_y, int size, unsigned int* p_selected_vessel);
        Icon ButtonRight(int but_pos_x, int but_pos_y, int mouse_pos_x, int mouse_pos_y, int size, unsigned int* p_selected_vessel);
        Icon ButtonCentre(int but_pos_x, int but_pos_y, int mouse_pos_x, int mouse_pos_y, int size, unsigned int* p_selected_vessel);
};



#endif //COCKPIT_H
