#include <iostream>

#include "map.h"
#include "data.h"
#include "time_code.c"

using namespace std;

Data Map::data;

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << flush << "Map::Draw " << _window_index << " " << _width << "," << _height << "\n" << flush;
    timestamp_t t0 = get_timestamp();

    Clear();

    //const unsigned int min_x = ((0.5f - _view_x) * MAX_SIZE) - (MAX_SIZE / (2 * _zoom));
    //const unsigned int max_x = ((0.5f - _view_x) * MAX_SIZE) + (MAX_SIZE / (2 * _zoom));
    //const unsigned int min_y = ((0.5f - _view_y) * MAX_SIZE) - (MAX_SIZE / (2 * _zoom));
    //const unsigned int max_y = ((0.5f - _view_y) * MAX_SIZE) + (MAX_SIZE / (2 * _zoom));

    DrawSection(0.0f, 0.0f, 1.0f, 1.0f);

    timestamp_t t1 = get_timestamp();
    cout << "Map::Draw took " << (double)(t1 - t0) / 1000000.0L << " seconds.\n";
}

void Map::DrawSection(float x0, float y0, float x1, float y1){

    const unsigned int min_x = ((0.5f - _view_x) * MAX_SIZE) - (((0.5f - x0) / _zoom) * MAX_SIZE);
    const unsigned int min_y = ((0.5f - _view_y) * MAX_SIZE) - (((0.5f - y0) / _zoom) * MAX_SIZE);
    const unsigned int max_x = ((0.5f - _view_x) * MAX_SIZE) - (((0.5f - x1) / _zoom) * MAX_SIZE);
    const unsigned int max_y = ((0.5f - _view_y) * MAX_SIZE) - (((0.5f - y1) / _zoom) * MAX_SIZE);

    /* step size will always be a power of 2.
     * This is important to ensure the display pixels coincide with the data structure recursion levels
     * which means un-necesary recursion through the datastructure need not occur. */
    int step_x = MAX_SIZE / (int)pow(2, (int)log2(_width * _zoom));
    int step_y = MAX_SIZE / (int)pow(2, (int)log2(_height * _zoom));
    //step_x *= 2;
    //step_y *= 2;

    float z_multiplier_wet = 255.0f / (data.Waterlevel() - data.Height_z_min());
    float z_multiplier_dry = 255.0f / (data.Height_z_max() - data.Waterlevel());

    MapPoint tl, tr, bl, br;

    /* The (min_y % step_y) part ensures the display is alligned with the datastructure, preventing un-necesary recursion. */
    cout << "min_x: " << min_x << "\tmax_x: " << max_x << "\tmin_y: " << min_y << "\tmax_y: " << max_y << "\n";
    cout << "step_x: " << step_x << "\tstep_y: " << step_y << "\n";
    for(unsigned int row = min_y - (min_y % step_y); row < max_y; row+=step_y){
        for(unsigned int col = min_x - (min_x % step_x); col < max_x; col+=step_x){
            //cout << col << "," << row << "\n" << flush;
            tl.x = col;
            tl.y = row;
            tl.calculateZ(data.p_mapData);
            tr.x = col + step_x;
            tr.y = row;
            tr.calculateZ(data.p_mapData);
            bl.x = col;
            bl.y = row + step_y;
            bl.calculateZ(data.p_mapData);
            br.x = col + step_x;
            br.y = row + step_y;
            br.calculateZ(data.p_mapData);

            if(tl.x and tl.y and bl.x and bl.y and tr.x and tr.y and br.x and br.y and tl.z and bl.z and tr.z and br.z){
                _data_points.push_back ((float)tl.x / MAX_SIZE);
                _data_points.push_back ((float)tl.y / MAX_SIZE);
                _data_points.push_back ((float)bl.x / MAX_SIZE);
                _data_points.push_back ((float)bl.y / MAX_SIZE);
                _data_points.push_back ((float)tr.x / MAX_SIZE);
                _data_points.push_back ((float)tr.y / MAX_SIZE);

                _data_points.push_back ((float)tr.x / MAX_SIZE);
                _data_points.push_back ((float)tr.y / MAX_SIZE);
                _data_points.push_back ((float)bl.x / MAX_SIZE);
                _data_points.push_back ((float)bl.y / MAX_SIZE);
                _data_points.push_back ((float)br.x / MAX_SIZE);
                _data_points.push_back ((float)br.y / MAX_SIZE);

                _data_colour.push_back (0);
                if(tl.z > data.Waterlevel()){
                    _data_colour.push_back ((float)(data.Height_z_max() - tl.z) * z_multiplier_dry);
                    _data_colour.push_back (0);
                } else {
                    _data_colour.push_back (0);
                    _data_colour.push_back ((float)(tl.z - data.Height_z_min()) * z_multiplier_wet);
                }
                _data_colour.push_back (0);
                if(bl.z > data.Waterlevel()){
                    _data_colour.push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                    _data_colour.push_back (0);
                } else {
                    _data_colour.push_back (0);
                    _data_colour.push_back ((float)(bl.z - data.Height_z_min()) * z_multiplier_wet);
                }
                _data_colour.push_back (0);
                if(tr.z > data.Waterlevel()){
                    _data_colour.push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                    _data_colour.push_back (0);
                } else {        
                    _data_colour.push_back (0);
                    _data_colour.push_back ((float)(tr.z - data.Height_z_min()) * z_multiplier_wet);
                }

                _data_colour.push_back (0);
                if(tr.z > data.Waterlevel()){
                    _data_colour.push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                    _data_colour.push_back (0);
                } else {        
                    _data_colour.push_back (0); 
                    _data_colour.push_back ((float)(tr.z - data.Height_z_min()) * z_multiplier_wet);
                }
                _data_colour.push_back (0);
                if(bl.z > data.Waterlevel()){
                    _data_colour.push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                    _data_colour.push_back (0);
                } else {        
                    _data_colour.push_back (0);
                    _data_colour.push_back ((float)(bl.z - data.Height_z_min()) * z_multiplier_wet);
                }
                _data_colour.push_back (0);
                if(br.z > data.Waterlevel()){
                    _data_colour.push_back ((float)(data.Height_z_max() - br.z) * z_multiplier_dry);
                    _data_colour.push_back (0);
                } else {        
                    _data_colour.push_back (0);
                    _data_colour.push_back ((float)(br.z - data.Height_z_min()) * z_multiplier_wet);
                }
            }
        }
    }

}

void Map::SetView(float view_x, float view_y, float zoom, int rotation){
    cout << "Map::SetView\n";
}
