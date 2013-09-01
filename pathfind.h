#ifndef PATHFIND_H
#define PATHFIND_H

#include <map>
#include <vector>
#include <unordered_map>
#include "time_code.c"

#include "map.h"
#include "data.h"

#define MIN_PATH_RECURSION 10
#define CHECKLINE_RESOLUTION 2

struct PathKeyXYZ{
    static int dest_x;
    static int dest_y;
    int recursion;
    int x;
    int y;

    bool operator<(const PathKeyXYZ pk) const;
};

struct PathKeyXY{
    static int dest_x;
    static int dest_y;
    int x;
    int y;

    bool operator<(const PathKeyXY pk) const;
};


class CalculatePath{
    private:
        static Data data;
        std::map<PathKeyXYZ, MapPoint> open_set;
        std::map<PathKeyXY, std::pair<int,int>> closed_set;
        std::vector<PathKeyXY> path;
        static std::vector<MapPoint> passages;
    public:
        CalculatePath(MapPoint start, MapPoint end, int min_depth);
        bool Process(timestamp_t end_time);
        bool Simplify(timestamp_t end_time);
        bool Straighten(timestamp_t end_time);
        bool CheckLine(PathKeyXY start, PathKeyXY end);
        bool CheckLine(MapPoint start, MapPoint end);
        void Display(void* p_map_view);
};

#endif  // PATHFIND_H
