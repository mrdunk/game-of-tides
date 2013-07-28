#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <vector>
#include <map>
#include <GL/gl.h>

#include "signaling.h"

#define MAX_WINDOWS 10
#define FRAME_LENGTH 100 // ms
#define BORDERWIDTH 2

enum
{
    POSITION_OBJECT = 0,
    COLOR_OBJECT = 1
};

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

    /* Contains the openGL data for the background. */
    std::vector<GLfloat>* _p_data_points;
    std::vector<GLubyte>* _p_data_colour;

    /* Contains the openGL data for the Low Reslution background. (if low_res > 0) */
    int low_res;
    std::vector<GLfloat>* _p_data_points_low_res;
    std::vector<GLubyte>* _p_data_colour_low_res;

    /* Contains the openGL data for the Icon layer. */
    std::vector<GLfloat>* _p_data_points_icons;
    std::vector<GLubyte>* _p_data_colour_icons;
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

#define ICON_TYPE_TEST      1
#define ICON_TYPE_VESSEL    2
struct Icon_key{
    unsigned int type;
    unsigned int key;

    bool operator<( const Icon_key & n ) const {
        if(this->type != n.type)
            return (this->type < n.type);
        return (this->key < n.key);
    }
};

struct Icon {
    unsigned int key;               // Key should be unique for this ICON_TYPE_XXX. 
    float angle;                    // in degrees.
    float scale;
    bool fixed_size;                // Stay the same size on screen as we zoom in.
    float centre_x;                 // Point in Icon data to rotate arround and act as centre for poitioning.
    float centre_y;                 // Point in Icon data to rotate arround and act as centre for poitioning.
    float pos_x;                    // Position in Viewport to place Icon.
    float pos_y;                    // Position in Viewport to place Icon.
    std::vector<GLfloat> points;
    std::vector<GLubyte> colour;
};



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

        /* Set resolution of background image to display while main image is being calculated.
         * if _low_res == 0, background image is not displayed. */
        int _low_res;

        std::vector<GLfloat> _data_points;
        std::vector<GLubyte> _data_colour;

        std::vector<GLfloat> _data_points_icons;
        std::vector<GLubyte> _data_colour_icons;

        std::map<Icon_key, Icon> _icons;
    public:
        void RedrawIcons(void);
        void AddIcon(Icon_key key, Icon icon);

        Viewport(unsigned int label, int pos_x, int pos_y, int width, int height);
        void SetView(float view_x, float view_y, float zoom, int rotation);
        virtual void Draw(void);
        virtual void ActOnSignal(signal sig);
        void Clear();

        Icon TestIcon(void);
};


#endif //VIEWPORT_H
