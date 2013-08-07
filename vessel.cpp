#include <iostream>

#include "vessel.h"

#define PI 3.14157

using namespace std;

void Vessel::Calculate(float wind_speed, float wind_dir){
    cout << "Vessel::Calculate " << description << "\n";
    
    bool optimise_trim = true;

    double force_leway = 0;
    double force_forward = 0;
    double force_heel = 0;
    double force_rotation = 0;

    /* Calculate relative wind. */
    //float wind_dir_x = wind_speed * sin(PI * wind_dir / 180);
    //float wind_dir_y = wind_speed * cos(PI * wind_dir / 180);
    //float local_wind_dir_x = speed * sin(PI * heading / 180);
    //float local_wind_dir_y = speed * cos(PI * heading / 180);
    float relaive_wind = wind_dir - heading;
    if(relaive_wind > 180)
        relaive_wind -= 360;
    else if(relaive_wind <= -180)
        relaive_wind += 360;
    float wind_dir_x = wind_speed * sin(PI * relaive_wind / 180);
    float wind_dir_y = wind_speed * cos(PI * relaive_wind / 180);
    float local_wind_dir_x = 0;
    float local_wind_dir_y = speed;

    apparent_wind_speed = sqrt((wind_dir_x*wind_dir_x) + ((wind_dir_y + speed)*(wind_dir_y + speed)));
    apparent_wind_dir = atan2((wind_dir_x + local_wind_dir_x), (wind_dir_y + local_wind_dir_y)) * 180 / PI;
    if(apparent_wind_dir > 180)
        apparent_wind_dir -= 360;
    else if(apparent_wind_dir <= -180)
        apparent_wind_dir += 360;

    /* Calculate forces on Spars and Sails. */

    // http://www.ivorbittle.co.uk/Books/The%20physics%20of%20sailing%20version%202/Section%202%20The%20single%20soft%20sail.htm
    // http://en.wikipedia.org/wiki/Forces_on_sails#Sailing_in_attached_flow

    for(std::vector<Spar>::iterator it_spar = spars.begin() ; it_spar != spars.end(); ++it_spar){

        // coeficient_drag == 1.5 for mast and rigging.
        // if we presume a spar is 1% as wide as it is high and the rigging presents the same windage,
        // the area seen by the wind will be the spar length * spar length * 0.02
        float force_drag = 0.5 * AIR_DENSITY * ((float)it_spar->length * it_spar->length * 0.02) * 1.5 * apparent_wind_speed * apparent_wind_speed;
        float force_drag_x = force_drag * sin(PI * apparent_wind_dir / 180);
        float force_drag_y = force_drag * cos(PI * apparent_wind_dir / 180);

        force_leway += -force_drag_x;
        force_forward += -force_drag_y;

        if(it_spar->type == SPAR_TYPE_BOWSPRIT){
            force_rotation += force_drag_x * (cod + it_spar->length /2);
        }
        else if(it_spar->type == SPAR_TYPE_MAST){
            force_rotation += force_drag_x * (cod - it_spar->position);
            force_heel += force_drag_x * it_spar->length /2;
        }
        for(std::vector<Sail>::iterator it_sail = it_spar->sails.begin() ; it_sail != it_spar->sails.end(); ++it_sail){
            int sail_area = (float)it_sail->height * it_sail->foot * it_sail->deployed / 2;

            /* Front of sail's position on the boat. */
            int tack_on_boat;
            int centre_of_effort;
            if(it_spar->type == SPAR_TYPE_BOWSPRIT and it_sail->type == SAIL_TYPE_FORE){
                tack_on_boat = it_sail->tack_position;
                centre_of_effort = tack_on_boat + (it_sail->foot * it_sail->deployed * 2/3);
            } else if(it_spar->type == SPAR_TYPE_MAST and it_sail->type == SAIL_TYPE_FORE){
                tack_on_boat = (int)it_spar->position - (int)it_sail->tack_position;
                centre_of_effort = tack_on_boat + (it_sail->foot * it_sail->deployed * 2/3);
            } else if(it_spar->type == SPAR_TYPE_MAST and it_sail->type == SAIL_TYPE_AFT){
                tack_on_boat = it_spar->position;
                centre_of_effort = tack_on_boat + (it_sail->foot * it_sail->deployed /3);
            }

            /* The optimum aoa to apparent wind is ~0.5 * angle of course to apparent wind. */
            float aoa;
            if(optimise_trim)
                it_sail->sheeted = (float)apparent_wind_dir / 2;

            if(apparent_wind_dir >= 0){
                aoa = it_sail->sheeted;
                if(apparent_wind_dir < aoa){
                    aoa = apparent_wind_dir;
                    if(aoa < it_sail->max_sheeted){
                        aoa = it_sail->max_sheeted;
                    }
                }
            } else {
                aoa = it_sail->sheeted;
                if(apparent_wind_dir > aoa){
                    aoa = apparent_wind_dir;
                    if(aoa > it_sail->max_sheeted){
                        aoa = it_sail->max_sheeted;
                    }
                }
            }
            cout << "aoa: \t\t" << aoa << "\n";

            float coeficient_lift = 1.3 * sin(2 * aoa * PI / 180);
            float coeficient_drag;
            if(aoa > 0){
                coeficient_drag = .9 * cos((2.0f * (aoa +5) + 180) * PI / 180) + .9;
            } else {
                coeficient_drag = .9 * cos((2.0f * (aoa -5) - 180) * PI / 180) + .9;
            }

            float force_lift = 0.5f * AIR_DENSITY * sail_area * coeficient_lift * apparent_wind_speed * apparent_wind_speed;
            force_drag = 0.5f * AIR_DENSITY * sail_area * coeficient_drag * apparent_wind_speed * apparent_wind_speed;

            float force_lift_x = force_lift * cos(PI * apparent_wind_dir / 180);
            float force_lift_y = force_lift * sin(PI * apparent_wind_dir / 180);
            force_drag_x = force_drag * sin(PI * apparent_wind_dir / 180);
            force_drag_y = force_drag * cos(PI * apparent_wind_dir / 180);
cout << "~ " << coeficient_lift << " " << coeficient_drag << "\n";
cout << ". " << (int)force_lift << " " << (int)force_drag << "\n";
cout << "* " << (int)force_lift_x << " " << (int)force_drag_x << "\n";
cout << "& " << (int)force_lift_y << " " << (int)force_drag_y << "\n";
            force_leway += -force_lift_x -force_drag_x;
            force_forward += force_lift_y - force_drag_y;

            force_rotation += (force_lift_x + force_drag_x) * (cod - centre_of_effort);
            force_heel += (force_lift_x + force_drag_x) * ((it_sail->height * it_sail->deployed) /3 + it_sail->tack_height);

        }
    }

    int timeslice = 10;


    // Apply forward_force.
    // force = mass * acceleration.
    // Acceleration is the change in speed in a set time.
    // Also the force due to drag is prportinal to the speed.
    float drag_coeficient_face = .1;
    float drag_coeficient_side = 2;
    int boat_area_face = beam * draft / 2;
    int boat_area_side = length * draft / 2;
    int resistive_force = 0.5 * WATER_DENSITY * boat_area_face * drag_coeficient_face * speed * std::abs(speed);
    int leeway_resistive_force = 0.5 * WATER_DENSITY * boat_area_side * drag_coeficient_side * leeway_speed * std::abs(leeway_speed);

    force_forward -= resistive_force;
    cout << (int)force_leway << "\n" << (int)leeway_resistive_force << "\n";
    force_leway -= leeway_resistive_force;

    speed += ((float)force_forward / displacment) / timeslice;
    leeway_speed += ((float)force_leway / displacment) / timeslice;


    /* 1 knot = 0.514444444 m/s. We round to 0.5m/s.
     * pow(2, MIN_RESOLUTION) == 16                   */
    // forwards
    pos_x += (float)sin(PI * heading / 180) * (float)speed * 16 * 0.5 / timeslice;
    pos_y += (float)cos(PI * heading / 180) * (float)speed * 16 * 0.5 / timeslice;

    //leeway
    pos_x += (float)cos(PI * heading / 180) * (float)leeway_speed * 16 * 0.5 / timeslice;
    pos_y -= (float)sin(PI * heading / 180) * (float)leeway_speed * 16 * 0.5 / timeslice;

    cout << "force_forward:\t\t" << (int)force_forward << "\n";
    cout << "force_leway: \t\t" << (int)force_leway << "\n";
    cout << "speed: \t\t\t" << speed << "\n";
    cout << "leeway_speed: \t\t" << leeway_speed << "\n";
    cout << "wind_dir:\t\t" << wind_dir << "\n";
    cout << "apparent_wind_dir:\t" << apparent_wind_dir << "\n";
    cout << "apparent_wind_speed:\t" << apparent_wind_speed << "\n";
}

