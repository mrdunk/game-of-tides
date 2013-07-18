#include <iostream>

#include "map.h"

using namespace std;

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << "Map::Draw " << _window_index << " " << windows[_window_index].window << "\n";
    _data_points.clear();
    _data_colour.clear();
    cout << "  " << _width << "," << _height << "\n";
    cout << "  " << _pos_x << "," << _pos_y << "\n";

    _data_points.push_back (0.5f);
    _data_points.push_back (1.0f);
    _data_points.push_back (0.6f);
    _data_points.push_back (0.0f);
    _data_points.push_back (0.4f);
    _data_points.push_back (0.0f);

    _data_colour.push_back (0);
    _data_colour.push_back (255);
    _data_colour.push_back (0);
    _data_colour.push_back (0);
    _data_colour.push_back (255);
    _data_colour.push_back (0);
    _data_colour.push_back (0);
    _data_colour.push_back (255);
    _data_colour.push_back (0);
}
