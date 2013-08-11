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
#include <math.h>       /* sin */
#include <algorithm>    // std::max

#define PI 3.14159265

#include "viewport.h"
#include "data.h"

using namespace std;

#define GL_BUFFERSIZE 2
static GLuint BufferName[GL_BUFFERSIZE];

Window windows[MAX_WINDOWS];

/* Instance of Signal handler for the non C++ openGL code to use. */
Signal Signal_instance;

int* p_shutdown_gl;

bool Init(unsigned int pos_x, unsigned int pos_y, unsigned int width, unsigned int height, int* _shutdown, int argc, char** argv){
    p_shutdown_gl = _shutdown;
    glutInit(&argc, argv);
    //glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);         // glFlush ();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);            // glutSwapBuffers();

    glutInitWindowSize (width, height);
    glutInitWindowPosition (pos_x, pos_y);

    windows[0].window = 0; 
    windows[0].window_border = 0;
    windows[0].pos_x = pos_x;
    windows[0].pos_y = pos_y; 
    windows[0].width = width; 
    windows[0].height = height;
    windows[0].initialised = 0;
    windows[0].dirty = 0;
    windows[0].data_size = max(width,height);
    windows[0].view_x = 0;
    windows[0].view_y = 0;
    windows[0].zoom = 1;
    windows[0].rotation = 0;

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
    glutCloseFunc(_close);
    _init();

    windows[0].initialised = 1;

    glutMainLoop();

    /* will never get here. */
    ++(*_shutdown);
    return 0;
}

void _close(void){
    cout << "close\n";
    ++(*p_shutdown_gl);      // Set other threads to terminate.
    while(*p_shutdown_gl < ACTIVE_THREADS);
    cout << "close 2\n";
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
    //cout << "_init(" << window_index << ")\n";
    setView(window_index, windows[window_index].view_x, windows[window_index].view_y, windows[window_index].zoom, windows[window_index].rotation);
}

void setView(int window_index, int view_x, int view_y, float zoom, int rotation){
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
    gluOrtho2D(0, windows[window_index].data_size * scale_x, 0, windows[window_index].data_size * scale_y); 
              
    int w = windows[window_index].data_size * scale_x / 2, h = windows[window_index].data_size * scale_y /2;
    glTranslated(w, h, 0.0);
    glRotatef(-rotation, 0.0, 0.0, 1.0);
    glTranslated(-min(w,h), -min(w,h), 0.0);        // use min(w,h) here to centre data on the square part of a rectangle.

    glTranslated(-view_x, -view_y, 0.0);
}

void Resize(int win_width, int win_height){
    cout<<"win_width:  "<<win_width<<"\n";
    cout<<"win_height: "<<win_height<<"\n";
    glutSetWindow(windows[0].window);
    glViewport(0, 0, win_width, win_height);
    glutPostRedisplay();
}

int LoadVertices(int window_index, std::mutex* data_mutex, std::vector<GLint>* p_points, std::vector<GLubyte>* p_colour){
    data_mutex->lock();
    
    int VertexCount = p_points->size() / 2;
    //PositionSize = p_points->size() * sizeof(GLint);
    //ColorSize = p_colour->size() * sizeof(GLubyte);
    VertexCount -= VertexCount % 3;
    GLsizeiptr PositionSize = VertexCount * 2 * sizeof(GLint);
    GLsizeiptr ColorSize = VertexCount * 3 * sizeof(GLubyte);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[COLOR_OBJECT]);
    glBufferData(GL_ARRAY_BUFFER, ColorSize, &(p_colour->front()), GL_DYNAMIC_DRAW);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[POSITION_OBJECT]);
    glBufferData(GL_ARRAY_BUFFER, PositionSize, &(p_points->front()), GL_DYNAMIC_DRAW);
    glVertexPointer(2, GL_INT, 0, 0);


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glDrawArrays(GL_TRIANGLES, 0, VertexCount);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    
    data_mutex->unlock();

    return VertexCount;
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

    int VertexCount;

    /* Do low resolution draw in background if enabled. */
    if(windows[window_index].low_res){
        VertexCount = LoadVertices(window_index, &windows[window_index].data_low_res_mutex, 
                               windows[window_index]._p_data_points_low_res, windows[window_index]._p_data_colour_low_res);
        //cout << "low\n";
    }

    /* Standard esolution. */
    VertexCount = LoadVertices(window_index, &windows[window_index].data_mutex, windows[window_index]._p_data_points, windows[window_index]._p_data_colour);

    /* Text output */
    glPushMatrix();
    glLoadIdentity();       // load default matrix
    gluOrtho2D(0, windows[window_index].width, 0, windows[window_index].height);

    char Result[64];
    sprintf ( Result, "Triangles: %d", VertexCount / 3 );
    renderBitmapString(10, 10, GLUT_BITMAP_TIMES_ROMAN_10, Result);

    int pixSize = MAX_SIZE / (windows[window_index].width * windows[window_index].zoom * MIN_RECURSION);
    sprintf ( Result, "PixelSize: %d m", pixSize );
    //glColor3f(0, 0, 1);
    renderBitmapString(10, 20, GLUT_BITMAP_TIMES_ROMAN_10, Result);

    sprintf ( Result, "_zoom: %lf", windows[window_index].zoom);
    renderBitmapString(10, 30, GLUT_BITMAP_TIMES_ROMAN_10, Result);

    glPopMatrix();

    /* Draw Icon buffer. */
    VertexCount = LoadVertices(window_index, &windows[window_index].data_icons_mutex, 
                               windows[window_index]._p_data_points_icons, windows[window_index]._p_data_colour_icons);

    //glFlush ();
    glutSwapBuffers();
}

