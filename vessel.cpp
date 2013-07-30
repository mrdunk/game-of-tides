#include <iostream>

#include "vessel.h"

using namespace std;

void Vessel::Calculate(void){
}

Icon Vessel::PopulateIcon(int x0, int y0, int x1, int y1){
    Icon icon;
    
    if(!(pos_x >= x0 and pos_x <= x1 and pos_y >= y0 and pos_y <= y1)){
        // Vessel not in view area.
        cout << "Vessel \"" << description << "\" not in view area.\n";
        icon.scale = 0;
        return icon;
    }
    cout << "Vessel \"" << description << "\"\n";

    icon.angle = heading;
    icon.scale = (float)16/100;     // 16 == pow(2, MIN_RECURSION), 100 == all vessel dimensions are specified in cm.
    //icon.scale = 1000;
    icon.fixed_size = 0;
    icon.centre_x = 0;
    icon.centre_y = 0;
    icon.pos_x = MAX_SIZE /2;
    icon.pos_y = MAX_SIZE /2;
    
    /*
    icon.points.push_back (0);
    icon.points.push_back (1000);
    icon.points.push_back (100);
    icon.points.push_back (0);
    icon.points.push_back (-100);
    icon.points.push_back (0);

    icon.colour.push_back (255);
    icon.colour.push_back (255);
    icon.colour.push_back (0);
    icon.colour.push_back (255);
    icon.colour.push_back (255);
    icon.colour.push_back (0);
    icon.colour.push_back (255);
    icon.colour.push_back (255);
    icon.colour.push_back (0);

    return icon;*/

    int bowsprit = 0;
    int size = length;

    icon.points.push_back(0);
    icon.points.push_back((size/2) - bowsprit);
    icon.points.push_back(-(int)beam/2);
    icon.points.push_back((size/2) - bowsprit - (length/3));
    icon.points.push_back(beam/2);
    icon.points.push_back((size/2) - bowsprit - (length/3));

    icon.points.push_back( -(int)beam/2);
    icon.points.push_back((size/2) - bowsprit - (length/3));
    icon.points.push_back( -(int)beam/2);
    icon.points.push_back((size/2) - bowsprit - (2*length/3));
    icon.points.push_back(beam/2);
    icon.points.push_back((size/2) - bowsprit - (length/3));


    icon.points.push_back(-(int)beam/2);
    icon.points.push_back((size/2) - bowsprit - (2*length/3));
    icon.points.push_back(beam/2);
    icon.points.push_back((size/2) - bowsprit - (2*length/3));
    icon.points.push_back(beam/2);
    icon.points.push_back((size/2) - bowsprit - (length/3));


    icon.points.push_back(-(int)beam/2);
    icon.points.push_back((size/2) - bowsprit - (2*length/3));
    icon.points.push_back(-(int)beam/4);
    icon.points.push_back(-size/2);
    icon.points.push_back(beam/2);
    icon.points.push_back((size/2) - bowsprit - (2*length/3));

    icon.points.push_back(beam/2);
    icon.points.push_back((size/2) - bowsprit - (2*length/3));
    icon.points.push_back(-(int)beam/4);
    icon.points.push_back(-size/2);
    icon.points.push_back(beam/4);
    icon.points.push_back(-size/2);

    for(int i = 0; i < 15 * 3; i++){
        icon.colour.push_back(128);
    }
    return icon;

}

std::vector<Vessel> Fleet::vessels;

Fleet::Fleet(void){
    StartIcon();

    Vessel v0;
    Spar v0_sp0, v0_sp1;
    Sail v0_sp1_sa0, v0_sp1_sa1;

    v0.description  = "Test skiff";
    v0.type         = VESSEL_TYPE_BOAT;
    v0.length       = 800;
    v0.beam         = 200;
    v0.draft        = 150;
    v0.displacment  = 800*200*150/4;
    v0.cod          = 400;

    
    v0_sp0.description  = "";
    v0_sp0.type         = SPAR_TYPE_BOWSPRIT;
    v0_sp0.position     = 0;
    v0_sp0.length       = 100;
    
    v0_sp1.description  = "";
    v0_sp0.type         = SPAR_TYPE_MAST;
    v0_sp0.position     = 300;
    v0_sp0.length       = 850;

    v0_sp1_sa0.description      = "";
    v0_sp1_sa0.type             = SAIL_TYPE_FORE;
    v0_sp1_sa0.tack_height      = 50;
    v0_sp1_sa1.tack_position    = 300;
    v0_sp1_sa0.height           = 800;
    v0_sp1_sa0.foot             = 350;
    v0_sp1_sa0.reef_size        = 0.2f;
    v0_sp1_sa0.deployed         = 1.0f;
    v0_sp1_sa0.sheeted          = 0;

    v0_sp1_sa1.description      = "";
    v0_sp1_sa1.type             = SAIL_TYPE_AFT;
    v0_sp1_sa1.tack_height      = 50;
    v0_sp1_sa1.tack_position    = 0;
    v0_sp1_sa1.height           = 800;
    v0_sp1_sa1.foot             = 400;
    v0_sp1_sa1.reef_size        = 0.2f;
    v0_sp1_sa1.deployed         = 1.0f;
    v0_sp1_sa1.sheeted          = 0;

    v0_sp1.sails.push_back(v0_sp1_sa0);
    v0_sp1.sails.push_back(v0_sp1_sa1);

    v0.spars.push_back(v0_sp0);
    v0.spars.push_back(v0_sp1);

    v0.state                    = VESSEL_STATE_MAKING_WAY;
    v0.pos_x                    = MAX_SIZE / 2;
    v0.pos_y                    = MAX_SIZE / 2;
    v0.desired_heading          = 0.0f;
    v0.heading                  = 0.0f;
    v0.desired_speed            = 3.5f;
    v0.speed                    = 3.5f;
    v0.apparent_wind_dir        = 0.25f;
    v0.apparent_wind_speed      = 10.0f;
    v0.last_updated             = 0;

    vessels.push_back(v0);
}

Icon Fleet::NextIcon(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1){
    Icon icon;
    icon.scale = 0;
    icon.key = 0;
    while(icon.scale == 0){
        if(_vessel_itterater >= vessels.size())
            break;
        icon = vessels.at(_vessel_itterater).PopulateIcon(x0, y0, x1, y1);
        cout << " ** " << icon.points.size() << "\n";
        icon.key = ++_vessel_itterater;
    }
    return icon;
}

void Fleet::StartIcon(void){
    _vessel_itterater = 0;
}


