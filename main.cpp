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
    //thread t1(task1, "Hello", 2);
    //thread t2(task1, "World", 1);
        
    //cout << "main says: hello\n" << flush;

    //t1.join();
    //t2.join();

    std::unordered_map<std::string, MapPoint> mapData;

    MapPoint testcoord;
    testcoord.x = 256;
    testcoord.y = 128;
    cout << "testcoord.toString: " << testcoord.toString() << "\n";

    testcoord.stringToInt("123,456");
    cout << "testcoord.toString: " << testcoord.toString() << "\n";

    cout << "-------------------\n";
//    MapPoint parent;
//    for(int row = 0; row <=MAX_SIZE; row+=(MAX_SIZE/64)){
//        for(int col = 0; col <=MAX_SIZE; col+=(MAX_SIZE/64)){
//            testcoord.x = col; testcoord.y = row;
//            testcoord.calculateZ(&mapData);
//            cout << row << "," << col << "\t\t" << testcoord.z << "\n";
//        }
//    }

    thread canvas(Init,800,0,800,800,argc,argv);
    thread t(task1, "World");

    Viewport testViewport(0, 0, 400, 200);
    Viewport testViewport2(400, 0, 400, 600);
    Viewport testViewport3(250, 250, 100, 100);
    Map testMap(0, 200, 200, 200);
    testMap.Draw();
    testViewport.SetView(0.5, 0, 1, 45); 
    testViewport3.SetView(0.5, 0, 0.5, 45);

    canvas.join();
    t.join();

    return 0;
}
