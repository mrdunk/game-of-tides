#ifndef DATA_H
#define DATA_H

#include <cmath>
#include <unordered_map>
#include <mutex>

#define MAX_RECURSION 24
#define MIN_RECURSION 4
#define MAX_SIZE (int)pow(2, MAX_RECURSION)
#define OFF_MAP_HEIGHT MAX_SIZE/2
#define SEED_MAP_HEIGHT OFF_MAP_HEIGHT
#define GAME_NUMBER 1234566
#define SEED .15
#define LANDMASS .2
#define MAX_DATA_SIZE 1000000

extern std::mutex g_mapData_lock;

/* http://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
 *
 * Args: size:      determines the range of the returned number.
 *      salt:       should be unique for each map.
 *      use:        should be unique for each place in the code it exists.
 *      x,y:        the x and y coordinates that are getting hashed.*/
inline short int jitter(unsigned int size, int salt, int use, unsigned int x,  unsigned int y);

struct Coordinate{
    unsigned int x;
    unsigned int y;

    std::string toString();
    void stringToInt(std::string key);

    /* getParent - Finds the parent node of coord. 
     * Returns: (bool) true = center of square.
     *                 false = center of diamond.
     *                 . f .
     *                 f t f
     *                 . f .
     */
    bool getParent(Coordinate* parent_coord);
    bool getParent(unsigned int* previousX, unsigned int* previousY);

    /* limitRecursion(void) - Returns the deepest recursion level Coordinate 
     * can exist at without rounding. */
    int limitRecursion(void);

    int limitRecursion(int recursionTarget);

    /* Limit the maximum recursion level to decend to.
     * This has the effect of rounding "limitedCoord" to the coordinates of the parent 
     * at level "recursionTarget".
     *
     * Returns: recursion level decended to. */
    int limitRecursion(int recursionTarget, Coordinate* limitedCoord);
    int limitRecursion(int recursionTarget, unsigned int* modifiedX, unsigned int* modifiedY);
};

struct MapPoint : public Coordinate{
    unsigned int z;
    int tile;
    static unsigned int counter;

    /* Will allow culling of seldom used nodes. */
    unsigned int last_accesed;

    unsigned int tide[16];

    /*
    */
    void calculateZ(void);
    void calculateZ(std::unordered_map<std::string, MapPoint>* mapData);

    /* This needs to be set for all MapPoint s once we know it. */
    static unsigned int waterlevel;

    /* Calculate the largest recursion level this point could cover without any possibility
     * of it overlapping the shore line.
     * Wet tiles are all wet. Dry tiles are all dry. */
    int calculateTile(void);
    int calculateTile(std::unordered_map<std::string, MapPoint>* mapData);

    /* Save the MapPoint to map. */
    void save(std::string key, std::unordered_map<std::string, MapPoint>* mapData);
};

class Data{
    private:
        static unsigned int height_z_max;
        static unsigned int height_z_min;
        static unsigned int waterlevel;
        std::string _key;
    public:
        Data(void);
        static std::unordered_map<std::string, MapPoint> mapData;
        static std::unordered_map<std::string, MapPoint>* p_mapData;
        unsigned int Height_z_min(void){return height_z_min;};
        unsigned int Height_z_max(void){return height_z_max;};
        unsigned int Waterlevel(void){return waterlevel;};

        static float wind_speed;
        static float wind_dir;

        /* Remove old entries from mapData. */
        void Cull(unsigned int time_left);
};


#endif // DATA_H
