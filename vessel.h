#ifndef VESSEL_H
#define VESSEL_H

#include <string>
#include <vector>
#include <GL/gl.h>

#include "data.h"
#include "viewport.h"

#define VESSEL_SIZE_MULTIPLIER (int)pow(2, MAX_RECURSION - MIN_RECURSION)
#define AIR_DENSITY 1.2
#define WATER_DENSITY 1000

#define MAST_RATIO              60

#define SAIL_TYPE_FORE          1
#define SAIL_TYPE_AFT           2
#define SAIL_TYPE_SQUARE        3
#define SAIL_TYPE_TOP           4
#define SAIL_TYPE_GENOA         5

#define SPAR_TYPE_MAST          1
#define SPAR_TYPE_BOWSPRIT      2
#define SPAR_TYPE_OAR_PORT      3
#define SPAR_TYPE_OAR_STARBOARD 4
#define SPAR_TYPE_LEEBOARD      5

#define VESSEL_TYPE_BOAT        1
#define VESSEL_TYPE_RAFT        2

#define VESSEL_STATE_ANCHORED   1
#define VESSEL_STATE_ADRIFT     2
#define VESSEL_STATE_MAKING_WAY 3


struct Sail {
    std::string description;        //
    unsigned int type;              // See "#define SAIL_TYPE_XXX".
    unsigned int tack_height;       // Height of sail's foot above deck.
    unsigned int tack_position;     // SAIL_TYPE_FORE only: How far the tack is from mast or front of bowsprit.
    unsigned int height;            // Height of sail.
    unsigned int foot;              // Length of sail's foot
    float reef_size;                // Size of reef points (ratio of whole sail). A sail that cannot be reefed will be 100%.
    float deployed;                 // How much of the sail is hoisted (ratio of whole sail). (Must be a multiple of reef_size.)
    float sheeted;                  // Angle the sail is sheeted to.
    float min_sheeted;              // Furthest out sail can be sheeted. (Typically 90deg for a mainsail and no limit for a foresail.)
    float max_sheeted;              // Furthest in sail can be sheeted. (Typically no limit for a mainsail and depends on rigging for a foresail.)

};

struct Spar {
    std::string description;        //
    unsigned int type;              // See "#define SPAR_TYPE_XXX".
    unsigned int position;          // Position os Spar from front of boat.
    unsigned int length;            // Length of spar.
    std::vector<Sail> sails;        // Contains Sail instances.
};

struct Vessel {
    std::string description;        //
    unsigned int type;              // See "#define VESSEL_TYPE_XXX".
    unsigned int length;            // Length of vessel.
    unsigned int beam;              // Beam (width).
    unsigned int draft;             // Draft (depth).
    unsigned int displacment;       // Volume of water displaced
    unsigned int cod;               // Center Of Drag. (pivot point of boat.)
    std::vector<Spar> spars;        // Contains Spar instances.

    unsigned int state;             // See "#define VESSEL_STATE_XXX"
    double pos_x;                      // Position on map.
    double pos_y;
    float desired_heading;          // Rudder is steering for this direction. (in degrees)
    float heading;                  // Vessel is actuallly pointing in this direction.
    float desired_speed;            
    float speed;                    // Current speed of vessel.
    float leeway_speed;             // Speed perpendicular to the keel. (ie. sideways.)
    float apparent_wind_dir;        // Wind direction relitive to vessel (taking vessel speed over groind into account).
    float apparent_wind_speed;      // Wind speed relitive to vessel (taking vessel speed over groind into account).
    unsigned int last_updated;      // Time when vessel's current speed and heading were last calculated.

    /* Update current speed and heading of the vessel. */
    void Calculate(float wind_speed, float wind_dir);

    /* Add the vessel's Icon. */
    Icon PopulateIcon(int x0, int y0, int x1, int y1);
};

class Fleet {
    private:
        static std::vector<Vessel> vessels;    // Contains all instances of Vessel.
        unsigned int _vessel_itterater;
    public:
        Fleet(void);

        /* Update current speed and heading of all vessels. */
        void CalculateVessels(float wind_speed, float wind_dir);

        /* Itterate through vessels Icons. */
        Icon NextIcon(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);

        /* Return to 1st vessels Icon. */
        void StartIcon(void);
};

#endif // VESSEL_H
