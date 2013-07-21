#include <sstream>
#include <string>
#include <iostream>

#include "data.h"

using namespace std;

//std::unordered_map<std::string, MapPoint> mapData;

std::string Coordinate::toString(){
    std::stringstream ss;
    ss << x << "," << y;
    return ss.str();
}

void Coordinate::stringToInt(std::string key){
    std::stringstream ss(key);
    std::string sx, sy;
    std::getline(ss, sx, ',');
    std::getline(ss, sy, ',');

    x = std::stoi(sx);
    y = std::stoi(sy);
}

bool Coordinate::getParent(Coordinate* parent_coord){
    return getParent(&(parent_coord->x), &(parent_coord->y));
}

bool Coordinate::getParent(unsigned int* previousX, unsigned int* previousY){
    /* Returns: (bool) true = center of square.
                       false = center of diamond.
                       . f .
                       f t f
                       . f .
    */
    int remainderX = x;
    int remainderY = y;
    int stepSize = MAX_SIZE;
    int recursion = MAX_RECURSION;
    int recurseX = -1;
    int recurseY = -1;

    if (remainderX == 0){
        recurseX = recursion;
    }
    if (remainderY == 0){
        recurseY = recursion;
    }
    while ((recurseX < 0) or (recurseY < 0)){
        *previousX = x - remainderX;
        *previousY = y - remainderY;

        if(stepSize <= remainderX){
            remainderX -= stepSize;
            if ((recurseX < 0) and (remainderX == 0)){
                recurseX = recursion;
            }
        }
        if(stepSize <= remainderY){
            remainderY -= stepSize;
            if ((recurseY < 0) and (remainderY == 0)){
                recurseY = recursion;
            }
        }
        stepSize /= 2;
        recursion -= 1;
    }

    return (recurseX == recurseY);
}

int Coordinate::limitRecursion(void){
    Coordinate tmp;
    return limitRecursion(0, &tmp);
}

int Coordinate::limitRecursion(int recursionTarget, Coordinate* limitedCoord){
    return limitRecursion(recursionTarget, &limitedCoord->x, &limitedCoord->y);
}

int Coordinate::limitRecursion(int recursionTarget, unsigned int* modifiedX, unsigned int* modifiedY){
    /* recursionTarget == 0 to query what level the point is at.
     */
    *modifiedX = 0;
    *modifiedY = 0;
    int stepSize = MAX_SIZE;
    int recursion = MAX_RECURSION;
    int remainderX = x;
    int remainderY = y;
    while((recursion >= recursionTarget) and (remainderX > 0 or remainderY > 0)){
        if(stepSize <= remainderX){
            remainderX -= stepSize;
            *modifiedX += stepSize;
        }
        if(stepSize <= remainderY){
            remainderY -= stepSize;
            *modifiedY += stepSize;
        }
        recursion --;
        stepSize /= 2;

    }
    return recursion;
}

//unsigned int MapPoint::_waterlevel;
unsigned int MapPoint::counter = 0;

