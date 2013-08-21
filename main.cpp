#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <stdio.h>
//#include "time_code.c"

//#include <google/profiler.h>
//#include <google/heap-profiler.h>

#include "viewport.h"
#include "signaling.h"
#include "data.h"
#include "map.h"
#include "vessel.h"
#include "cockpit.h"


using namespace std;


void drawBoats(int* shutdown, Map* p_testMap, Cockpit* p_cockpit){
    Signal Sig;
    Fleet vessels;
    Data data;
    timestamp_t second, then, now;
    second = get_timestamp();
    int total_spare_time = 0;
    while(*shutdown == 0){
        then = get_timestamp();
        if((then - second) / 1000 > 1000){
            cout << "total_spare_time:    " << total_spare_time << " ms\n";
            cout << "data.mapData.size(): " << data.mapData.size() << "\n";
            total_spare_time = 0;
            second = get_timestamp();
        }

        Sig.ServiceSignals();
        vessels.CalculateVessels(data.wind_speed, data.wind_dir);
        p_testMap->DrawBoats();
        p_testMap->DrawText();
        p_cockpit->Draw();

        now = get_timestamp();
        int time_left = 100 - ((now - then) / 1000);
        p_testMap->ProcessAllTasks(time_left);

        now = get_timestamp();
        time_left = 100 - ((now - then) / 1000);
        if(time_left > 10)
        data.Cull(10);
        
        now = get_timestamp();
        time_left = 100 - ((now - then) / 1000);
        total_spare_time += time_left;
        //cout << time_left << "\n";
        if(time_left > 0)
            usleep(time_left * 1000);
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
    drawBoats(&shutdown, &testMap, &cockpit);

    //ProfilerStop();
    //HeapProfilerStop();

    canvas.join();
    //ProfilerStop();

    // we never get here because glutMainLoop() has allready close()-ed.
    ++shutdown;
//    t3.join();

    return 0;
}
