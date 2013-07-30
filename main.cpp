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
        usleep(FRAME_LENGTH * 500);
        Sig.ServiceSignals(); 
    }
}

int main(int argc, char** argv)
{

    thread canvas(Init,800,0,800,800,argc,argv);
    thread t(task1, "World");

    Viewport testViewport(SIG_DEST_TEST, 0, 0, 400, 400);
    Viewport testViewport2(SIG_DEST_TEST, 400, 0, 400, 600);
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

    canvas.join();
    t.join();

    return 0;
}
