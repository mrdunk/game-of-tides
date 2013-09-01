#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

#include "data.h"

#include "console.h"


Console::Console(unsigned int label, int pos_x, int pos_y, int width, int height) : Viewport(label, pos_x, pos_y, width, height){
    _data_points.clear();
    _data_colour.clear();

    Draw();
}

void Console::Draw(void){
    //std::cout << "Console::Draw " << Data::mapData.size() << "\n";
    Text text;
    text.font = GLUT_BITMAP_TIMES_ROMAN_10;

    sprintf(text.text, "mapData:  %i", (int)Data::mapData.size());
    AddText(10, _height - 15, text);

    sprintf(text.text, "mapFPS:   %i", mapFPS);
    AddText(10, _height - 25, text);

    sprintf(text.text, "boatsFPS: %i", boatsFPS);
    AddText(10, _height - 35, text);

    // Mark window for redraw.
    windows[_window_index].dirty = 1;
}
