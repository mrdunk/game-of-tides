#ifndef MAP_H
#define MAP_H

#include "viewport.h"
#include "data.h"
#include "vessel.h"

#define LOW_RESOLUTION 8 
#define TASK_TYPE_ZOOM 1
#define TASK_TYPE_PAN  2
#define KEY_MOVMENT 0.01f * MAX_SIZE

struct Task{
    int type;
    int x0;
    int y0;
    int x1;
    int y1;
    int view_x;
    int view_y;
    float zoom;
    int progress_x;
    int progress_y;
};

class Map : public Viewport{
    private:
        static Data data;

        /* _task_list is for queuing screen segments to be drawn when time allows. */
        std::vector<Task> _task_list;
        std::vector<Task> _task_list_low_res;

        std::vector<GLint> _data_points_low_res;
        std::vector<GLubyte> _data_colour_low_res;

        Fleet vessels;

    public:
        Map(unsigned int label, int pos_x, int pos_y, int width, int height, int low_res=0);
        void Draw(void);
        int DrawSection(Task*, int resolution);
        int DrawSection(int x0, int y0, int x1, int y1, int resolution, int* p_progress_x=0, int* p_progress_y=0);
        bool ScrubView(void);
        bool ScrubView(int x0, int y0, int x1, int y1);
        void ActOnSignal(signal sig);
        bool ProcessTasks(std::vector<Task>* p_task_list);
        GLubyte WaterCol(float height);
};


#endif // MAP_H
