#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "viewport.h"
//#include "signaling.h"

using namespace std;

#define GL_BUFFERSIZE 2
static GLuint BufferName[GL_BUFFERSIZE];

Window windows[MAX_WINDOWS];

/* Instance of Sihnal handler for the non C++ openGL code to use. */
Signal Signal_instance;

bool Init(int pos_x, int pos_y, int width, int height, int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize (width, height);
    glutInitWindowPosition (pos_x, pos_y);

    windows[0] = {windows[0].window = 0, windows[0].window_border = 0,
                  windows[0].pos_x = pos_x, windows[0].pos_y = pos_y, 
                  windows[0].width = width, windows[0].height = height, windows[0].initialised = 0,
                  windows[0].dirty = 0, windows[0].view_x = 0, windows[0].view_y = 0, 
                  windows[0].zoom = 1, windows[0].rotation = 0};

    windows[0].window = glutCreateWindow ("Tides");
    GLenum err=glewInit();
    if(err!=GLEW_OK)
    {
        //Problem: glewInit failed, something is seriously wrong.
        cout<<"glewInit failed, aborting."<<endl;
        return 1;
    }

    // Make sure depth testing and lighting are disabled for 2D rendering
    glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );

    // Disable lighting
    glDisable( GL_LIGHTING );

    // Disable dithering
    glDisable( GL_DITHER );

    // Disable blending (for now)
    glDisable( GL_BLEND );

    // Disable depth testing
    glDisable( GL_DEPTH_TEST );

    // only draw front faces. 
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glGenBuffers(GL_BUFFERSIZE, BufferName);

    glutDisplayFunc(Display);
    glutReshapeFunc(Resize);
    glutMouseFunc(click);
    glutTimerFunc(FRAME_LENGTH, timer, 1);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardSecial);
    _init();

    windows[0].initialised = 1;

    glutMainLoop();

    return 0;
}

void _init(void){
    int window_index = 0;
    while(window_index < MAX_WINDOWS){
        //cout << " ** " << window_index << " " << windows[window_index].window << " " << glutGetWindow() << "\n";
        if (windows[window_index].window == glutGetWindow())
            break;
        ++window_index;
    }
    if(window_index <= MAX_WINDOWS){
        cout << "window_index: " << window_index << "\n";
        _init(window_index);
    }
    else
        cout << "Invalid window_index for  " << glutGetWindow() << "\n";
}

void _init(int window_index){
    cout << "_init(" << window_index << ")\n";
    setView(window_index, windows[window_index].view_x, windows[window_index].view_y, windows[window_index].zoom, windows[window_index].rotation);
}

void setView(int window_index, float view_x, float view_y, float zoom, int rotation){
    float scale_x = 1.0f, scale_y = 1.0f;
    if(windows[window_index].width == windows[window_index].height){
    } else if(windows[window_index].width > windows[window_index].height){
        scale_x = (float)windows[window_index].width / windows[window_index].height;
    } else {
        scale_y = (float)windows[window_index].height / windows[window_index].width;
    }

    if(windows[window_index].window_border){
        glutSetWindow(windows[window_index].window_border);
        glClearColor (1.0, 1.0, 1.0, 1.0);
    }
    glutSetWindow(windows[window_index].window);
    glClearColor (0.0, 0.0, 0.0, 0.0);

    /* initialize viewing values */
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();         // add copy of curent matrix to top of stack and push existing one down by 1.
    glLoadIdentity();       // load default matrix

    glMatrixMode( GL_MODELVIEW );
    //glPushMatrix();
    glLoadIdentity();
    glScalef(zoom, zoom, 1);
    gluOrtho2D(0.5f - (0.5f * scale_x), 0.5f + (0.5f * scale_x), 0.5f - (0.5f * scale_y), 0.5f + (0.5f * scale_y));
    glTranslatef(0.5, 0.5, 0.0);
    glRotatef(-rotation, 0.0, 0.0, 1.0);
    glTranslatef(-0.5, -0.5, 0.0);

    glTranslatef(view_x, view_y, 0.0);
}

void Resize(int win_width, int win_height){
    cout<<"win_width:  "<<win_width<<"\n";
    cout<<"win_height: "<<win_height<<"\n";
    glutSetWindow(windows[0].window);
    glViewport(0, 0, win_width, win_height);
    glutPostRedisplay();
}