void displayBorder(void){
    glClear (GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void click(int button, int state, int x, int y){
    /* http://pages.cpsc.ucalgary.ca/~tfalders/CPSC453/GLUT.html */

    int window_index = 0;
    while(window_index < MAX_WINDOWS){
        if ( windows[window_index].window == glutGetWindow())
            break;
        ++window_index;
    }

    cout<<"window: " << window_index << "\tbutton:  "<<button<<"\tstate: "<<state<<"\tx: "<<x<<"  y: "<<y<<"\n";

    signal sig = {sig.type = SIG_TYPE_MOUSE_BUT, sig.source = window_index, sig.key = button, sig.val = state};
    Signal_instance.PushEvent(sig);

    windows[window_index].mouse_button = button;
}

void mouse_move(int x, int y){
    int window_index = 0;
    while(window_index < MAX_WINDOWS){
        if ( windows[window_index].window == glutGetWindow())
            break;
        ++window_index;
    }

    //cout << window_index << "\t" << x << "," << y << "\n";
    windows[window_index].mouse_x = x;
    windows[window_index].mouse_y = y;
}

void mouse_enter_window(int state){
    int window_index = 0;
    while(window_index < MAX_WINDOWS){
        if ( windows[window_index].window == glutGetWindow())
            break;
        ++window_index;
    }

    if(state == GLUT_LEFT){
        windows[window_index].mouse_x = -1;
        windows[window_index].mouse_y = -1;
    }
}

void timer(int value){
    //cout<<"timer "<<value<<"\n";
    glutTimerFunc(FRAME_LENGTH, timer, 1);
    refreshChildWindows();
    static int count = 0;
    if(++count == 10){
        Signal_instance.SwapBuf();
        count = 0;
    }
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
                glutMotionFunc(mouse_move);
                glutPassiveMotionFunc(mouse_move);
                glutEntryFunc(mouse_enter_window);
                glutKeyboardFunc(keyboard);
                glutSpecialFunc(keyboardSecial);
                glutDisplayFunc(Display);
                windows[i].initialised = 1;
        }
        if(windows[i].dirty){
                //cout << " Updating window " << i << "\n" << flush;
                _init(i);
                windows[i].dirty = 0;
        }
        if(windows[i].window){
            //cout << " * " << i << " " << windows[i]._p_data_points->size() << "\t" << windows[i]._p_data_colour->size() << "\n";
            //glutSetWindow(windows[i].window_border);
            //glutPostRedisplay();
            glutSetWindow(windows[i].window);
            glutPostRedisplay();
        }
        ++i;
    }
}

void renderBitmapString(unsigned int x, unsigned int y, void *font, char *string) {

    char *c;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

Viewport::Viewport(unsigned int label, unsigned int pos_x, unsigned int pos_y, unsigned int width, unsigned int height){
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
            windows[i].window = 0;
            windows[i].window_border = 0;
            windows[i].pos_x = pos_x;
            windows[i].pos_y = pos_y;
            windows[i].width = width - 2*BORDERWIDTH;
            windows[i].height = height - 2*BORDERWIDTH;
            windows[i].initialised = 0;
            windows[i].dirty = 1;
            windows[i].data_size = 200;
            windows[i].view_x = 0;
            windows[i].view_y = 0;
            windows[i].zoom = 1;
            windows[i].rotation = 0;
            windows[i]._p_data_points = &_data_points;
            windows[i]._p_data_colour = &_data_colour;
            windows[i]._p_data_points_icons = &_data_points_icons;
            windows[i]._p_data_colour_icons = &_data_colour_icons;
            windows[i].low_res = 0;
            windows[i].mouse_x = -1;
            windows[i].mouse_y = -1;
            _window_index = i;
            _pos_x = pos_x;
            _pos_y = pos_y;
            _width = width;
            _height = height;
            _data_size = 200;
            _view_x = 0;
            _view_y = 0;
            _zoom = 1;
            _rotation = 0;
            _low_res = 0;

            Draw();
            break;
        }
        ++i;
    }
    cout << "Viewport::Viewport -\n" << flush;
}

