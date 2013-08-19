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
    //cout << "Cockpit::Draw " << _icons.size() << "\n";
    if(windows[_window_index].mouse_x >= 0 or windows[_window_index].mouse_y >= 0){
        //cout << windows[_window_index].mouse_x << "," << windows[_window_index].mouse_y << "\n";
    }
    windows[_window_index].data_size = _data_size = MAX_SIZE;
    _data_points.clear();
    _data_colour.clear();

    static unsigned int selected_vessel = 2;
    if(vessels.vessels.size() <= selected_vessel){
        selected_vessel = 0;
    }
    //cout << "selected_vessel: " << selected_vessel << "\n";

    if(vessels.vessels.size() > selected_vessel){
        //cout << vessels.vessels[selected_vessel].description << "\n";
        /* Rather than re-position the view, we re-position the boats 
           to stop flickering caused by the delay between the icons being drawn 
           and the glut thread re-positioning the viewport. */
        //SetView(vessels.vessels[selected_vessel].pos_x - MAX_SIZE/2, vessels.vessels[selected_vessel].pos_y - MAX_SIZE/2, 40000, 0);

        double remember_pos_x = vessels.vessels[selected_vessel].pos_x;
        double remember_pos_y = vessels.vessels[selected_vessel].pos_y;

        vessels.vessels[selected_vessel].pos_x = MAX_SIZE/2;
        vessels.vessels[selected_vessel].pos_y = MAX_SIZE/2;

        //Icon icon = vessel_to_draw.PopulateIcon(_window_index);
        Icon icon =  vessels.vessels[selected_vessel].PopulateIcon(_window_index);

        if(vessels.vessels[selected_vessel].pos_x != MAX_SIZE/2 or vessels.vessels[selected_vessel].pos_y != MAX_SIZE/2){
            cout << "WTF\n";
            // another thread has modified this while we have been drawing icon.
        }

        vessels.vessels[selected_vessel].pos_x = remember_pos_x;
        vessels.vessels[selected_vessel].pos_y = remember_pos_y;

        icon.key = selected_vessel;
        Icon_key key;
        key.type = ICON_TYPE_VESSEL;
        key.key = icon.key;

        _icons.clear();
        AddIcon(key, icon);
    }

    //Icon button_left = ButtonLeft((_data_size - _data_size/_zoom) /2, (_data_size + _data_size/_zoom) /2 - 1,
    int win_cent_x = _data_size/2;
    int win_cent_y = _data_size/2;
    int win_scale = _data_size/_zoom;
    int but_l_x = win_cent_x - win_scale /2 + 100;
    int but_l_y = win_cent_y + win_scale /2 - 100;
    int but_r_x = win_cent_x + win_scale /2 - 100;
    int but_r_y = win_cent_y + win_scale /2 - 100;

    Icon button_left = ButtonLeft(but_l_x, but_l_y,
                                  but_l_x - win_scale * windows[_window_index].mouse_x / _width, 
                                  but_l_y - win_scale * (_height - windows[_window_index].mouse_y) / _height,
                                  200, &selected_vessel);
    
    Icon_key key;
    key.type = ICON_TYPE_BUTTON;
    key.key = button_left.key;
    AddIcon(key, button_left);

    Icon button_right = ButtonRight(but_r_x, but_r_y,
                                    but_r_x - win_scale * (_width - windows[_window_index].mouse_x) / _width,
                                    but_r_y - win_scale * (_height - windows[_window_index].mouse_y) / _height,
                                    200, &selected_vessel);

    key.key = button_right.key;
    AddIcon(key, button_right);
    
    Icon button_centre = ButtonCentre(win_cent_x, but_r_y,
                                      win_cent_x - (win_scale * ((int)_width/2 - (int)windows[_window_index].mouse_x) / (int)_width),
                                      but_r_y - win_scale * (_height - windows[_window_index].mouse_y) / _height,
                                      200, &selected_vessel);

    key.key = button_centre.key;
    AddIcon(key, button_centre);

    RedrawIcons();

    Text text;
    text.font = GLUT_BITMAP_TIMES_ROMAN_10;
    
    int pixSize = MAX_SIZE / (_width * _zoom * MIN_RECURSION);
    sprintf(text.text, "PixelSize: %d m", pixSize );
    AddText(10, 10, text);

    sprintf(text.text, "speed:   %.1f  (%.1f)", vessels.vessels[selected_vessel].speed, vessels.vessels[selected_vessel].leeway_speed);
    AddText(10, 20, text);

    sprintf(text.text, "heading: %d", (int)vessels.vessels[selected_vessel].heading);
    AddText(10, 30, text);

    // Mark window for redraw.
    windows[_window_index].dirty = 1;
}

