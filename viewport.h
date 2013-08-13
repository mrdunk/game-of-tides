#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <vector>
#include <map>
#include <GL/gl.h>
#include <mutex>

#include "signaling.h"

#define MAX_WINDOWS 10
#define FRAME_LENGTH 100 // ms
#define BORDERWIDTH 2
#define ACTIVE_THREADS 4

enum
{
    POSITION_OBJECT = 0,
    COLOR_OBJECT = 1
};

struct Window{
    int window;                 // Glut window instance.
    int window_border;          // Thickness of border in pixels.
    unsigned int pos_x;         // Position of window in main window.
    unsigned int pos_y;
    unsigned int width;         // Size in pixels.
    unsigned int height;
    bool initialised;           // Set to 1 when initialisation is complete.
    bool dirty;                 // Will be re-drawn if this is set to 1.
    unsigned int data_size;     // Size of displayed data.
    int view_x;                 // position within the displayed data structure. (0,0) is the bottom left.
    int view_y;
    float zoom;                 // How far to zoom in/out.
    int rotation;               // Rotation about current view position.
    int mouse_x;                // position of mouse on window
    int mouse_y;
    int mouse_x_rel;            // position of mouse on window relative to the datastructure.
    int mouse_y_rel;
    int mouse_button;           // Mouse button state on window.

    /* Contains the openGL data for the background. */
    std::vector<GLint>* _p_data_points;
    std::vector<GLubyte>* _p_data_colour;
    std::mutex data_mutex;

    /* Contains the openGL data for the Low Reslution background. (if low_res > 0) */
    int low_res;
    std::vector<GLint>* _p_data_points_low_res;
    std::vector<GLubyte>* _p_data_colour_low_res;
    std::mutex data_low_res_mutex;

    /* Contains the openGL data for the Icon layer. */
    std::vector<GLint>* _p_data_points_icons;
    std::vector<GLubyte>* _p_data_colour_icons;
    std::mutex data_icons_mutex;
};

extern Window windows[MAX_WINDOWS];

/* If this has been incremented, one of the threads is shutting down so do the same to the openGL thread. */
extern int* p_shutdown_gl;

bool Init(unsigned int pos_x, unsigned int pos_y, unsigned int width, unsigned int height, int* shutdown, int argc, char** argv);

/* Initialise the currently active window. */
void _init(void);

/* Initialise the window reffered to by window_index. */
void _init(int window_index);

void setView(int window_index, int view_x, int view_y, float zoom, int rotation);
void Resize(int win_width, int win_height);
int LoadVertices(int window_index, std::mutex* data_mutex, std::vector<GLint>* p_points, std::vector<GLubyte>* _p_colour);
void Display(void);
void displayBorder(void);
void click(int button, int state, int x, int y);
void mouse_move(int x, int y);
void mouse_enter_window(int state);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void keyboardSecial(int key, int x, int y);
void _close(void);
void refreshChildWindows(void);
void renderBitmapString(unsigned int x, unsigned int y, void *font, char *string);

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
    int key;                        // Key should be unique for this ICON_TYPE_XXX. 
    float angle;                    // in degrees.
    float scale;
    bool fixed_size;                // Stay the same size on screen as we zoom in.
    int centre_x;                 // Point in Icon data to rotate arround and act as centre for poitioning.
    int centre_y;                 // Point in Icon data to rotate arround and act as centre for poitioning.
    int pos_x;                    // Position in Viewport to place Icon.
    int pos_y;                    // Position in Viewport to place Icon.
    std::vector<GLint> points;
    std::vector<GLubyte> colour;
};



class Viewport : public Signal{
    protected:
        int _window_index;
        unsigned int _pos_x;
        unsigned int _pos_y;
        unsigned int _width;
        unsigned int _height;
        unsigned int _data_size;
        int _view_x;
        int _view_y;
        float _zoom;
        int _rotation;
        unsigned int _label;

        /* Set resolution of background image to display while main image is being calculated.
         * if _low_res == 0, background image is not displayed. */
        int _low_res;

        std::vector<GLint> _data_points;
        std::vector<GLubyte> _data_colour;

        std::vector<GLint> _data_points_icons;
        std::vector<GLubyte> _data_colour_icons;

        std::map<Icon_key, Icon> _icons;
    public:
        void RedrawIcons(void);

        /* Add the Icon struct to the list to be drawn. 
         * Args: (Icon_key)key: A struct containing the (int)type (as defined by #define ICON_TYPE_XXX)
         *                      and a unique (int)key.
         *                      If another icon is added with a matching (Icon_key)key, the original is overwritten.
         *       (Icon)icon:    A struct containing the icon data.
         */
        void AddIcon(Icon_key key, Icon icon);

        Viewport(unsigned int label, unsigned int pos_x, unsigned int pos_y, unsigned int width, unsigned int height);
        void SetView(int view_x, int view_y, float zoom, int rotation);
        virtual void Draw(void);
        virtual void ActOnSignal(signal sig);
        void Clear();

        Icon TestIcon(void);
};


#endif //VIEWPORT_H
