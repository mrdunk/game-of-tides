#include <iostream>

#include "map.h"
#include "data.h"
#include "time_code.c"
#include <GL/glut.h>

using namespace std;

Data Map::data;

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << flush << "Map::Draw " << _window_index << " " << _width << "," << _height << "\n" << flush;
    timestamp_t t0 = get_timestamp();

    Clear();

    DrawSection(0.0f, 0.0f, 1.0f, 1.0f);

    timestamp_t t1 = get_timestamp();
    cout << "Map::Draw took " << (double)(t1 - t0) / 1000000.0L << " seconds.\n";
}

int Map::DrawSection(Task* p_task){
    float _zoom_holder = _zoom;
    float _view_x_holder = _view_x;
    float _view_y_holder = _view_y;
    _zoom = p_task->zoom;
    _view_x = p_task->view_x;
    _view_y = p_task->view_y;

    int retval = DrawSection(p_task->x0, p_task->y0, p_task->x1, p_task->y1, &(p_task->progress_x), &(p_task->progress_y));

    _zoom = _zoom_holder;
    _view_x = _view_x_holder;
    _view_y = _view_y_holder;

    return retval;
}

int Map::DrawSection(float x0, float y0, float x1, float y1, unsigned int* p_progress_x, unsigned int* p_progress_y){

    /* step size will always be a power of 2.
     * This is important to ensure the display pixels coincide with the data structure recursion levels
     * which means un-necesary recursion through the datastructure need not occur. */
    int step_x = MAX_SIZE / (int)pow(2, (int)log2(_width * _zoom));
    int step_y = MAX_SIZE / (int)pow(2, (int)log2(_height * _zoom));
    //step_x *= resolution;
    //step_y *= resolution;

    /* The (min_y % step_y) part ensures the display is alligned with the datastructure, preventing un-necesary recursion. */
    int min_x = ((0.5f - _view_x) * MAX_SIZE) - (((0.5f - x0) / _zoom) * MAX_SIZE);
    int min_y = ((0.5f - _view_y) * MAX_SIZE) - (((0.5f - y0) / _zoom) * MAX_SIZE);
    min_x = min_x - (min_x % step_x);
    min_y = min_y - (min_y % step_y);
    if( min_x < 0)
         min_x = 0;
    if( min_y < 0)
        min_y = 0;
    unsigned int max_x = ((0.5f - _view_x) * MAX_SIZE) - (((0.5f - x1) / _zoom) * MAX_SIZE);
    unsigned int max_y = ((0.5f - _view_y) * MAX_SIZE) - (((0.5f - y1) / _zoom) * MAX_SIZE);
    if (max_x > (unsigned int)MAX_SIZE)
        max_x = MAX_SIZE;
    if (max_y > (unsigned int)MAX_SIZE)
        max_y = MAX_SIZE;

    float z_multiplier_dry = 255.0f / (data.Height_z_max() - data.Waterlevel());

    MapPoint tl, tr, bl, br;

    //cout << "min_x: " << min_x << "\tmax_x: " << max_x << "\tmin_y: " << min_y << "\tmax_y: " << max_y << "\n";
    //cout << "step_x: " << step_x << "\tstep_y: " << step_y << "\n";
    for(unsigned int row = min_y; row < max_y; row+=step_y){
        if(p_progress_y and *p_progress_y > row){
            continue;
        }
        //if(TestInterupt(SIG_DEST_MAP)){
        //    cout << "INTERUPT!!\n";
        //        if(p_progress_y)
        //            *p_progress_y = row;
        //        return 1;
        //}
        for(unsigned int col = min_x; col < max_x; col+=step_x){
            if(p_progress_x and p_progress_y and *p_progress_x > col and *p_progress_y > row){
                continue;
            }
            int retval = TestInterupt(SIG_DEST_MAP);
            if(retval){
                cout << "INTERUPT!! " << retval << "\n";
                if(p_progress_x)
                    *p_progress_x = col;
                if(p_progress_y)
                    *p_progress_y = row;
                return retval;
            }

            //cout << col << "," << row << "\n" << flush;
            tl.x = col;
            tl.y = row;
            tl.calculateZ(data.p_mapData);
            tr.x = col + step_x;
            tr.y = row;
            tr.calculateZ(data.p_mapData);
            bl.x = col;
            bl.y = row + step_y;
            bl.calculateZ(data.p_mapData);
            br.x = col + step_x;
            br.y = row + step_y;
            br.calculateZ(data.p_mapData);

            _data_points.push_back ((float)tl.x / MAX_SIZE);
            _data_points.push_back ((float)tl.y / MAX_SIZE);
            _data_points.push_back ((float)bl.x / MAX_SIZE);
            _data_points.push_back ((float)bl.y / MAX_SIZE);
            _data_points.push_back ((float)tr.x / MAX_SIZE);
            _data_points.push_back ((float)tr.y / MAX_SIZE);

            _data_points.push_back ((float)tr.x / MAX_SIZE);
            _data_points.push_back ((float)tr.y / MAX_SIZE);
            _data_points.push_back ((float)bl.x / MAX_SIZE);
            _data_points.push_back ((float)bl.y / MAX_SIZE);
            _data_points.push_back ((float)br.x / MAX_SIZE);
            _data_points.push_back ((float)br.y / MAX_SIZE);

            _data_colour.push_back (0);
            if(tl.z > data.Waterlevel()){
                _data_colour.push_back ((float)(data.Height_z_max() - tl.z) * z_multiplier_dry);
                _data_colour.push_back (0);
            } else {
                _data_colour.push_back (0);
                _data_colour.push_back (WaterCol(bl.z));
            }
            _data_colour.push_back (0);
            if(bl.z > data.Waterlevel()){
                _data_colour.push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                _data_colour.push_back (0);
            } else {
                _data_colour.push_back (0);
                _data_colour.push_back (WaterCol(bl.z));
            }
            _data_colour.push_back (0);
            if(tr.z > data.Waterlevel()){
                _data_colour.push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                _data_colour.push_back (0);
            } else {        
                _data_colour.push_back (0);
                _data_colour.push_back (WaterCol(bl.z));
            }

            _data_colour.push_back (0);
            if(tr.z > data.Waterlevel()){
                _data_colour.push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                _data_colour.push_back (0);
            } else {        
                _data_colour.push_back (0); 
                _data_colour.push_back (WaterCol(bl.z));
            }
            _data_colour.push_back (0);
            if(bl.z > data.Waterlevel()){
                _data_colour.push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                _data_colour.push_back (0);
            } else {        
                _data_colour.push_back (0);
                _data_colour.push_back (WaterCol(bl.z));
            }
            _data_colour.push_back (0);
            if(br.z > data.Waterlevel()){
                _data_colour.push_back ((float)(data.Height_z_max() - br.z) * z_multiplier_dry);
                _data_colour.push_back (0);
            } else {        
                _data_colour.push_back (0);
                _data_colour.push_back (WaterCol(bl.z));
            }
        }
    }
    if(p_progress_x and p_progress_y){
        *p_progress_x = MAX_SIZE;
        *p_progress_y = MAX_SIZE;
    }
    return 0;
}

