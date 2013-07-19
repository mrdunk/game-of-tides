#include <iostream>

#include "map.h"
#include "data.h"

using namespace std;

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << flush << "Map::Draw " << _window_index << " " << _width << "," << _height << "\n" << flush;
    int step_x = MAX_SIZE / (int)pow(2, (int)log2(_width));
    int step_y = MAX_SIZE / (int)pow(2, (int)log2(_height));
    //float multiplier_x = _width / pow(2, (int)log2(_width));
    //float multiplier_y = _height / pow(2, (int)log2(_height));
    

    _data_points.clear();
    _data_colour.clear();

    unsigned int max_z = 0;
    unsigned int min_z = MAX_SIZE;

    std::unordered_map<std::string, MapPoint> mapData;
    MapPoint tl, tr, bl, br;
    for(int row = step_y; row < MAX_SIZE; row+=step_y){
        for(int col = step_x; col < MAX_SIZE; col+=step_x){
            tl.x = col;
            tl.y = row;
            tl.calculateZ(&mapData);
            tr.x = col + step_x;
            tr.y = row;
            tr.calculateZ(&mapData);
            bl.x = col;
            bl.y = row + step_y;
            bl.calculateZ(&mapData);
            br.x = col + step_x;
            br.y = row + step_y;
            br.calculateZ(&mapData);

            if(max_z < tl.z)
                max_z = tl.z;
            if(min_z > tl.z)
                min_z = tl.z;

            if(tl.x and tl.y and bl.x and bl.y and tr.x and tr.y and br.x and br.y and tl.z and bl.z and tr.z and br.z){
                //cout << ".";
                //cout << tl.z << "\t" << tr.z << "\t" << bl.z << "\t" << br.z << "\n";
                //cout << (float)tl.x / MAX_SIZE << "," << (float)tl.y / MAX_SIZE << "\t" << (float)((int)tl.z - 8314220)/90000 << "\n";
                //cout << (float)tr.x / MAX_SIZE << "," << (float)tr.y / MAX_SIZE << "\t" << 255.0f * ((float)tr.z / MAX_SIZE) << "\n";
                //cout << (float)bl.x / MAX_SIZE << "," << (float)bl.y / MAX_SIZE << "\t" << 255.0f * ((float)bl.z / MAX_SIZE) << "\n";
                //cout << (float)br.x / MAX_SIZE << "," << (float)br.y / MAX_SIZE << "\t" << 255.0f * ((float)br.z / MAX_SIZE) << "\n";

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
            _data_colour.push_back (0);
            _data_colour.push_back (255.0f * ((float)((int)tl.z - 8314220)/90000));
            _data_colour.push_back (0);
            _data_colour.push_back (0);
            _data_colour.push_back (255.0f * ((float)((int)tl.z - 8314220)/90000));
            _data_colour.push_back (0);
            _data_colour.push_back (0);
            _data_colour.push_back (255.0f * ((float)((int)tl.z - 8314220)/90000));

            _data_colour.push_back (0);
            _data_colour.push_back (0);
            _data_colour.push_back (255.0f * ((float)((int)tl.z - 8314220)/90000));
            _data_colour.push_back (0);
            _data_colour.push_back (0);
            _data_colour.push_back (255.0f * ((float)((int)tl.z - 8314220)/90000));
            _data_colour.push_back (0);
            _data_colour.push_back (0);
            _data_colour.push_back (255.0f * ((float)((int)tl.z - 8314220)/90000));
            }


        }
    }
    return;

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

    cout << min_z << "\t" << max_z << "\n";
}