void MapPoint::calculateZ(std::unordered_map<std::string, MapPoint>* mapData){
    //cout << "MapPoint::calculateZ + " << x << "," << y << "\n";

    std::string key = toString();
    std::unordered_map<std::string, MapPoint>::const_iterator got = mapData->find (key);

    if ( got != mapData->end() ){
        // already have the data in buffer.
        z = got->second.z;
        
        last_accesed = ++counter;
        std::pair<std::string, MapPoint> entry(key, *this);
        mapData->insert(entry);

        return;
    }
   
    if(!x and !y){
        // 0,0 point.
        z = SEED_MAP_HEIGHT;
        //cout << "MapPoint::calculateZ r\n";
        return;
    }
    if(x < 0 or y < 0 or x >= (unsigned)MAX_SIZE or y >= (unsigned)MAX_SIZE){
        z = OFF_MAP_HEIGHT;
        //cout << "MapPoint::calculateZ r2\n";
        return;
    }
 
    MapPoint parent;
    bool squareOrDiamond = getParent(&parent);
    
    int pixSize;
    if(x != parent.x)
        pixSize = abs((int)x - (int)parent.x);
    else
        pixSize = abs((int)y - (int)parent.y);


    if(squareOrDiamond){
        // center of square.
        MapPoint tl;
        tl.x = x - pixSize;
        tl.y = y - pixSize;
        tl.calculateZ(mapData);

        MapPoint tr;
        tr.x = x + pixSize;
        tr.y = y - pixSize;
        tr.calculateZ(mapData);

        MapPoint bl;
        bl.x = x - pixSize;
        bl.y = y + pixSize;
        bl.calculateZ(mapData);

        MapPoint br;
        br.x = x + pixSize;
        br.y = y + pixSize;
        br.calculateZ(mapData);

        z = (tl.z + tr.z + bl.z + br.z) / 4;
        z += jitter(pixSize, GAME_NUMBER, 1, x, y);

    } else {
        // center of diamond.
        MapPoint t;
        t.x = x;
        t.y = y - pixSize;
        t.calculateZ(mapData);

        MapPoint l;
        l.x = x - pixSize;
        l.y = y;
        l.calculateZ(mapData);

        MapPoint r;
        r.x = x + pixSize;
        r.y = y;
        r.calculateZ(mapData);

        MapPoint b;
        b.x = x;
        b.y = y + pixSize;
        b.calculateZ(mapData);

        z = (t.z + l.z + r.z + b.z) / 4;
        z += jitter(pixSize, GAME_NUMBER, 1, x, y);

    }

    last_accesed = ++counter;
    std::pair<std::string, MapPoint> entry(key, *this);
    mapData->insert(entry);

    //cout << "MapPoint::calculateZ -\n";
}


#define PRIME 1099511628211
#define OFFSET 14695981039346656037u
/* http://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function */
inline short int jitter(unsigned int size, int salt, int use, unsigned int x,  unsigned int y){
    //std::cout << "Data::jitter\n";
    size *= SEED;

    if(!size)
        size = 1;
    unsigned long hash = OFFSET;
    hash ^= x;
    hash *= PRIME;
    hash ^= salt;
    hash *= PRIME;
    hash ^= use;
    hash *= PRIME;
    hash ^= y;
    hash *= PRIME;
    int out = (hash % size) - (size / 2);
    return out;
}

std::unordered_map<std::string, MapPoint> Data::mapData;
std::unordered_map<std::string, MapPoint>* Data::p_mapData;
unsigned int Data::waterlevel = 0;
unsigned int Data::height_z_min;
unsigned int Data::height_z_max;

Data::Data(void){
    p_mapData = &mapData;

    if(!waterlevel){
        MapPoint testcoord;
        height_z_max = 0;
        height_z_min = MAX_SIZE;

        for(int row = MAX_SIZE/128; row < MAX_SIZE; row+=(MAX_SIZE/128)){
            for(int col = MAX_SIZE/128; col < MAX_SIZE; col+=(MAX_SIZE/128)){
                testcoord.x = col;
                testcoord.y = row;
                testcoord.calculateZ(p_mapData);
                if(height_z_max < testcoord.z)
                    height_z_max = testcoord.z;
                if(height_z_min > testcoord.z)
                    height_z_min = testcoord.z;
            }
        }

        waterlevel = height_z_max - ((height_z_max - height_z_min) * LANDMASS);
        float landmass = 0;
        int INIT_RES = 128;
        while((int)(LANDMASS * 10) != (int)(landmass * 10)){
            cout << "waterleveldd: " << waterlevel << "\tlandmass: " << landmass << "\n";
            int drycount = 0;
            int totalcount = 0;
            for(int row = MAX_SIZE/INIT_RES; row < MAX_SIZE; row+=(MAX_SIZE/INIT_RES)){
                for(int col = MAX_SIZE/INIT_RES; col < MAX_SIZE; col+=(MAX_SIZE/INIT_RES)){
                    testcoord.x = col;
                    testcoord.y = row;
                    testcoord.calculateZ(p_mapData);
                    if(testcoord.z > waterlevel)
                        ++drycount;
                    ++totalcount;
                }
            }

            landmass = (float)drycount / totalcount;
            if(landmass > LANDMASS)
                waterlevel += 2000;
            else
                waterlevel -= 2000;
        }
    }
    cout << "Data::Data height_z_max: " << height_z_max << "\theight_z_min: " << height_z_min << "\twaterlevel: " << waterlevel << "\n";
    //testcoord.Set_waterlevel(waterlevel);
}
