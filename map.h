#ifndef MAP_H
#define MAP_H

#include "viewport.h"
#include "data.h"

#define TASK_TYPE_ZOOM 1
#define TASK_TYPE_PAN  2

struct Task{
    int type;
    float x0;
    float y0;
    float x1;
    float y1;
    float view_x;
    float view_y;
    float zoom;
    unsigned int progress_x;
    unsigned int progress_y;
};

class Map : public Viewport{
    private:
        static Data data;

        /* _task_list is for queuing screen segments to be drawn when time allows. */
        std::vector<Task> _task_list;
    public:
        Map(unsigned int label, int pos_x, int pos_y, int width, int height) : Viewport(label, pos_x, pos_y, width, height){
            _task_list.clear();
        };
        void Draw(void);
        bool DrawSection(Task*);
        bool DrawSection(float x0, float y0, float x1, float y1, unsigned int* p_progress_x=0, unsigned int* p_progress_y=0);
        void ScrubView(void);
        void ScrubView(float x0, float y0, float x1, float y1);
        void ActOnSignal(signal sig);
        void ProcessTasks(void);
        GLubyte WaterCol(float height);
};


#endif // MAP_H
