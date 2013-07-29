#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <stdio.h>

#include "viewport.h"
#include "signaling.h"
#include "data.h"
#include "map.h"


using namespace std;


void task1(string msg)
{
    Signal Sig;
    //signal sig;
    while(true){
        usleep(FRAME_LENGTH * 100);
        Sig.ServiceSignals(); 
    }
}

int main(int argc, char** argv)
{

    thread canvas(Init,800,0,800,800,argc,argv);
    thread t(task1, "World");

    Viewport testViewport(SIG_DEST_TEST, 0, 0, 400, 200);
    Viewport testViewport2(SIG_DEST_TEST, 400, 0, 400, 600);
    Viewport testViewport3(SIG_DEST_TEST, 250, 50, 100, 100);
    Map testMap(SIG_DEST_MAP, 0, 200, 600, 600, LOW_RESOLUTION);
    //Map testMap(SIG_DEST_MAP, 0, 0, 800,800, LOW_RESOLUTION);

    //testMap.SetView(0,0,5,0);

    testMap.Draw();
    testViewport.SetView(0.5, 0, 1, 45); 
    testViewport3.SetView(0.5, 0, 0.5, 45);

    Icon icon = testMap.TestIcon();
    Icon_key key;
    key.type = 1;
    key.key = 1;
    testMap.AddIcon(key, icon);
    testMap.RedrawIcons();

    canvas.join();
    t.join();

    return 0;
}
