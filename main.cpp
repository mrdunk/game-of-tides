#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <stdio.h>
#include "time_code.c"

//#include <google/profiler.h>
//#include <google/heap-profiler.h>

#include "viewport.h"
#include "signaling.h"
#include "data.h"
#include "map.h"
#include "vessel.h"
#include "cockpit.h"


using namespace std;


void signal_thread(int* shutdown)
{
    Signal Sig;
    while(*shutdown == 0){
        usleep(FRAME_LENGTH * 500);
        Sig.ServiceSignals(); 
    }
    ++(*shutdown);
    cout << "closing signal_thread\n";
}

void housekeeping(int* shutdown){
    Data data;
    while(*shutdown == 0){
        usleep(1000000);
        data.Cull(shutdown);
    }
    ++(*shutdown);
    cout << "closing housekeeping\n";
}

void drawBoats(int* shutdown, Map* p_testMap, Cockpit* p_cockpit){
    usleep(1000000);     // 1000ms to allow boats to initialise in Map thread.
    Fleet vessels;
    Data data;
    timestamp_t then, now;
    then = now = get_timestamp();
    while(*shutdown == 0){
        while((int)(now - then) / 1000.0L < 100){
            usleep(1000);
            now = get_timestamp();
        }
        //cout << "tick " << (int)(now - then) / 1000.0L << "\n";

        vessels.CalculateVessels(data.wind_speed, data.wind_dir);
        p_testMap->DrawBoats();
        p_testMap->DrawText();
        p_cockpit->Draw();

        then = get_timestamp();
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
    thread t(signal_thread, &shutdown);
    thread t2(housekeeping, &shutdown);

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

    thread t3(drawBoats, &shutdown, &testMap, &cockpit);

    //ProfilerStop();
    //HeapProfilerStop();

    canvas.join();
    //ProfilerStop();

    // we never get here because glutMainLoop() has allready close()-ed.
    ++shutdown;
    t.join();
    t2.join();
    t3.join();

    return 0;
}
