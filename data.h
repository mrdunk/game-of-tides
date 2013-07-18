#ifndef DATA_H
#define DATA_H

#include <cmath>
#include <unordered_map>

#define MAX_RECURSION 24
#define MIN_RECURSION 4
#define MAX_SIZE (int)pow(2, MAX_RECURSION)
#define OFF_MAP_HEIGHT MAX_SIZE/2
#define SEED_MAP_HEIGHT OFF_MAP_HEIGHT
#define GAME_NUMBER 123456
#define SEED .1


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

    /* limitRecursion(void) - Returns the highest recursion level Coordinate 
     * can exist at without rounding. */
    int limitRecursion(void);

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
    unsigned int recursion;

    /*
    */
    void calculateZ(std::unordered_map<std::string, MapPoint>* mapData);
};


#endif // DATA_H