void Display(void){
    //cout << "display " << glutGetWindow() << "\n";
    
    int window_index = 0;
    while(window_index < MAX_WINDOWS){
        //cout << " ** " << window_index << " " << windows[window_index].window << " " << glutGetWindow() << "\n";
        if (windows[window_index].window == glutGetWindow())
            break;
        ++window_index;
    }

    /* clear all pixels */
    glClear (GL_COLOR_BUFFER_BIT);

    if(!windows[window_index]._p_data_colour)
        return;

    const GLsizeiptr PositionSize = windows[window_index]._p_data_points->size() * sizeof(GLfloat);
    const int TriangleCount = windows[window_index]._p_data_points->size() / 2;
    const GLsizeiptr ColorSize = windows[window_index]._p_data_colour->size() * sizeof(GLubyte);

    //cout << "** " << window_index << " " << PositionSize<< "\t" << ColorSize << "\n";

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[COLOR_OBJECT]);
    glBufferData(GL_ARRAY_BUFFER, ColorSize, &(windows[window_index]._p_data_colour->front()), GL_STREAM_DRAW);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[POSITION_OBJECT]);
    glBufferData(GL_ARRAY_BUFFER, PositionSize, &(windows[window_index]._p_data_points->front()), GL_STREAM_DRAW);
    glVertexPointer(2, GL_FLOAT, 0, 0);


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_TRIANGLES, 0, TriangleCount);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    //glFlush ();
    glutSwapBuffers();
}

void displayBorder(void){
    glClear (GL_COLOR_BUFFER_BIT);
}

void click(int button, int state, int x, int y){
    /* http://pages.cpsc.ucalgary.ca/~tfalders/CPSC453/GLUT.html */
    //cout<<"window: " << glutGetWindow() << "\tbutton:  "<<button<<"\tstate: "<<state<<"\tx: "<<x<<"  y: "<<y<<"\n";

    signal sig = {sig.type = SIG_TYPE_MOUSE_BUT, sig.source = glutGetWindow(), sig.key = button, sig.val = state};
    Signal_instance.PushEvent(sig);
}

void timer(int value){
    //cout<<"timer "<<value<<"\n";
    glutTimerFunc(FRAME_LENGTH, timer, 1);
    //refresh();
    refreshChildWindows();
    //Signal_instance.SwapBuf();
}

void keyboard(unsigned char key, int x, int y){
    cout << "keyboard " << (int)key << " " << x << "," << y << "\n";
    //if(key == ' ')
    //    timer(0);
    if(key == '='){
        signal keypress;
        keypress.type = SIG_TYPE_KEYBOARD;
        keypress.source = 1;
        keypress.dest = SIG_DEST_MAP;
        keypress.val = SIG_VAL_ZOOM_IN;
        Signal_instance.PushEvent(keypress);
    }
    if(key == '-'){
        signal keypress;
        keypress.type = SIG_TYPE_KEYBOARD;
        keypress.source = 1;
        keypress.dest = SIG_DEST_MAP;
        keypress.val = SIG_VAL_ZOOM_OUT;
        Signal_instance.PushEvent(keypress);
    }
}

void keyboardSecial(int key, int x, int y){
    signal keypress;
    keypress.type = SIG_TYPE_KEYBOARD;
    keypress.source = 1;
    keypress.dest = SIG_DEST_MAP;
    keypress.val = 0;
    
    switch(key) {
        case GLUT_KEY_LEFT :
            keypress.val = SIG_VAL_LEFT;
            break;
        case GLUT_KEY_RIGHT :
            keypress.val = SIG_VAL_RIGHT;
            break;
        case GLUT_KEY_UP :
            keypress.val = SIG_VAL_UP;
            break;
        case GLUT_KEY_DOWN :
            keypress.val = SIG_VAL_DOWN;
            break;
    }
    Signal_instance.PushEvent(keypress);    

}

void refreshChildWindows(void){
    glutSetWindow(windows[0].window);
    int i = 1;
    while(i <= MAX_WINDOWS){
        if(!windows[i].window and windows[i].dirty){
                cout << " Creating window " << i << "\n" << flush;
                windows[i].window_border = glutCreateSubWindow(windows[0].window, windows[i].pos_x, windows[i].pos_y, 
                                                               windows[i].width + 2*BORDERWIDTH, windows[i].height + 2*BORDERWIDTH);
                glutDisplayFunc(displayBorder);
                windows[i].window = glutCreateSubWindow(windows[0].window, windows[i].pos_x + BORDERWIDTH, windows[i].pos_y + BORDERWIDTH, 
                                                        windows[i].width, windows[i].height);
                glutMouseFunc(click);
                glutKeyboardFunc(keyboard);
                glutSpecialFunc(keyboardSecial);
                glutDisplayFunc(Display);
                windows[i].initialised = 1;
        }
        if(windows[i].dirty){
                cout << " Updating window " << i << "\n" << flush;
                _init(i);
                windows[i].dirty = 0;
        }
        if(windows[i].window){
            //cout << " * " << i << " " << windows[i]._p_data_points->size() << "\t" << windows[i]._p_data_colour->size() << "\n";
            glutSetWindow(windows[i].window);
            glutPostRedisplay();
        }
        ++i;
    }
}

