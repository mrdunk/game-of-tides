#include <iostream>

#include "map.h"
#include "data.h"
#include "time_code.c"

using namespace std;

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << flush << "Map::Draw " << _window_index << " " << _width << "," << _height << "\n" << flush;
    timestamp_t t0 = get_timestamp();

    int step_x = MAX_SIZE / (int)pow(2, (int)log2(_width));
    int step_y = MAX_SIZE / (int)pow(2, (int)log2(_height));
    //step_x *= 2;
    //step_y *= 2;

    _data_points.clear();
    _data_colour.clear();

    Data data;
    float z_multiplier_wet = 255.0f / (data.Waterlevel() - data.Height_z_min());
    float z_multiplier_dry = 255.0f / (data.Height_z_max() - data.Waterlevel());
    cout << data.Height_z_min() << " " << data.Height_z_max() << " " << z_multiplier_wet << " * \n";

    MapPoint tl, tr, bl, br;
    for(int row = step_y; row < MAX_SIZE; row+=step_y){
        for(int col = step_x; col < MAX_SIZE; col+=step_x){
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

    timestamp_t t1 = get_timestamp();
    cout << "Map::Draw took " << (double)(t1 - t0) / 1000000.0L << " seconds.\n";
}

void Map::SetView(float view_x, float view_y, float zoom, int rotation){
    cout << "Map::SetView\n";
}
