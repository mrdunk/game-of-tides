#ifndef PATHFIND_H
#define PATHFIND_H

#include <map>
#include <unordered_map>
#include "time_code.c"

#include "map.h"
#include "data.h"

struct PathKey{
    static int dest_x;
    static int dest_y;
    int recursion;
    int x;
    int y;

    bool operator<(const PathKey pk) const;
};

class CalculatePath{
    private:
        static Data data;
        std::map<PathKey, MapPoint> open_set;
        std::map<PathKey, int> closed_set;
    public:
        CalculatePath(MapPoint start, MapPoint end, int min_depth);
        bool Process(timestamp_t end_time);
        void Display(void* p_map_view);
};

#endif  // PATHFIND_H