Icon Vessel::PopulateIcon(int x0, int y0, int x1, int y1){
    Icon icon;
    
    if(!(pos_x >= x0 and pos_x <= x1 and pos_y >= y0 and pos_y <= y1)){
        // Vessel not in view area.
        cout << "Vessel \"" << description << "\" not in view area.\n";
        icon.scale = 0;
        return icon;
    }
    cout << "Vessel::PopulateIcon " << description << "\n";

    icon.angle = heading;
    icon.scale = (float)16/100;     // 16 == pow(2, MIN_RECURSION), 100 == all vessel dimensions are specified in cm.
    //icon.scale = 1000;
    icon.fixed_size = 0;
    icon.centre_x = 0;
    icon.centre_y = 0;
    icon.pos_x = pos_x;
    icon.pos_y = pos_y;
    
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

    for(std::vector<Spar>::iterator it = spars.begin() ; it != spars.end(); ++it){
        if(it->type == SPAR_TYPE_BOWSPRIT){
            bowsprit = it->length / 2;
        }
    }

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

    for(std::vector<Spar>::iterator it_spar = spars.begin() ; it_spar != spars.end(); ++it_spar){
        if(it_spar->type == SPAR_TYPE_BOWSPRIT){
            icon.points.push_back(-(int)it_spar->length/15);
            icon.points.push_back((size/2) + bowsprit);
            icon.points.push_back(-(int)it_spar->length/10);
            icon.points.push_back((size/2) - bowsprit - length/20);
            icon.points.push_back(it_spar->length/10);
            icon.points.push_back((size/2) - bowsprit - length/20);

            icon.points.push_back(-(int)it_spar->length/15);
            icon.points.push_back((size/2) + bowsprit);
            icon.points.push_back(it_spar->length/10);
            icon.points.push_back((size/2) - bowsprit - length/20);
            icon.points.push_back(it_spar->length/15);
            icon.points.push_back((size/2) + bowsprit);

            for(int i = 0; i < 6; i++){
                icon.colour.push_back(50);
                icon.colour.push_back(20);
                icon.colour.push_back(20);
            }
        }
        else if(it_spar->type == SPAR_TYPE_MAST){
            icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
            icon.points.push_back((size/2) - bowsprit - it_spar->position + it_spar->length/MAST_RATIO);
            icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
            icon.points.push_back((size/2) - bowsprit - it_spar->position - it_spar->length/MAST_RATIO);
            icon.points.push_back(it_spar->length/MAST_RATIO);
            icon.points.push_back((size/2) - bowsprit - it_spar->position + it_spar->length/MAST_RATIO);

            icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
            icon.points.push_back((size/2) - bowsprit - it_spar->position - it_spar->length/MAST_RATIO);
            icon.points.push_back(it_spar->length/MAST_RATIO);
            icon.points.push_back((size/2) - bowsprit - it_spar->position - it_spar->length/MAST_RATIO);
            icon.points.push_back(it_spar->length/MAST_RATIO);
            icon.points.push_back((size/2) - bowsprit - it_spar->position + it_spar->length/MAST_RATIO);

            for(int i = 0; i < 6; i++){
                icon.colour.push_back(50);
                icon.colour.push_back(20);
                icon.colour.push_back(20);
            }
        }

        for(std::vector<Sail>::iterator it_sail = it_spar->sails.begin() ; it_sail != it_spar->sails.end(); ++it_sail){
            int tack_on_boat;
            float aoa;          //Angle Of Atack
            if(it_spar->type == SPAR_TYPE_BOWSPRIT and it_sail->type == SAIL_TYPE_FORE){
                tack_on_boat = it_sail->tack_position;
            } else if(it_spar->type == SPAR_TYPE_MAST and it_sail->type == SAIL_TYPE_FORE){
                tack_on_boat = (int)it_spar->position - (int)it_sail->tack_position;
            } else if(it_spar->type == SPAR_TYPE_MAST and it_sail->type == SAIL_TYPE_AFT){
                tack_on_boat = it_spar->position;
                tack_on_boat += it_spar->length/MAST_RATIO;     // Thickness of mast.
            }
            if(apparent_wind_dir >= 0){
                aoa = it_sail->sheeted;
                if(apparent_wind_dir < aoa){
                    aoa = apparent_wind_dir;
                    if(aoa < it_sail->max_sheeted){
                        aoa = it_sail->max_sheeted;
                    }
                }
            } else {
                aoa = it_sail->sheeted;
                if(apparent_wind_dir > aoa){
                    aoa = apparent_wind_dir;
                    if(aoa > it_sail->max_sheeted){
                        aoa = it_sail->max_sheeted;
                    }
                }
            }

            if(it_sail->type == SAIL_TYPE_AFT){
                icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * aoa / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)((float)it_sail->foot * cos(3.141 * aoa / 180)));
                icon.points.push_back((int)it_spar->length/MAST_RATIO);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);

                icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * aoa / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * aoa / 180));
