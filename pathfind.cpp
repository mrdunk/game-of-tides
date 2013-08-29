#include <math.h>
#include <iostream>
#include <limits>

//#include "map.h"

#include "pathfind.h"

using namespace std;

bool PathKeyXYZ::operator<(const PathKeyXYZ other) const {
    int recursion_multiplier = 200000;

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

int PathKeyXYZ::dest_x;
int PathKeyXYZ::dest_y;

bool PathKeyXY::operator<(const PathKeyXY other) const {

    //double d_this = sqrt((double)(x - dest_x) * (double)(x - dest_x) + (double)(y - dest_y) * (double)(y - dest_y));
    //double d_other = sqrt((double)(other.x - dest_x) * (double)(other.x - dest_x) + (double)(other.y - dest_y) * (double)(other.y - dest_y));

    //return d_this < d_other;
    if(x != other.x)
        return x < other.x;
    return y < other.y;
}

int PathKeyXY::dest_x;
int PathKeyXY::dest_y;

Data CalculatePath::data;

CalculatePath::CalculatePath(MapPoint start, MapPoint end, int min_depth){
    PathKeyXYZ::dest_x = PathKeyXY::dest_x = end.x;
    PathKeyXYZ::dest_y = PathKeyXY::dest_y = end.y;

    start.calculateZ(data.p_mapData);
    PathKeyXYZ start_key_xyz;
    start_key_xyz.x = start.x;
    start_key_xyz.y = start.y;
    if(start.tile == 0)
        start.calculateTile(data.p_mapData);
    start_key_xyz.recursion = start.tile;

    open_set[start_key_xyz] = start;
    closed_set.clear();
}

bool CalculatePath::Process(timestamp_t end_time){
    timestamp_t t0 = get_timestamp();

    int steps = 0;
    while(get_timestamp() < end_time and open_set.size()){
        // open_set is sorted by distance from destinaton * tilesize so the first element is always the one closest to the destination.
        map<PathKeyXYZ, MapPoint>::iterator it = open_set.begin();

        int speedup = 1;
                
        int size = pow(2, it->first.recursion);
        int counter = 0;
        MapPoint test;

        //left side
        while(counter < size){
            test.x = it->second.x -1;
            test.y = it->second.y + counter;
            test.calculateZ(data.p_mapData);
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKeyXYZ key_xyz;
                PathKeyXY key_xy;
                key_xyz.x = key_xy.x = test.x;
                key_xyz.y = key_xy.y = test.y;
                key_xyz.recursion = test.tile +speedup;
                if(open_set.find(key_xyz) == open_set.end() and closed_set.find(key_xy) == closed_set.end()){
                    open_set[key_xyz] = test;
                }
                if(key_xyz.x == key_xyz.dest_x and key_xyz.y == key_xyz.dest_y){
                    //open_set.clear();
                    cout << "SUCESS\n";
                    timestamp_t t1 = get_timestamp();
                    cout << "CalculatePath::Process took " << (long)(t1 - t0) / 1000000.0L << " seconds.\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile +speedup);
        }

        //right side
        counter = 0;
        while(counter < size){
            test.x = it->second.x + size +1;
            test.y = it->second.y + counter;
            test.calculateZ(data.p_mapData);
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKeyXYZ key_xyz;
                PathKeyXY key_xy;
                key_xyz.x = key_xy.x = test.x;
                key_xyz.y = key_xy.y = test.y;
                key_xyz.recursion = test.tile +speedup;
                if(open_set.find(key_xyz) == open_set.end() and closed_set.find(key_xy) == closed_set.end()){
                    open_set[key_xyz] = test;
                }
                if(key_xyz.x == key_xyz.dest_x and key_xyz.y == key_xyz.dest_y){
                    //open_set.clear();
                    cout << "SUCESS\n";
                    timestamp_t t1 = get_timestamp();
                    cout << "CalculatePath::Process took " << (long)(t1 - t0) / 1000000.0L << " seconds.\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile +speedup);
        }

        //bottom side
        counter = 0;
        while(counter < size){
            test.x = it->second.x + counter;
            test.y = it->second.y -1;
            test.calculateZ(data.p_mapData);
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKeyXYZ key_xyz;
                PathKeyXY key_xy;
                key_xyz.x = key_xy.x = test.x;
                key_xyz.y = key_xy.y = test.y;
                key_xyz.recursion = test.tile +speedup;
                if(open_set.find(key_xyz) == open_set.end() and closed_set.find(key_xy) == closed_set.end()){
                    open_set[key_xyz] = test;
                }
                if(key_xyz.x == key_xyz.dest_x and key_xyz.y == key_xyz.dest_y){
                    //open_set.clear();
                    cout << "SUCESS\n";
                    timestamp_t t1 = get_timestamp();
                    cout << "CalculatePath::Process took " << (long)(t1 - t0) / 1000000.0L << " seconds.\n";
                    return 1;
                }
            }
            counter += pow(2, test.tile +speedup);
        }

        //top side
        counter = 0;
        while(counter < size){
            test.x = it->second.x + counter;
            test.y = it->second.y + size +1;
            test.calculateZ(data.p_mapData);
            test.calculateTile(data.p_mapData);
            if(test.tile > MIN_RECURSION){
                test.limitRecursion(test.tile);
                PathKeyXYZ key_xyz;
                PathKeyXY key_xy;
                key_xyz.x = key_xy.x = test.x;
                key_xyz.y = key_xy.y = test.y;
                key_xyz.recursion = test.tile  +speedup;
                if(open_set.find(key_xyz) == open_set.end() and closed_set.find(key_xy) == closed_set.end()){
                    open_set[key_xyz] = test;
                }
                if(key_xyz.x == key_xyz.dest_x and key_xyz.y == key_xyz.dest_y){
                    //open_set.clear();
                    cout << "SUCESS\n";
                    timestamp_t t1 = get_timestamp();
                    cout << "CalculatePath::Process took " << (long)(t1 - t0) / 1000000.0L << " seconds.\n";

                    return 1;
                }
            }
            counter += pow(2, test.tile +speedup);
        }


        PathKeyXY key_xy;
        key_xy.x = it->first.x;
        key_xy.y = it->first.y;
        pair<int,int> p;
        p.first = ++steps;
        p.second = it->first.recursion;
        closed_set[key_xy] = p;
        open_set.erase(it);

    }
    cout << "FAIL\n";
    return 0;
}

bool CalculatePath::Simplify(timestamp_t end_time){
    //int lowest = std::numeric_limits<int>::max();
    timestamp_t t0 = get_timestamp();

    path.clear();

    //MapPoint point, last;
    //int last_size, point_size;
    MapPoint last;

    last.x = PathKeyXY::dest_x;
    last.y = PathKeyXY::dest_y;
    last.calculateZ(data.p_mapData);
    last.calculateTile(data.p_mapData);
    //last_size = pow(2, last.tile);
    

    PathKeyXY best;
    int best_tile;
    int best_so_far = std::numeric_limits<int>::max();
    bool updated = 1;

    while(get_timestamp() < end_time and closed_set.size() and best_so_far > 1){
        //cout << "\t" << path.size() << "\t" << best_so_far << "\t";

        updated = 0;


            int rescue_dist = std::numeric_limits<int>::max();
            int tmp_best_so_far = best_so_far;
            for(map<PathKeyXY, pair<int,int>>::iterator it = closed_set.begin(); it != closed_set.end(); it++){
                if(it->second.first < best_so_far){
                    if(abs((int)it->first.x - (int)last.x) + abs((int)it->first.y - (int)last.y) < rescue_dist){
                        //cout << ".";
                        rescue_dist = abs((int)it->first.x - (int)last.x) + abs((int)it->first.y - (int)last.y);
                        best.x = it->first.x;
                        best.y = it->first.y;
                        best_tile = it->second.second;
                        tmp_best_so_far = it->second.first;
                        updated = 1;
                    }
                }
            }
            best_so_far = tmp_best_so_far;

        //cout << "\n" << flush;

        if(updated){
            last.x = best.x;
            last.y = best.y;
            //last.calculateZ(data.p_mapData);
            //last.calculateTile(data.p_mapData);
            last.tile = best_tile;

            path.push_back(best);
        }

    }

    timestamp_t t1 = get_timestamp();
    cout << "CalculatePath::Simplify took " << (long)(t1 - t0) / 1000000.0L << " seconds.\n";

    return 1;
}

void CalculatePath::Display(void* p_map_view){
    cout << "open_set.size(): " << open_set.size() << "\n";
    cout << "closed_set.size(): " << closed_set.size() << "\n";
    cout << "path.size(): " << path.size() << "\n";

    ((Map*)p_map_view)->ClearIcons();
    int counter = 3;
    for(map<PathKeyXYZ, MapPoint>::iterator it = open_set.begin(); it != open_set.end(); it++){
        MapPoint point;
        point.x = it->first.x;
        point.y = it->first.y;
        point.tile = it->first.recursion;

        Icon icon = ((Map*)p_map_view)->IconSquare(64);
        icon.pos_x = point.x;
        icon.pos_y = point.y;
        icon.scale = pow(2, point.tile);
        Icon_key key ;
        key.type = ICON_TYPE_TEST;
        key.key = ++counter;
        ((Map*)p_map_view)->AddIcon(key, icon);
    }

    for(map<PathKeyXY, pair<int,int>>::iterator it = closed_set.begin(); it != closed_set.end(); it++){
        MapPoint point;
        point.x = it->first.x;
        point.y = it->first.y;
        //point.calculateZ(data.p_mapData);
        //point.calculateTile(data.p_mapData);
        point.tile = it->second.second;

        Icon icon = ((Map*)p_map_view)->IconSquare(128);
        icon.pos_x = point.x;
        icon.pos_y = point.y;
        icon.scale = pow(2, point.tile);
        Icon_key key ;
        key.type = ICON_TYPE_TEST;
        key.key = ++counter;
        ((Map*)p_map_view)->AddIcon(key, icon);
    }

    for(vector<PathKeyXY>::iterator it = path.begin(); it != path.end(); it++){
        if(it->x >= 0 and it->y >= 0){
            MapPoint point;
            point.x = it->x;
            point.y = it->y;
            point.calculateZ(data.p_mapData);
            point.calculateTile(data.p_mapData);

            Icon icon = ((Map*)p_map_view)->IconSquare(255);
            icon.pos_x = point.x;
            icon.pos_y = point.y;
            icon.scale = pow(2, point.tile);
            if(icon.scale < 1)
                icon.scale = 1;
            Icon_key key ;
            key.type = ICON_TYPE_TEST;
            key.key = ++counter;
            ((Map*)p_map_view)->AddIcon(key, icon);
        }
    }
}