inline GLubyte Map::WaterCol(float height){
    const float z_multiplier_wet = _zoom * 255.0f / (data.Waterlevel() - data.Height_z_min());
    return (GLubyte)(255 - (float)(data.Waterlevel() - height) * z_multiplier_wet);
}

void Map::ScrubView(void){
    ScrubView(0.0f, 0.0f, 1.0f, 1.0f);
}

void Map::ScrubView(float x0, float y0, float x1, float y1){
    const float min_x = (0.5f - _view_x) - ((0.5f - x0) / _zoom);
    const float min_y = (0.5f - _view_y) - ((0.5f - y0) / _zoom);
    const float max_x = (0.5f - _view_x) - ((0.5f - x1) / _zoom);
    const float max_y = (0.5f - _view_y) - ((0.5f - y1) / _zoom);

    std::vector<GLfloat> _data_points_tmp;
    std::vector<GLubyte> _data_colour_tmp;

    std::vector<GLfloat>::iterator data_it = _data_points.begin();
    std::vector<GLubyte>::iterator colour_it = _data_colour.begin();

    float x[6], y[6], z[18];
    for (unsigned int i = 0; i < _data_points.size() / 12; ++i){
        for(unsigned int d = 0; d < 6; ++d){
            x[d] = *data_it;
            ++data_it;
            y[d] = *data_it;
            ++data_it;
        }
        for(unsigned int c = 0; c < 18; ++c){
            z[c] = *colour_it;
            ++colour_it;
        }

        if(x[0] >= min_x and x[2] <= max_x and y[0] >= min_y and y[1] <= max_y){
            for(unsigned int d = 0; d < 6; ++d){
                _data_points_tmp.push_back(x[d]);
                _data_points_tmp.push_back(y[d]);
            }
            for(unsigned int c = 0; c < 18; ++c){
                _data_colour_tmp.push_back(z[c]);
            }
        }
    }
    _data_points.swap(_data_points_tmp);
    _data_colour.swap(_data_colour_tmp);
}

