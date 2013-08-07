#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <stdio.h>

#include "viewport.h"
#include "signaling.h"
#include "data.h"
#include "map.h"
#include "vessel.h"


using namespace std;


void task1(int* shutdown)
{
    Signal Sig;
    //signal sig;
    while(*shutdown == 0){
        usleep(FRAME_LENGTH * 500);
        Sig.ServiceSignals(); 
    }
    ++(*shutdown);
    cout << "closing task1\n";
}

void housekeeping(int* shutdown){
    Data data;
    while(*shutdown == 0){
        usleep(1000000);
        data.Cull();
    }
    ++(*shutdown);
    cout << "closing housekeeping\n";
}

void drawBoats(int* shutdown, Map* p_testMap){
    Fleet vessels;
    Data data;
    while(*shutdown == 0){
        usleep(100000);     // 100ms
        //usleep(1000000);
        cout << "tick\n";
        vessels.CalculateVessels(data.wind_speed, data.wind_dir);
        p_testMap->DrawBoats();
        p_testMap->RedrawIcons();
    }
    ++(*shutdown);
    cout << "closing drawBoats\n";
}

int main(int argc, char** argv)
{
    int shutdown = 0;

    thread canvas(Init,800,0,800,800,&shutdown,argc,argv);
    thread t(task1, &shutdown);
    thread t2(housekeeping, &shutdown);

    Viewport testViewport(SIG_DEST_TEST, 0, 0, 400, 400);
    Viewport testViewport2(SIG_DEST_TEST, 400, 0, 400, 200);
    Viewport testViewport3(SIG_DEST_TEST, 250, 50, 200, 100);
    Map testMap(SIG_DEST_MAP, 0, 200, 800, 600, LOW_RESOLUTION);

    testMap.Draw();
    testViewport.SetView(20, 20, 1, 0);
    testViewport2.SetView(0, 0, 0.5, 0);
    testViewport3.SetView(0, 0, 1, 45);

    Icon icon = testViewport2.TestIcon();
    Icon_key key;
    key.type = 1;
    key.key = 1;
    testViewport2.AddIcon(key, icon);
    testViewport2.RedrawIcons();

    thread t3(drawBoats, &shutdown, &testMap);

    canvas.join();
    // we never get here because glutMainLoop() has allready close()-ed.
    ++shutdown;
    t.join();
    t2.join();
    t3.join();

    return 0;
}
