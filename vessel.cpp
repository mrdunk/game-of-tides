#include <iostream>
#include <math.h>
#include <GL/freeglut.h>

#include "vessel.h"

#define PI 3.14157

using namespace std;

void Vessel::Calculate(float wind_speed, float wind_dir){
    //cout << "Vessel::Calculate " << description << "\n";
    
    //heading = desired_heading;

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
        float force_drag = 0.5f * AIR_DENSITY * ((float)it_spar->length * it_spar->length * 0.02) * 1.5 * apparent_wind_speed * apparent_wind_speed;
        float force_drag_x = force_drag * sin(PI * apparent_wind_dir / 180);
        float force_drag_y = force_drag * cos(PI * apparent_wind_dir / 180);

        force_leway += -force_drag_x;
        force_forward += -force_drag_y;

        if(it_spar->type == SPAR_TYPE_BOWSPRIT){
            force_rotation -= force_drag_x * (cod + it_spar->length /2) / length;
        }
        else if(it_spar->type == SPAR_TYPE_MAST){
            force_rotation -= force_drag_x * ((int)cod - (int)it_spar->position) / (int)length;
            force_heel += force_drag_x * it_spar->length /2;
        }
        
        //cout << it_spar->description << "\t" << (int)(force_drag_x * ((int)cod - (int)it_spar->position) / (int)length) << "\n";
        //cout << "\t\t" << ((int)cod - (int)it_spar->position) / (int)length << "\n";

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
            if(it_sail->state == SAIL_STATE_SET){
                if(optimise_trim)
                    it_sail->sheeted = (float)apparent_wind_dir / 2;
                aoa = it_sail->sheeted;
                if(apparent_wind_dir >= 0){
                    if(apparent_wind_dir < it_sail->sheeted){
                        it_sail->sheeted = apparent_wind_dir;
                    }
                    if(it_sail->sheeted < it_sail->max_sheeted){
                        it_sail->sheeted = it_sail->max_sheeted;
                    }
                    if(it_sail->sheeted > it_sail->min_sheeted){
                        it_sail->sheeted = it_sail->min_sheeted;
                    }
                } else {
                    if(apparent_wind_dir > it_sail->sheeted){
                        it_sail->sheeted = apparent_wind_dir;
                    }
                    if(it_sail->sheeted > -it_sail->max_sheeted){
                        it_sail->sheeted = -it_sail->max_sheeted;
                    }
                    if(it_sail->sheeted < -it_sail->min_sheeted){
                        it_sail->sheeted = -it_sail->min_sheeted;
                    }
                }
                aoa = it_sail->sheeted;
            } else {
                // sail pointing straight down wind.
                it_sail->sheeted = apparent_wind_dir;
                aoa = 0;
            }

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
            force_leway += -force_lift_x -force_drag_x;
            force_forward += force_lift_y - force_drag_y;

            force_rotation -= (force_lift_x + force_drag_x) * (cod - centre_of_effort) / length;
            force_heel += (force_lift_x + force_drag_x) * ((it_sail->height * it_sail->deployed) /3 + it_sail->tack_height);
        }
    }

    int timeslice = 10;


    // Apply forward_force.
    // force = mass * acceleration.
    // Acceleration is the change in speed in a set time.
    // Also the force due to drag is prportinal to the speed.
    float drag_coeficient_face = .05;
    float drag_coeficient_side = 10;
    int boat_area_face = beam * draft / 2;
    int boat_area_side = length * draft / 2;
    double resistive_force = 0.5 * WATER_DENSITY * boat_area_face * drag_coeficient_face * speed * std::abs(speed);
    double leeway_resistive_force = 0.5 * WATER_DENSITY * boat_area_side * drag_coeficient_side * leeway_speed * std::abs(leeway_speed);

    force_forward -= resistive_force;
    force_leway -= leeway_resistive_force;

    speed += ((float)force_forward / displacment) / timeslice / 5;
    leeway_speed += ((double)force_leway / displacment) / timeslice / 5;

    long force_rudder = 0;
    if(state == VESSEL_STATE_MAKING_WAY){

        if(desired_heading < heading){
            force_rudder = -(double)speed * speed * length  * length *2;
        } else {
            force_rudder = (double)speed * speed * length  * length *2;
        }

        if(heading - desired_heading > 180.0 or heading - desired_heading < -180.0){
            force_rudder = -force_rudder;
        }

        if(desired_heading - heading > -2 and desired_heading - heading < 2){
            force_rudder /= 10;
        }
        //cout << description << "\t" << heading << "\t" << desired_heading << "\t" << force_rudder << "\n";
    }
    //cout << description << "\t" << (int)force_rotation << "\t" << force_rudder << "\n";
    //cout << heading << "\t" << desired_heading << "\n";
    
    force_rotation += force_rudder;

    //cout << (int)force_rotation << "\t" << displacment << "\t" << force_rotation/displacment << "\n\n";

    heading += force_rotation / displacment / 4;
    if(heading < -180)
        heading += 360.0;
    else if(heading > 180)
        heading -= 360.0;

    /* 1 knot = 0.514444444 m/s. We round to 0.5m/s.
     * pow(2, MIN_RESOLUTION) == 16                   */
    // forwards
    pos_x += (float)sin(PI * heading / 180.0) * (float)speed * 16.0 * 0.5 / timeslice;
    pos_y += (float)cos(PI * heading / 180.0) * (float)speed * 16.0 * 0.5 / timeslice;

    //leeway
    pos_x += (float)cos(PI * heading / 180.0) * (float)leeway_speed * 16.0 * 0.5 / timeslice;
    pos_y -= (float)sin(PI * heading / 180.0) * (float)leeway_speed * 16.0 * 0.5 / timeslice;

}

