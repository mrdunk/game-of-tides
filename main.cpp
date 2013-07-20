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
    signal sig;
    while(true){
        usleep(FRAME_LENGTH * 1000);
        while(Sig.PopEvent(&sig)){
            cout << " " << Sig._read_counter << "\tsig.source: " << sig.source << "\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
        }
    }
}

int main(int argc, char** argv)
{

    thread canvas(Init,800,0,800,800,argc,argv);
    thread t(task1, "World");

    Viewport testViewport(0, 0, 400, 200);
    Viewport testViewport2(400, 0, 400, 600);
    Viewport testViewport3(250, 50, 100, 100);
    Map testMap(0, 200, 600, 600);
    //testMap.SetView(0,0,5,0);

    testMap.Draw();
    testViewport.SetView(0.5, 0, 1, 45); 
    testViewport3.SetView(0.5, 0, 0.5, 45);

    canvas.join();
    t.join();

    return 0;
}