void Map::ActOnSignal(signal sig){
    //cout << "Viewport::ActOnSignal\n";
    //cout << "Viewport::ActOnSignal " << "\tsig.source: " << sig.source << "\tsig.dest: " << 
    //sig.dest << "\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
    if(sig.type == SIG_TYPE_KEYBOARD){
        Task task;
        task.x0 = 0.0f;
        task.x1 = 1.0f;
        task.y0 = 0.0f;
        task.y1 = 1.0f;
        task.view_x = _view_x;
        task.view_y = _view_y;
        task.zoom = _zoom;
        task.progress_x = 0;
        task.progress_y = 0;


        switch(sig.val){
            case SIG_VAL_ZOOM_IN:
                _zoom *= 1.5;
                task.zoom = _zoom;
                task.type = TASK_TYPE_ZOOM;
                break;
            case SIG_VAL_ZOOM_OUT:
                _zoom /= 1.5;
                task.zoom = _zoom;
                task.type = TASK_TYPE_ZOOM;
                break;
            case SIG_VAL_UP:
                _view_y -= 0.1 / _zoom;
                task.view_y = _view_y;
                task.type = TASK_TYPE_PAN;
                task.y0 = 0.9f;
                break;
            case SIG_VAL_DOWN:
                _view_y += 0.1 / _zoom;
                task.view_y = _view_y;
                task.type = TASK_TYPE_PAN;
                task.y1 = 0.1f;
                break;
            case SIG_VAL_LEFT:
                _view_x += 0.1 / _zoom;
                task.view_x = _view_x;
                task.type = TASK_TYPE_PAN;
                task.x1 = 0.1f;
                break;
            case SIG_VAL_RIGHT:
                _view_x -= 0.1 / _zoom;
                task.view_x = _view_x;
                task.type = TASK_TYPE_PAN;
                task.x0 = 0.9f;
                break;
        }
        if(_zoom > 16000)
            task.zoom = _zoom = 16000;      // TODO base this on MIN_RECURSION
        SetView(_view_x, _view_y, _zoom, _rotation);
        _task_list.push_back (task);
    }
    cout << " _zoom: " << _zoom << "\n";
    ProcessTasks();
}

void Map::ProcessTasks(void){
    int interupt_val = 0;

    for(std::vector<Task>::iterator it = _task_list.begin() ; it != _task_list.end(); ++it){
        cout << "  type: " << it->type << "\tprogress_x: " << it->progress_x << "\tview_x,view_y: " << it->view_x << "," << it->view_y << "\n";
        if(it->type == TASK_TYPE_ZOOM){
            if(!it->progress_x and !it->progress_y)
                Clear();            // only do this if we are not continuing a previously started DrawSection().
            interupt_val = DrawSection(&(*it));
            // "< SIG_VAL_UP" covers zoom interrupt and no interrupt.
            if(interupt_val < SIG_VAL_UP){
                it->type = 0;       // only do this if next keypress will draw whole map.
            }
        } else if(it->type == TASK_TYPE_PAN){
            ScrubView();
            interupt_val = DrawSection(&(*it));
            if(interupt_val == 0){
                it->type = 0;
            }
        }
        if (interupt_val){
            return;
        }
    }
    while(_task_list.back().type == 0)
        _task_list.pop_back();

}