/*
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3 - (int)it_sail->foot * sin(3.141 * aoa / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3  - (int)it_sail->foot * cos(3.141 * aoa / 180) / 3);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * aoa / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * aoa / 180));
*/
                for(int i = 0; i < 6; i++){
                    icon.colour.push_back(255);
                    icon.colour.push_back(255);
                    icon.colour.push_back(255);
                }
            } else if(it_sail->type == SAIL_TYPE_FORE){
                icon.points.push_back(0);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back(-(int)it_sail->foot * sin(3.141 * aoa / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)it_sail->foot * cos(3.141 * aoa / 180));
  
                //icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
                //icon.points.push_back((size/2) - bowsprit - tack_on_boat);


                for(int i = 0; i < 3; i++){
                    icon.colour.push_back(255);
                    icon.colour.push_back(255);
                    icon.colour.push_back(255);
                }
            }
        }
    }

    return icon;

}

std::vector<Vessel> Fleet::vessels;

Fleet::Fleet(void){
    if(!vessels.size()){
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
        v0_sp1.type         = SPAR_TYPE_MAST;
        v0_sp1.position     = 300;
        v0_sp1.length       = 850;

        v0_sp1_sa0.description      = "";
        v0_sp1_sa0.type             = SAIL_TYPE_FORE;
        v0_sp1_sa0.tack_height      = 50;
        v0_sp1_sa0.tack_position    = 300;
        v0_sp1_sa0.height           = 800;
        v0_sp1_sa0.foot             = 350;
        v0_sp1_sa0.reef_size        = 0.2f;
        v0_sp1_sa0.deployed         = 1.0f;
        v0_sp1_sa0.sheeted          = 30;
        v0_sp1_sa0.min_sheeted      = 180;
        v0_sp1_sa0.max_sheeted      = 20;

        v0_sp1_sa1.description      = "";
        v0_sp1_sa1.type             = SAIL_TYPE_AFT;
        v0_sp1_sa1.tack_height      = 50;
        v0_sp1_sa1.tack_position    = 0;
        v0_sp1_sa1.height           = 800;
        v0_sp1_sa1.foot             = 400;
        v0_sp1_sa1.reef_size        = 0.2f;
        v0_sp1_sa1.deployed         = 1.0f;
        v0_sp1_sa1.sheeted          = 30;
        v0_sp1_sa1.min_sheeted      = 90;
        v0_sp1_sa1.max_sheeted      = 0;


        v0_sp1.sails.push_back(v0_sp1_sa0);
        v0_sp1.sails.push_back(v0_sp1_sa1);

        v0.spars.push_back(v0_sp0);
        v0.spars.push_back(v0_sp1);

        v0.state                    = VESSEL_STATE_MAKING_WAY;
        v0.pos_x                    = MAX_SIZE / 2;
        v0.pos_y                    = MAX_SIZE / 2;
        v0.desired_heading          = 0.0f;
        v0.heading                  = 135.0f;
        v0.desired_speed            = 3.5f;
        v0.speed                    = 0.0f;
        v0.apparent_wind_dir        = 45;
        v0.apparent_wind_speed      = 10.0f;
        v0.last_updated             = 0;

        vessels.push_back(v0);
    }
}

Icon Fleet::NextIcon(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1){
    Icon icon;
    icon.scale = 0;
    icon.key = 0;
    while(icon.scale == 0){
        if(_vessel_itterater >= vessels.size())
            break;
        icon = vessels.at(_vessel_itterater).PopulateIcon(x0, y0, x1, y1);
        //cout << "Fleet::NextIcon vessels: pos_x: " << vessels.at(_vessel_itterater).pos_x << "\tpos_y: " << vessels.at(_vessel_itterater).pos_y << "\n";
        //cout << "Fleet::NextIcon icon:    pos_x: " << icon.pos_x << "\tpos_y: " << icon.pos_y << "\n";
        icon.key = ++_vessel_itterater;
    }
    return icon;
}

void Fleet::StartIcon(void){
    _vessel_itterater = 0;
}

void Fleet::CalculateVessels(float wind_speed, float wind_dir){
    for(std::vector<Vessel>::iterator it = vessels.begin() ; it != vessels.end(); ++it){
        it->Calculate(wind_speed, wind_dir);
        //cout << "Fleet::CalculateVessels pos_x: " << it->pos_x << "\tpos_y: " << it->pos_y << "\n";
    }
}
