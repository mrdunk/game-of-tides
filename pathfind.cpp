#include <math.h>
#include <iostream>

//#include "map.h"

#include "pathfind.h"

using namespace std;

bool PathKey::operator<(const PathKey other) const {
    int recursion_multiplier = 100000;

    double d_this = sqrt((double)(x - dest_x) * (double)(x - dest_x) + (double)(y - dest_y) * (double)(y - dest_y));
    double d_other = sqrt((double)(other.x - dest_x) * (double)(other.x - dest_x) + (double)(other.y - dest_y) * (double)(other.y - dest_y));
    
    if(d_this - recursion * recursion_multiplier != d_other - other.recursion * recursion_multiplier){
        return d_this - recursion * recursion_multiplier  < d_other - other.recursion * recursion_multiplier;
    }
    if(x != other.x){
        return x < other.x;
    }
    return y < other.y;
}

int PathKey::dest_x;
int PathKey::dest_y;

Data CalculatePath::data;

CalculatePath::CalculatePath(MapPoint start, MapPoint end, int min_depth){
    //PathKey init;
    //init.x = end.x;
    //init.y = end.y;
    PathKey::dest_x = end.x;
    PathKey::dest_y = end.y;

    start.calculateZ(data.p_mapData);
    PathKey start_key;
    start_key.x = start.x;
    start_key.y = start.y;
    if(start.tile == 0)
        start.calculateTile(data.p_mapData);
    start_key.recursion = start.tile;

    open_set[start_key] = start;
    closed_set.clear();
}

bool CalculatePath::Process(timestamp_t end_time){
    int steps = 0;
    while(get_timestamp() < end_time and open_set.size()){
        map<PathKey, MapPoint>::iterator it = open_set.begin();
        
        int size = pow(2, it->first.recursion);
        int counter = 0;
        MapPoint test;

        //left side
        while(counter < size){
            test.x = it->second.x -1;
            test.y = it->second.y + counter;
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKey key;
                key.x = test.x;
                key.y = test.y;
                key.recursion = test.tile;
                if(open_set.find(key) == open_set.end() and closed_set.find(key) == closed_set.end()){
                    open_set[key] = test;
                }
                if(key.x == key.dest_x and key.y == key.dest_y){
                    cout << "SUCESS\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile);
        }

        //right side
        counter = 0;
        while(counter < size){
            test.x = it->second.x + size +1;
            test.y = it->second.y + counter;
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKey key;
                key.x = test.x;
                key.y = test.y;
                key.recursion = test.tile;
                if(open_set.find(key) == open_set.end() and closed_set.find(key) == closed_set.end()){
                    open_set[key] = test;
                }
                if(key.x == key.dest_x and key.y == key.dest_y){
                    cout << "SUCESS\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile);
        }

        //bottom side
        counter = 0;
        while(counter < size){
            test.x = it->second.x + counter;
            test.y = it->second.y -1;
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKey key;
                key.x = test.x;
                key.y = test.y;
                key.recursion = test.tile;
                if(open_set.find(key) == open_set.end() and closed_set.find(key) == closed_set.end()){
                    open_set[key] = test;
                }
                if(key.x == key.dest_x and key.y == key.dest_y){
                    cout << "SUCESS\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile);
        }

        //top side
        counter = 0;
        while(counter < size){
            test.x = it->second.x + counter;
            test.y = it->second.y + size +1;
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKey key;
                key.x = test.x;
                key.y = test.y;
                key.recursion = test.tile;
                if(open_set.find(key) == open_set.end() and closed_set.find(key) == closed_set.end()){
                    open_set[key] = test;
                }
                if(key.x == key.dest_x and key.y == key.dest_y){
                    cout << "SUCESS\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile);
        }


        closed_set[it->first] = ++steps;
        open_set.erase(it);

    }
    cout << "open_set.size(): " << open_set.size() << "\n";
    cout << "closed_set.size(): " << closed_set.size() << "\n";
    return 0;
}

void CalculatePath::Display(void* p_map_view){
    int counter = 3;
    for(map<PathKey, MapPoint>::iterator it = open_set.begin(); it != open_set.end(); it++){
        MapPoint point;
        point.x = it->first.x;
        point.y = it->first.y;
        point.tile = it->first.recursion;

        Icon icon = ((Map*)p_map_view)->IconSquare(128);
        icon.pos_x = point.x;
        icon.pos_y = point.y;
        icon.scale = pow(2, point.tile);
        Icon_key key ;
        key.type = ICON_TYPE_TEST;
        key.key = ++counter;
        ((Map*)p_map_view)->AddIcon(key, icon);
    }

    for(map<PathKey, int>::iterator it = closed_set.begin(); it != closed_set.end(); it++){
        MapPoint point;
        point.x = it->first.x;
        point.y = it->first.y;
        point.tile = it->first.recursion;

        Icon icon = ((Map*)p_map_view)->IconSquare(255);
        icon.pos_x = point.x;
        icon.pos_y = point.y;
        icon.scale = pow(2, point.tile);
        Icon_key key ;
        key.type = ICON_TYPE_TEST;
        key.key = ++counter;
        ((Map*)p_map_view)->AddIcon(key, icon);
    }

}