Viewport::Viewport(unsigned int label, int pos_x, int pos_y, int width, int height){
    cout << "Viewport::Viewport\n" << flush;
    _label = label;
    RegisterEndpoint(_label, this);
    int i = 0;
    /* Wait for Parent Window to be initialised. */
    while(!windows[0].initialised or !windows[0].window);
    glutSetWindow(windows[0].window);
    while(i < MAX_WINDOWS){
        if(windows[i].window == 0 and windows[i].initialised == 0 and windows[i].dirty == 0){
            cout << "Viewport::Viewport 3: " << i << "\n" << flush;
            windows[i] = {windows[i].window = 0, windows[i].window_border = 0,
                          windows[i].pos_x = pos_x, windows[i].pos_y = pos_y, 
                          windows[i].width = width - 2*BORDERWIDTH, windows[i].height = height - 2*BORDERWIDTH, 
                          windows[i].initialised = 0, windows[i].dirty = 1,
                          windows[i].view_x = 0, windows[i].view_y = 0,
                          windows[i].zoom = 1, windows[i].rotation = 0,
                          windows[i]._p_data_points = &_data_points, windows[i]._p_data_colour = &_data_colour};
            _window_index = i;
            _pos_x = pos_x;
            _pos_y = pos_y;
            _width = width;
            _height = height;
            _view_x = 0;
            _view_y = 0;
            _zoom = 1;
            _rotation = 0;

            Draw();
            break;
        }
        ++i;
    }
    cout << "Viewport::Viewport -\n" << flush;
}

void Viewport::SetView(float view_x, float view_y, float zoom, int rotation){
    cout << "Viewport::SetView " << _window_index << "\n" << flush;
    _view_x = view_x;
    _view_y = view_y;
    _zoom = zoom;
    _rotation = rotation;
    windows[_window_index].view_x = view_x;
    windows[_window_index].view_y = view_y;
    windows[_window_index].zoom = zoom;
    windows[_window_index].rotation = rotation;
    windows[_window_index].dirty = 1;
}

void Viewport::Draw(void){
    int size;
    if(windows[_window_index].height > windows[_window_index].width)
        size = windows[_window_index].width / 20;
    else
        size = windows[_window_index].height / 20;
    //size = 4;
    cout << "size: " << _window_index << " " << size << "\n";
    size = abs(size);
    int rows = (int)size, cols = (int)size;
    //int rows = 8, cols = 8;
    _data_points.clear();
    _data_colour.clear();
    float row, col;
    for(row = 0; row < 1; row += 1.0f/rows){
        for(col = 0; col < 1; col += 1.0f/cols){
            //cout << col << "\t" << row << "\t" << col + 1.0f/num_squares << "\t" << row + 1.0f/num_squares << "\n";
            _data_points.push_back (col);
            _data_points.push_back (row);
            _data_points.push_back (col + 1.0f/cols);
            _data_points.push_back (row);
            _data_points.push_back (col + 1.0f/cols);
            _data_points.push_back (row + 1.0f/rows);
            _data_points.push_back (col + 1.0f/cols);
            _data_points.push_back (row + 1.0f/rows);
            _data_points.push_back (col);
            _data_points.push_back (row + 1.0f/rows);
            _data_points.push_back (col);
            _data_points.push_back (row);

            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (128);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
            _data_colour.push_back (255);
        }
    }
    //cout << "\n";
    _data_points.push_back (0.5f);
    _data_points.push_back (1.0f);
    _data_points.push_back (0.6f);
    _data_points.push_back (0.0f);
    _data_points.push_back (0.4f);
    _data_points.push_back (0.0f);

    _data_colour.push_back (255);
    _data_colour.push_back (0);
    _data_colour.push_back (0);
    _data_colour.push_back (255);
    _data_colour.push_back (0);
    _data_colour.push_back (0);
    _data_colour.push_back (255);
    _data_colour.push_back (0);
    _data_colour.push_back (0);

}

void Viewport::Clear(void){
    _data_points.clear();
    _data_colour.clear();
}

void Viewport::DrawSection(float x0, float y0, float x1, float y1){
    cout << "Viewport::DrawSection  ***\n";
    Draw();
}

void Viewport::ActOnSignal(signal sig){
    //cout << "Viewport::ActOnSignal " << "\tsig.source: " << sig.source << "\tsig.dest: " << 
    //sig.dest << "\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
    if(sig.type == SIG_TYPE_KEYBOARD){
        float x0 = 0.0f, x1 = 1.0f, y0 = 0.0f, y1 = 1.0f;
        switch(sig.val){
            case SIG_VAL_ZOOM_IN:
                _zoom *= 1.5;
                Clear();
                break;
            case SIG_VAL_ZOOM_OUT:
                _zoom /= 1.5;
                Clear();
                break;
            case SIG_VAL_UP:
                _view_y -= 0.1 / _zoom;
                y0 = 0.9f;
                break;
            case SIG_VAL_DOWN:
                _view_y += 0.1 / _zoom;
                y1 = 0.1f;
                break;
            case SIG_VAL_LEFT:
                _view_x += 0.1 / _zoom;
                x1 = 0.1f;
                break;
            case SIG_VAL_RIGHT:
                _view_x -= 0.1 / _zoom;
                x0 = 0.9f;
                break;
        }
        SetView(_view_x, _view_y, _zoom, _rotation);
        //Draw();
        DrawSection(x0, y0,  x1, y1);
    }
}