Icon Vessel::PopulateIcon(int window_index){
    Icon icon;
    
    int mouse_dist_to_boat = -1;
    if(windows[window_index].mouse_x_rel >= 0 or windows[window_index].mouse_y_rel >= 0){
        mouse_dist_to_boat = sqrt(((windows[window_index].mouse_x_rel - pos_x) * (windows[window_index].mouse_x_rel - pos_x)) + 
                                  ((windows[window_index].mouse_y_rel - pos_y) * (windows[window_index].mouse_y_rel - pos_y))) / 16;
        //cout << mouse_dist_to_boat << "\n";
    }

/*    if(!(pos_x >= x0 and pos_x <= x1 and pos_y >= y0 and pos_y <= y1)){
        // Vessel not in view area.
        cout << "Vessel \"" << description << "\" not in view area.\n";
        icon.scale = 0;
        return icon;
    }*/
    //cout << "Vessel::PopulateIcon " << description << "\n";

    icon.angle = heading;
    icon.scale = (float)16/100;     // 16 == pow(2, MIN_RECURSION), 100 == all vessel dimensions are specified in cm.
    //icon.scale = 1000;
    icon.fixed_size = 0;
    icon.centre_x = 0;
    icon.centre_y = 0;
    icon.pos_x = pos_x;
    icon.pos_y = pos_y;
    

    int bowsprit = 0;

    for(std::vector<Spar>::iterator it = spars.begin() ; it != spars.end(); ++it){
        if(it->type == SPAR_TYPE_BOWSPRIT){
            bowsprit = it->length / 2;
        }
    }

    int size = length;
    float mouse_rel_boat_x = -1, mouse_rel_boat_y = -1, mouse_on_boat_x = -1, mouse_on_boat_y = -1;
    if(windows[window_index].mouse_x >= 0 or windows[window_index].mouse_y >= 0){

        mouse_rel_boat_x = (windows[window_index].mouse_x_rel - pos_x) / 16;
        mouse_rel_boat_y = (windows[window_index].mouse_y_rel - pos_y) / 16;

        mouse_on_boat_x = mouse_rel_boat_x * cos(3.141 * heading / 180) - mouse_rel_boat_y * sin(3.141 * heading / 180);
        mouse_on_boat_y = mouse_rel_boat_x * sin(3.141 * heading / 180) + mouse_rel_boat_y * cos(3.141 * heading / 180);
        mouse_on_boat_y = (float)length/200 - mouse_on_boat_y;

        //cout << window_index << "\t\t\t" << mouse_on_boat_x << " ,\t" << mouse_on_boat_y << "\n";
    }

    // Mouse close to boat so activate highlighter.
    if(mouse_dist_to_boat >= (int)(length + bowsprit)/200 and mouse_dist_to_boat < (int)length/100){
        float potential_desired_heading;

        potential_desired_heading = atan(mouse_rel_boat_x / mouse_rel_boat_y) * 180.0 / 3.141;
        if(mouse_rel_boat_y < 0)
            potential_desired_heading += 180;
        //cout << mouse_rel_boat_x << " , " << mouse_rel_boat_y << "\t" << potential_desired_heading << "\n";

        if(windows[window_index].mouse_button == GLUT_LEFT_BUTTON){
            cout << "GLUT_LEFT_BUTTON\t" << windows[window_index].mouse_button << GLUT_LEFT_BUTTON << "\n";
            desired_heading = potential_desired_heading;
            if(desired_heading > 180){
                desired_heading -= 360;
            } else if(desired_heading < -180){
                desired_heading += 360;
            }
        }

        icon.points.push_back (sin((potential_desired_heading - heading) * PI / 180.0) * length);
        icon.points.push_back (cos((potential_desired_heading - heading) * PI / 180.0) * length);
        icon.points.push_back (sin((potential_desired_heading - heading +160) * PI / 180.0) * length);
        icon.points.push_back (cos((potential_desired_heading - heading +160) * PI / 180.0) * length);
        icon.points.push_back (sin((potential_desired_heading - heading +200) * PI / 180.0) * length);
        icon.points.push_back (cos((potential_desired_heading - heading +200) * PI / 180.0) * length);

        icon.colour.push_back (255);
        icon.colour.push_back (255);
        icon.colour.push_back (0);
        icon.colour.push_back (255);
        icon.colour.push_back (255);
        icon.colour.push_back (0);
        icon.colour.push_back (255);
        icon.colour.push_back (255);
        icon.colour.push_back (0);
    }


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
            
            aoa = apparent_wind_dir - it_sail->sheeted;

            int foot = it_sail->foot * sqrt(it_sail->deployed);
            int sail_col = 255;
            // See if mouse pointer near sail.
            if(mouse_on_boat_y * 100 > tack_on_boat + bowsprit and mouse_on_boat_y * 100 < tack_on_boat + bowsprit + foot){
                float dist_from_tack = sqrt((mouse_on_boat_x*100 * mouse_on_boat_x*100) + ((tack_on_boat + bowsprit - mouse_on_boat_y*100) * (tack_on_boat + bowsprit - mouse_on_boat_y*100)));
                float angle_to_tack = asin(mouse_on_boat_x*100 / dist_from_tack) * 180.0 / 3.141;
                float dist_to_sail = sin((apparent_wind_dir - aoa + angle_to_tack) * 3.141 / 180.0) * dist_from_tack;
                if((unsigned int)dist_to_sail < (unsigned)foot / 4){
                    sail_col = 0;
                    if(windows[window_index].mouse_button == GLUT_LEFT_BUTTON){
                        if(it_sail->state == SAIL_STATE_SET)
                            it_sail->state = SAIL_STATE_UNSET;
                        else
                            it_sail->state = SAIL_STATE_SET;
                    }
                }
            }

            if(it_sail->type == SAIL_TYPE_AFT){
                icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);
                icon.points.push_back(-(int)foot * sin(3.141 * (apparent_wind_dir - aoa) / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)((float)foot * cos(3.141 * (apparent_wind_dir - aoa) / 180)));
                icon.points.push_back((int)it_spar->length/MAST_RATIO);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);

                icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);
                icon.points.push_back(-(int)foot * sin(3.141 * ((apparent_wind_dir - aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * ((apparent_wind_dir - aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back(-(int)foot * sin(3.141 * (apparent_wind_dir - aoa) / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * (apparent_wind_dir - aoa) / 180));
/*
                icon.points.push_bac6k(-(int)foot * sin(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back(-(int)foot * sin(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3 - (int)foot * sin(3.141 * aoa / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * ((aoa + apparent_wind_dir)/2) / 180) / 3  - (int)foot * cos(3.141 * aoa / 180) / 3);
                icon.points.push_back(-(int)foot * sin(3.141 * aoa / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * aoa / 180));
*/
                for(int i = 0; i < 6; i++){
                    icon.colour.push_back(255);
                    icon.colour.push_back(sail_col);
                    icon.colour.push_back(255);
                }
            } else if(it_sail->type == SAIL_TYPE_FORE){
                icon.points.push_back(0);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat);
                icon.points.push_back(-(int)foot * sin(3.141 * ((apparent_wind_dir - aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * ((apparent_wind_dir - aoa + apparent_wind_dir)/2) / 180) / 3);
                icon.points.push_back(-(int)foot * sin(3.141 * (apparent_wind_dir - aoa) / 180));
                icon.points.push_back((size/2) - bowsprit - tack_on_boat - (int)foot * cos(3.141 * (apparent_wind_dir - aoa) / 180));
  
                //icon.points.push_back(-(int)it_spar->length/MAST_RATIO);
                //icon.points.push_back((size/2) - bowsprit - tack_on_boat);


                for(int i = 0; i < 3; i++){
                    icon.colour.push_back(255);
                    icon.colour.push_back(sail_col);
                    icon.colour.push_back(255);
                }
            }
        }
    }

    return icon;

}

std::vector<Vessel> Fleet::vessels;
bool Fleet::_fleet_lock = 0;

Fleet::Fleet(void){
    cout << "Fleet::Fleet\n";
    while(_fleet_lock){
        cout << "." << flush;
    }
    cout << "\n";
    if(!vessels.size()){
         cout << "*\n";;
        _fleet_lock = 1;
        StartIcon();


        Vessel v0;
        Spar v0_sp0, v0_sp1;
        Sail v0_sp1_sa0, v0_sp1_sa1, v0_sp0_sa2;

        v0.description  = "Test skiff";
        v0.type         = VESSEL_TYPE_BOAT;
        v0.length       = 800;
        v0.beam         = 200;
        v0.draft        = 100;
        v0.displacment  = v0.length * v0.beam * v0.draft /4;
        v0.cod          = 400;


        v0_sp0.description  = "Bowsprit";
        v0_sp0.type         = SPAR_TYPE_BOWSPRIT;
        v0_sp0.position     = 0;
        v0_sp0.length       = 100;

        v0_sp1.description  = "Mast";
        v0_sp1.type         = SPAR_TYPE_MAST;
        v0_sp1.position     = 300;
        v0_sp1.length       = 850;

        v0_sp1_sa0.description      = "Jib";
        v0_sp1_sa0.type             = SAIL_TYPE_FORE;
        v0_sp1_sa0.tack_height      = 50;
        v0_sp1_sa0.tack_position    = 250;
        v0_sp1_sa0.height           = 800;
        v0_sp1_sa0.foot             = 350;
        v0_sp1_sa0.reef_size        = 0.2f;
        v0_sp1_sa0.deployed         = 1.0f;
        v0_sp1_sa0.sheeted          = 30;
        v0_sp1_sa0.min_sheeted      = 180;
        v0_sp1_sa0.max_sheeted      = 15;
        v0_sp1_sa0.state            = SAIL_STATE_SET;

        v0_sp1_sa1.description      = "Main";
        v0_sp1_sa1.type             = SAIL_TYPE_AFT;
        v0_sp1_sa1.tack_height      = 50;
        v0_sp1_sa1.tack_position    = 0;
        v0_sp1_sa1.height           = 800;
        v0_sp1_sa1.foot             = 450;
        v0_sp1_sa1.reef_size        = 0.2f;
        v0_sp1_sa1.deployed         = 1.0f;
        v0_sp1_sa1.sheeted          = 30;
        v0_sp1_sa1.min_sheeted      = 90;
        v0_sp1_sa1.max_sheeted      = 0;
        v0_sp1_sa1.state            = SAIL_STATE_SET;

        v0_sp0_sa2.description      = "Jib";
        v0_sp0_sa2.type             = SAIL_TYPE_FORE;
        v0_sp0_sa2.tack_height      = 50;
        v0_sp0_sa2.tack_position    = -100;
        v0_sp0_sa2.height           = 800;
        v0_sp0_sa2.foot             = 250;
        v0_sp0_sa2.reef_size        = 0.2f;
        v0_sp0_sa2.deployed         = 1.0f;
        v0_sp0_sa2.sheeted          = 30;
        v0_sp0_sa2.min_sheeted      = 180;
        v0_sp0_sa2.max_sheeted      = 15;
        v0_sp0_sa2.state            = SAIL_STATE_SET;

        v0_sp1.sails.push_back(v0_sp1_sa0);
        v0_sp1.sails.push_back(v0_sp1_sa1);
        v0_sp0.sails.push_back(v0_sp0_sa2);

        v0.spars.push_back(v0_sp0);
        v0.spars.push_back(v0_sp1);

        v0.state                    = VESSEL_STATE_MAKING_WAY;
        v0.pos_x                    = MAX_SIZE / 2;
        v0.pos_y                    = MAX_SIZE / 2;
        v0.desired_heading          = 0.0f;
        v0.heading                  = 0.0f;
        v0.desired_speed            = 3.5f;
        v0.speed                    = 0.0f;
        v0.apparent_wind_dir        = 45;
        v0.apparent_wind_speed      = 10.0f;
        v0.last_updated             = 0;

        vessels.push_back(v0);


        Vessel v1;
        Spar v1_sp0, v1_sp1, v1_sp2, v1_sp3;
        Sail v1_sp0_sa0, v1_sp0_sa1, v1_sp0_sa2, v1_sp1_sa0, v1_sp1_sa1, v1_sp2_sa0, v1_sp3_sa0;
                
        v1.description  = "Schooner";
        v1.type         = VESSEL_TYPE_BOAT;
        v1.length       = 4800;
        v1.beam         = 690;
        v1.draft        = 280;
        v1.displacment  = v1.length * v1.beam * v1.draft / 4;
        v1.cod          = 2400;


        v1_sp0.description  = "Bowsprit";
        v1_sp0.type         = SPAR_TYPE_BOWSPRIT;
        v1_sp0.position     = 0;
        v1_sp0.length       = 1000;

        v1_sp1.description  = "Fore Mast";
        v1_sp1.type         = SPAR_TYPE_MAST;
        v1_sp1.position     = 1100;
        v1_sp1.length       = 2700;

        v1_sp2.description  = "Main Mast";
        v1_sp2.type         = SPAR_TYPE_MAST;
        v1_sp2.position     = 2250;
        v1_sp2.length       = 2900;
        
        v1_sp3.description  = "Mizzen";
        v1_sp3.type         = SPAR_TYPE_MAST;
        v1_sp3.position     = 3400;
        v1_sp3.length       = 2500;

        v1_sp1_sa0.description      = "Schooner Sail";
        v1_sp1_sa0.type             = SAIL_TYPE_AFT;
        v1_sp1_sa0.tack_height      = 200;
        v1_sp1_sa0.tack_position    = 0;
        v1_sp1_sa0.height           = 1700;
        v1_sp1_sa0.foot             = 1100;
        v1_sp1_sa0.reef_size        = 0.2f;
        v1_sp1_sa0.deployed         = 1.0f;
        v1_sp1_sa0.sheeted          = 30;
        v1_sp1_sa0.min_sheeted      = 90;
        v1_sp1_sa0.max_sheeted      = 0;
        v1_sp1_sa0.state            = SAIL_STATE_SET;

        v1_sp2_sa0.description      = "Main Sail";
        v1_sp2_sa0.type             = SAIL_TYPE_AFT;
        v1_sp2_sa0.tack_height      = 200;
        v1_sp2_sa0.tack_position    = 0;
        v1_sp2_sa0.height           = 2700;
        v1_sp2_sa0.foot             = 1100;
        v1_sp2_sa0.reef_size        = 0.2f;
        v1_sp2_sa0.deployed         = 1.0f;
        v1_sp2_sa0.sheeted          = 30;
        v1_sp2_sa0.min_sheeted      = 90;
        v1_sp2_sa0.max_sheeted      = 0;
        v1_sp2_sa0.state            = SAIL_STATE_SET;

        v1_sp3_sa0.description      = "Mizzen Sail";
        v1_sp3_sa0.type             = SAIL_TYPE_AFT;
        v1_sp3_sa0.tack_height      = 200;
        v1_sp3_sa0.tack_position    = 0;
        v1_sp3_sa0.height           = 2300;
        v1_sp3_sa0.foot             = 1100;
        v1_sp3_sa0.reef_size        = 0.2f;
        v1_sp3_sa0.deployed         = 1.0f;
        v1_sp3_sa0.sheeted          = 30;
        v1_sp3_sa0.min_sheeted      = 90;
        v1_sp3_sa0.max_sheeted      = 0;
        v1_sp3_sa0.state            = SAIL_STATE_SET;

        v1_sp1_sa1.description      = "Fore Sail";
        v1_sp1_sa1.type             = SAIL_TYPE_FORE;
        v1_sp1_sa1.tack_height      = 100;
        v1_sp1_sa1.tack_position    = 1100;
        v1_sp1_sa1.height           = 1700;
        v1_sp1_sa1.foot             = 1100;
        v1_sp1_sa1.reef_size        = 0.2f;
        v1_sp1_sa1.deployed         = 1.0f;
        v1_sp1_sa1.sheeted          = 30;
        v1_sp1_sa1.min_sheeted      = 180;
        v1_sp1_sa1.max_sheeted      = 15;
        v1_sp1_sa1.state            = SAIL_STATE_SET;

        v1_sp0_sa0.description      = "Inner Jib";
        v1_sp0_sa0.type             = SAIL_TYPE_FORE;
        v1_sp0_sa0.tack_height      = 100;
        v1_sp0_sa0.tack_position    = -500;
        v1_sp0_sa0.height           = 1700;
        v1_sp0_sa0.foot             = 600;
        v1_sp0_sa0.reef_size        = 0.2f;
        v1_sp0_sa0.deployed         = 1.0f;
        v1_sp0_sa0.sheeted          = 30;
        v1_sp0_sa0.min_sheeted      = 180;
        v1_sp0_sa0.max_sheeted      = 15;
        v1_sp0_sa0.state            = SAIL_STATE_SET;

        v1_sp0_sa1.description      = "Outer Jib";
        v1_sp0_sa1.type             = SAIL_TYPE_FORE;
        v1_sp0_sa1.tack_height      = 100;
        v1_sp0_sa1.tack_position    = -1000;
        v1_sp0_sa1.height           = 1700;
        v1_sp0_sa1.foot             = 600;
        v1_sp0_sa1.reef_size        = 0.2f;
        v1_sp0_sa1.deployed         = 1.0f;
        v1_sp0_sa1.sheeted          = 30;
        v1_sp0_sa1.min_sheeted      = 180;
        v1_sp0_sa1.max_sheeted      = 15;
        v1_sp0_sa1.state            = SAIL_STATE_SET;

        v1_sp0_sa2.description      = "Top Jib";
        v1_sp0_sa2.type             = SAIL_TYPE_FORE;
        v1_sp0_sa2.tack_height      = 1600;
        v1_sp0_sa2.tack_position    = -500;
        v1_sp0_sa2.height           = 1100;
        v1_sp0_sa2.foot             = 1600;
        v1_sp0_sa2.reef_size        = 0.2f;
        v1_sp0_sa2.deployed         = 1.0f;
        v1_sp0_sa2.sheeted          = 30;
        v1_sp0_sa2.min_sheeted      = 180;
        v1_sp0_sa2.max_sheeted      = 15;
        v1_sp0_sa2.state            = SAIL_STATE_SET;

        v1_sp0.sails.push_back(v1_sp0_sa0);
        v1_sp0.sails.push_back(v1_sp0_sa1);
        v1_sp0.sails.push_back(v1_sp0_sa2);
        v1_sp1.sails.push_back(v1_sp1_sa0);
        v1_sp1.sails.push_back(v1_sp1_sa1);
        v1_sp2.sails.push_back(v1_sp2_sa0);
        v1_sp3.sails.push_back(v1_sp3_sa0);

        v1.spars.push_back(v1_sp0);
        v1.spars.push_back(v1_sp1);
        v1.spars.push_back(v1_sp2);
        v1.spars.push_back(v1_sp3);


        v1.state                    = VESSEL_STATE_MAKING_WAY;
        v1.pos_x                    = MAX_SIZE / 2 + 160;
        v1.pos_y                    = MAX_SIZE / 2;
        v1.desired_heading          = 0.0f;
        v1.heading                  = 0.0f;
        v1.desired_speed            = 3.5f;
        v1.speed                    = 0.0f;
        v1.apparent_wind_dir        = 45;
        v1.apparent_wind_speed      = 10.0f;
        v1.last_updated             = 0;

        vessels.push_back(v1);


        Vessel v2;
        Spar v2_sp0, v2_sp1;
        Sail v2_sp1_sa0, v2_sp1_sa1, v2_sp0_sa2;

        v2.description  = "Test skiff 2";
        v2.type         = VESSEL_TYPE_BOAT;
        v2.length       = 3200;
        v2.beam         = 800;
        v2.draft        = 600;
        v2.displacment  = v2.length * v2.beam * v2.draft /4;
        v2.cod          = 1600;


        v2_sp0.description  = "Bowsprit";
        v2_sp0.type         = SPAR_TYPE_BOWSPRIT;
        v2_sp0.position     = 0;
        v2_sp0.length       = 400;

        v2_sp1.description  = "Mast";
        v2_sp1.type         = SPAR_TYPE_MAST;
        v2_sp1.position     = 1200;
        v2_sp1.length       = 3400;

        v2_sp1_sa0.description      = "Jib";
        v2_sp1_sa0.type             = SAIL_TYPE_FORE;
        v2_sp1_sa0.tack_height      = 100;
        v2_sp1_sa0.tack_position    = 1000;
        v2_sp1_sa0.height           = 3200;
        v2_sp1_sa0.foot             = 1400;
        v2_sp1_sa0.reef_size        = 0.2f;
        v2_sp1_sa0.deployed         = 1.0f;
        v2_sp1_sa0.sheeted          = 30;
        v2_sp1_sa0.min_sheeted      = 180;
        v2_sp1_sa0.max_sheeted      = 15;
        v2_sp1_sa0.state            = SAIL_STATE_SET;

        v2_sp1_sa1.description      = "Main";
        v2_sp1_sa1.type             = SAIL_TYPE_AFT;
        v2_sp1_sa1.tack_height      = 200;
        v2_sp1_sa1.tack_position    = 0;
        v2_sp1_sa1.height           = 3200;
        v2_sp1_sa1.foot             = 1800;
        v2_sp1_sa1.reef_size        = 0.2f;
        v2_sp1_sa1.deployed         = 1.0f;
        v2_sp1_sa1.sheeted          = 30;
        v2_sp1_sa1.min_sheeted      = 90;
        v2_sp1_sa1.max_sheeted      = 0;
        v2_sp1_sa1.state            = SAIL_STATE_SET;

        v2_sp0_sa2.description      = "Jib";
        v2_sp0_sa2.type             = SAIL_TYPE_FORE;
        v2_sp0_sa2.tack_height      = 50;
        v2_sp0_sa2.tack_position    = -400;
        v2_sp0_sa2.height           = 3200;
        v2_sp0_sa2.foot             = 1000;
        v2_sp0_sa2.reef_size        = 0.2f;
        v2_sp0_sa2.deployed         = 1.0f;
        v2_sp0_sa2.sheeted          = 30;
        v2_sp0_sa2.min_sheeted      = 180;
        v2_sp0_sa2.max_sheeted      = 15;
        v2_sp0_sa2.state            = SAIL_STATE_SET;

        v2_sp1.sails.push_back(v2_sp1_sa0);
        v2_sp1.sails.push_back(v2_sp1_sa1);
        v2_sp0.sails.push_back(v2_sp0_sa2);

        v2.spars.push_back(v2_sp0);
        v2.spars.push_back(v2_sp1);

        v2.state                    = VESSEL_STATE_MAKING_WAY;
        v2.pos_x                    = MAX_SIZE / 2 - 16*20;
        v2.pos_y                    = MAX_SIZE / 2;
        v2.desired_heading          = 0.0f;
        v2.heading                  = 0.0f;
        v2.desired_speed            = 3.5f;
        v2.speed                    = 0.0f;
        v2.apparent_wind_dir        = 45;
        v2.apparent_wind_speed      = 10.0f;
        v2.last_updated             = 0;

        vessels.push_back(v2);

    }
    _fleet_lock = 0;
    cout << "Fleet::Fleet -\n";
}

Icon Fleet::NextIcon(int window_index){
    Icon icon;
    icon.scale = 0;
    icon.key = -1;
    while(icon.key == -1){
        cout << "Fleet::NextIcon " << _vessel_itterater << "\t";
        if(_vessel_itterater >= vessels.size()){
            cout << " .\n\n";
            break;
        }
        icon = vessels.at(_vessel_itterater).PopulateIcon(window_index);
        icon.key = ++_vessel_itterater;
        cout << icon.key << "\n";
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
