#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <vector>
#include <GL/gl.h>

#include "signaling.h"

#define MAX_WINDOWS 10
#define FRAME_LENGTH 100 // ms
#define BORDERWIDTH 2

struct Window{
    int window;
    int window_border;
    int pos_x;
    int pos_y;
    int width; 
    int height;
    bool initialised;
    bool dirty;
    float view_x;
    float view_y;
    float zoom;
    int rotation;
    std::vector<GLfloat>* _p_data_points;
    std::vector<GLubyte>* _p_data_colour;
};

extern Window windows[MAX_WINDOWS];

bool Init(int pos_x, int pos_y, int width, int height, int argc, char** argv);
void _init(void);
void _init(int window_index);
void setView(int window_index, float view_x, float view_y, float zoom, int rotation);
void Resize(int win_width, int win_height);
void Display(void);
void displayBorder(void);
void click(int button, int state, int x, int y);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void keyboardSecial(int key, int x, int y);
void refreshChildWindows(void);
void renderBitmapString(float x, float y, void *font, char *string);

class Viewport : public Signal{
    protected:
        int _window_index;
        int _pos_x;
        int _pos_y;
        int _width;
        int _height;
        float _view_x;
        float _view_y;
        float _zoom;
        int _rotation;
        unsigned int _label;
    public:
        std::vector<GLfloat> _data_points;
        std::vector<GLubyte> _data_colour;
        Viewport(unsigned int label, int pos_x, int pos_y, int width, int height);
        void SetView(float view_x, float view_y, float zoom, int rotation);
        virtual void Draw(void);
        virtual void ActOnSignal(signal sig);
        void Clear();
};

enum
{
    POSITION_OBJECT = 0,
    COLOR_OBJECT = 1
};
#endif //VIEWPORT_H
