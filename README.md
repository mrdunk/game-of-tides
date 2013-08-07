game-of-tides
=============

2d openGL experiment.
It's not a game yet, but getting there.
This should build on any modern Linux system.

Dependencies:
  On my test machine the only dependencies i needed were freeglut:
  $ sudo apt-get install freeglut3 freeglut3-dev g++ libglew-dev
  
  Plese let me know if you require anything else on your system.
  
Build:
  $ cd $MY_WORKING_DIR
  $ unzip ./master.zip
  $ cd ./game-of-tides-master/
  $ make
  
Instructions:
  At time of writing there are 6 keys that do something:
    Cursor-Up       Pan up
    Cursor-Down     Pan down
    Cursor-Left     Pan left
    Cursor-Right    Pan right
    -               Zoom out
    =               Zoom in

