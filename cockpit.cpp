#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

#include "data.h"

#include "cockpit.h"

using namespace std;

Cockpit::Cockpit(unsigned int label, int pos_x, int pos_y, int width, int height) : Viewport(label, pos_x, pos_y, width, height){
    Draw();
    glutSetWindow(_window_index);
    SetView(0, 0, 40000, 0);
}

void Cockpit::Draw(void){
    //cout << "Cockpit::Draw\n";
    if(windows[_window_index].mouse_x >= 0 or windows[_window_index].mouse_y >= 0){
        //cout << windows[_window_index].mouse_x << "," << windows[_window_index].mouse_y << "\n";
    }
    windows[_window_index].data_size = _data_size = MAX_SIZE;
    _data_points.clear();
    _data_colour.clear();

    unsigned int selected_vessel = 2;
    if(vessels.vessels.size() > selected_vessel){
        //cout << vessels.vessels[selected_vessel].description << "\n";
        /* Rather than re-position the view, we re-position the boats 
           to stop flickering caused by the delay between the icons being drawn 
           and the glut thread re-positioning the viewport. */
        //SetView(vessels.vessels[selected_vessel].pos_x - MAX_SIZE/2, vessels.vessels[selected_vessel].pos_y - MAX_SIZE/2, 40000, 0);
        
        Vessel vessel_to_draw = vessels.vessels[selected_vessel];
        vessel_to_draw.pos_x -= vessels.vessels[selected_vessel].pos_x - MAX_SIZE/2;
        vessel_to_draw.pos_y -= vessels.vessels[selected_vessel].pos_y - MAX_SIZE/2;

        Icon icon = vessel_to_draw.PopulateIcon(_window_index);
        //Icon icon =  vessels.vessels[selected_vessel].PopulateIcon(_window_index);
        icon.key = selected_vessel;
        Icon_key key;
        key.type = ICON_TYPE_VESSEL;
        key.key = icon.key;

        vessel_to_draw.pos_x = vessels.vessels[selected_vessel].pos_x;
        vessel_to_draw.pos_y = vessels.vessels[selected_vessel].pos_y;
        vessels.vessels[selected_vessel] = vessel_to_draw;
        
        AddIcon(key, icon);
    }
    RedrawIcons();
}
