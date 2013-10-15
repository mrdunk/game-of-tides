#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
//#include "time_code.c"

//#include <google/profiler.h>
//#include <google/heap-profiler.h>

#include "viewport.h"
#include "signaling.h"
#include "data.h"
#include "map.h"
#include "vessel.h"
#include "cockpit.h"
#include "console.h"


using namespace std;

void drawMap(int* shutdown, Map* p_testMap, Console* p_console){
    Signal Sig;
    Data data;

    timestamp_t end_second, end_frame;
    int count = 0;
    end_second = get_timestamp() + 1000 * 1000;
    while(*shutdown == 0){
        end_frame = get_timestamp() + 100 * 1000;
        if(get_timestamp() >= end_second){
            if(get_timestamp() >= end_second + 1000 * 1000){
                cout << "drawMap missed multiple frames\n";
                end_second = get_timestamp() + 1000 * 1000;
            }
            end_second += 1000 * 1000;
            //cout << count << " fps drawMap\n";
            p_console->mapFPS = count;
            count = 0;
        }
        
        Sig.ServiceSignals();
        p_testMap->ProcessAllTasks((end_frame - get_timestamp()) /100);
        data.Cull((end_frame - get_timestamp()) / 1000);

        if(end_frame > get_timestamp() +10)
            usleep(end_frame - get_timestamp());
        ++count;
    }
    ++(*shutdown);
    cout << "closing drawMap\n";
}

void drawBoats(int* shutdown, Map* p_testMap, Cockpit* p_cockpit, Console* p_console){
    //Signal Sig;
    Fleet vessels;
    Data data;
    
    timestamp_t end_second, end_frame;
    int count = 0;
    end_second = get_timestamp() + 1000 * 1000;
    while(*shutdown == 0){
        end_frame = get_timestamp() + 100 * 1000;
        if(get_timestamp() >= end_second){
            if(get_timestamp() >= end_second + 1000 * 1000){
                cout << "drawBoats missed multiple frames\n";
                end_second = get_timestamp() + 1000 * 1000;
            }
            end_second += 1000 * 1000;
            //cout << "data.mapData.size(): " << data.mapData.size() << "\t" << Data::mapData.size() << "\n";
            //cout << count << " fps drawBoats\n";
            p_console->boatsFPS = count;
            count = 0;
        }

        //Sig.ServiceSignals();
        vessels.CalculateVessels(data.wind_speed, data.wind_dir);
        p_testMap->DrawBoats();
        p_testMap->DrawText();
        p_cockpit->Draw();
        p_console->Draw();

        //now = get_timestamp();
        //time_left = 100 - ((now - then) / 1000);
        //p_testMap->ProcessAllTasks(time_left);

        //data.Cull((end_frame - get_timestamp()) / 1000);
        
        if(end_frame > get_timestamp() +10)
            usleep(end_frame - get_timestamp());
        ++count;
    }
    ++(*shutdown);
    cout << "closing drawBoats\n";
}

int main(int argc, char** argv)
{
    //ProfilerStart("profile_test");
    //HeapProfilerStart("heapprofile_test");

    int shutdown = 0;

    thread canvas(Init,800,0,1000,800,&shutdown,argc,argv);

    //Map testMap(SIG_DEST_MAP, 0, 200, 600, 600, LOW_RESOLUTION);
    Cockpit cockpit(SIG_DEST_TEST, 800, 0, 200, 200);
    Viewport testViewport2(SIG_DEST_TEST, 800, 200, 200, 200);
    Viewport testViewport3(SIG_DEST_TEST, 800, 350, 100, 200);
    Map testMap(SIG_DEST_MAP, 0, 0, 800, 800, LOW_RESOLUTION);
    Console console(SIG_DEST_TEST, 800, 600, 200, 200);

    testMap.Draw();
    //testMap.SetView(0, 0, 40000, 0);
    
    cockpit.SetView(0, 0, 10000, 0);

    testViewport2.SetView(0, 0, 0.5, 0);
    testViewport3.SetView(0, 0, 1, 45);

    Icon icon = testViewport2.TestIcon();
    Icon_key key;
    key.type = 1;
    key.key = 1;
    testViewport2.AddIcon(key, icon);
    testViewport2.RedrawIcons();

//    thread t3(drawBoats, &shutdown, &testMap, &cockpit);
    thread t3(drawMap, &shutdown, &testMap, &console);
    drawBoats(&shutdown, &testMap, &cockpit, &console);

    //ProfilerStop();
    //HeapProfilerStop();

    canvas.join();
    //ProfilerStop();

    // we never get here because glutMainLoop() has allready close()-ed.
    ++shutdown;
    t3.join();

    return 0;
}