void Viewport::SetView(int view_x, int view_y, float zoom, int rotation){
    //cout << "Viewport::SetView " << _window_index << "\n" << flush;
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
    int SQUARE_SIZE = 20;
    windows[_window_index].data_size = _data_size = 10 * SQUARE_SIZE;

    _data_points.clear();
    _data_colour.clear();
    
    unsigned int row, col;
    for(row = 0; row < _data_size; row += SQUARE_SIZE){
        for(col = 0; col < _data_size; col += SQUARE_SIZE){
            //cout << col << "\t" << row << "\t" << col + 1.0f/num_squares << "\t" << row + 1.0f/num_squares << "\n";
            _data_points.push_back (col);
            _data_points.push_back (row);
            _data_points.push_back (col + SQUARE_SIZE);
            _data_points.push_back (row);
            _data_points.push_back (col + SQUARE_SIZE);
            _data_points.push_back (row + SQUARE_SIZE);
            _data_points.push_back (col + SQUARE_SIZE);
            _data_points.push_back (row + SQUARE_SIZE);
            _data_points.push_back (col);
            _data_points.push_back (row + SQUARE_SIZE);
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
}

void Viewport::Clear(void){
    _data_points.clear();
    _data_colour.clear();
}

void Viewport::ActOnSignal(signal sig){
    //cout << "Viewport::ActOnSignal " << "\tsig.source: " << sig.source << "\tsig.dest: " << 
    //sig.dest << "\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
}

void Viewport::AddIcon(Icon_key key, Icon icon){
    //cout << "Viewport::AddIcon         type: " << key.type << "\tkey: " << key.key << "\tpos_x: " << icon.pos_x << "\tpos_y: " << icon.pos_y << "\n";
    //_icons.insert(pair<Icon_key,Icon>(key,icon));
    _icons[key] = icon;
}

void Viewport::RedrawIcons(void){
    //cout << "Viewport::RedrawIcons\n";
    float scale;
    _data_points_icons.clear();
    _data_colour_icons.clear();
    for (std::map<Icon_key,Icon>::iterator it=_icons.begin(); it!=_icons.end(); ++it){
        //cout << "Viewport::RedrawIcons type: " << it->first.type << "\tkey: " << it->first.key << "\tpos_x: " << it->second.pos_x << "\tpos_y: " << it->second.pos_y << "\n";
        scale = it->second.scale;
        if (it->second.fixed_size == 1)
            scale /= _zoom;
        for(unsigned int p = 0; p < it->second.points.size() / 2; ++p){
            float x = (it->second.points.at(p * 2   ) - it->second.centre_x) * scale;
            float y = (it->second.points.at(p * 2 +1) - it->second.centre_y) * scale;
            _data_points_icons.push_back(cos(-it->second.angle * PI / 180) * x - sin(-it->second.angle * PI / 180) * y + it->second.pos_x);
            _data_points_icons.push_back(sin(-it->second.angle * PI / 180) * x + cos(-it->second.angle * PI / 180) * y + it->second.pos_y);
            _data_colour_icons.push_back(it->second.colour.at(p * 3   ));
            _data_colour_icons.push_back(it->second.colour.at(p * 3 +1));
            _data_colour_icons.push_back(it->second.colour.at(p * 3 +2));
        }
    }
}

Icon Viewport::TestIcon(void){
    Icon icon;
    icon.points.push_back (5);
    icon.points.push_back (10);
    icon.points.push_back (6);
    icon.points.push_back (0);
    icon.points.push_back (4);
    icon.points.push_back (0);

    icon.colour.push_back (255);
    icon.colour.push_back (0);
    icon.colour.push_back (0);
    icon.colour.push_back (255);
    icon.colour.push_back (0);
    icon.colour.push_back (0);
    icon.colour.push_back (255);
    icon.colour.push_back (0);
    icon.colour.push_back (0);

    icon.angle = 0.0f;
    icon.scale = 10;
    icon.fixed_size = 0;
    icon.centre_x = 5;
    icon.centre_y = 5;
    icon.pos_x = 50;
    icon.pos_y = 50;

    return icon;
}