Icon Cockpit::ButtonLeft(int but_pos_x, int but_pos_y, int mouse_pos_x, int mouse_pos_y, int size, unsigned int* p_selected_vessel){
    int colour = 255;

    if(but_pos_x - mouse_pos_x < size and but_pos_y - mouse_pos_y < size){
        colour = 128;
        if(windows[_window_index].mouse_button == GLUT_LEFT_BUTTON){
            if(*p_selected_vessel == 0)
                *p_selected_vessel = vessels.vessels.size() -1;
            else
                --(*p_selected_vessel);
        }
    }

    Icon retvalue;
    retvalue.key = 1;
    retvalue.angle = 0;
    retvalue.scale = size / 20;
    retvalue.centre_x = 0;
    retvalue.centre_y = 0;
    retvalue.pos_x = but_pos_x;
    retvalue.pos_y = but_pos_y;
    retvalue.fixed_size = 0;

    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-8);
    retvalue.points.push_back(0);
    retvalue.points.push_back(8);
    retvalue.points.push_back(8);
    retvalue.points.push_back(8);
    retvalue.points.push_back(-8);

    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);

    return retvalue;
}

Icon Cockpit::ButtonRight(int but_pos_x, int but_pos_y, int mouse_pos_x, int mouse_pos_y, int size, unsigned int* p_selected_vessel){
    int colour = 255;

    if(but_pos_x - mouse_pos_x < size and but_pos_y - mouse_pos_y < size){
        colour = 128;
        if(windows[_window_index].mouse_button == GLUT_LEFT_BUTTON){
            ++(*p_selected_vessel);
        }
    }

    Icon retvalue;
    retvalue.key = 2;
    retvalue.angle = 0;
    retvalue.scale = size / 20;
    retvalue.centre_x = 0;
    retvalue.centre_y = 0;
    retvalue.pos_x = but_pos_x;
    retvalue.pos_y = but_pos_y;
    retvalue.fixed_size = 0;

    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(8);
    retvalue.points.push_back(0);
    retvalue.points.push_back(-8);
    retvalue.points.push_back(-8);
    retvalue.points.push_back(-8);
    retvalue.points.push_back(8);

    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);

    return retvalue;
}

Icon Cockpit::ButtonCentre(int but_pos_x, int but_pos_y, int mouse_pos_x, int mouse_pos_y, int size, unsigned int* p_selected_vessel){
    int colour = 255;
    if(but_pos_x - mouse_pos_x < size/2 and but_pos_x -(int)mouse_pos_x > -size/2 and but_pos_y - mouse_pos_y < size){
        colour = 128;
        if(windows[_window_index].mouse_button == GLUT_LEFT_BUTTON){
            signal keypress;
            keypress.type = SIG_TYPE_ICON_CLICK;
            keypress.source = 1;
            keypress.dest = SIG_DEST_MAP;
            keypress.val = SIG_VAL_SNAP;
            keypress.val2 = vessels.vessels[*p_selected_vessel].pos_x - MAX_SIZE/2;
            keypress.val3 = vessels.vessels[*p_selected_vessel].pos_y - MAX_SIZE/2;
            PushEvent(keypress);

        }
    }

    Icon retvalue;
    retvalue.key = 3;
    retvalue.angle = 0;
    retvalue.scale = size / 20;
    retvalue.centre_x = 0;
    retvalue.centre_y = 0;
    retvalue.pos_x = but_pos_x;
    retvalue.pos_y = but_pos_y;
    retvalue.fixed_size = 0;

    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(-10);
    retvalue.points.push_back(0);
    retvalue.points.push_back(8);
    retvalue.points.push_back(8);
    retvalue.points.push_back(0);
    retvalue.points.push_back(-8);
    retvalue.points.push_back(0);
    retvalue.points.push_back(-8);
    retvalue.points.push_back(0);
    retvalue.points.push_back(8);
    retvalue.points.push_back(0);
    retvalue.points.push_back(0);
    retvalue.points.push_back(-8);

    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(colour);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(20);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);
    retvalue.colour.push_back(0);

    return retvalue;
}
