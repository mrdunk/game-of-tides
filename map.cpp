#include <iostream>
#include <GL/glut.h>

#include "map.h"
#include "data.h"

using namespace std;

Data Map::data;

Map::Map(unsigned int label, int pos_x, int pos_y, int width, int height, int low_res) : Viewport(label, pos_x, pos_y, width, height){
    while(!_window_index); // wait until window is initialised.
    _task_list.clear();
    _task_list_low_res.clear();
    _low_res = low_res;
    windows[_window_index].low_res = 0;     // Set this to 0 so we don't start displaying low_res layer until there is data in it.
    windows[_window_index]._p_data_points_low_res = &_data_points_low_res;
    windows[_window_index]._p_data_colour_low_res = &_data_colour_low_res;

    RegisterEndpoint(SIG_DEST_ALL, this);
    
    DrawBoats();

    finish_time = get_timestamp() + 1000*1000;  // 1 second in the future.
}

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << flush << "Map::Draw " << _window_index << " " << _width << "," << _height << "\n" << flush;
    timestamp_t t0 = get_timestamp();

    windows[_window_index].data_size = _data_size = MAX_SIZE;

    Clear();

    Task initialise;

    initialise.type = TASK_TYPE_ZOOM;
    initialise.x0 = 0;
    initialise.y0 = 0;
    initialise.x1 = MAX_SIZE;
    initialise.y1 = MAX_SIZE;
    initialise.view_x = MAX_SIZE/2;
    initialise.view_y = MAX_SIZE/2;
    initialise.zoom = 1;
    initialise.progress_x = 0;
    initialise.progress_y = 0;

    _task_list.push_back (initialise);
    _task_list_low_res.push_back (initialise);
    //DrawSection(0, 0, MAX_SIZE, MAX_SIZE, 1);

    timestamp_t t1 = get_timestamp();
    cout << "Map::Draw took " << (long)(t1 - t0) / 1000000.0L << " seconds.\n";
}

int Map::DrawSection(Task* p_task, int resolution){

    /* Currently viewed bounding box. */
    int screen_x0, screen_y0, screen_x1, screen_y1;
    if(_width > _height){
	screen_x0 = _view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
	screen_x1 = _view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
        screen_y0 = _view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
        screen_y1 = _view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
    } else {
        screen_x0 = _view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
        screen_x1 = _view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
	screen_y0 = _view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_height / (long)_width);
        screen_y1 = _view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_height / (long)_width);
    }

    //cout << "screen_x0: " << screen_x0 << "\tscreen_x1: " << screen_x1 << "\tscreen_y0: " << screen_y0 << "\tscreen_y1: " << screen_y1 << "\n";

    if(p_task->x1 <= screen_x0 or p_task->x0 >= screen_x1 or p_task->y1 <= screen_y0 or p_task->y0 >= screen_y1){
        cout << "No overlap\n";
        return 0;
    }

    if(p_task->x0 < screen_x0)
        p_task->x0 = screen_x0;
    if(p_task->x1 > screen_x1)
        p_task->x1 = screen_x1;
    if(p_task->y0 < screen_y0)
        p_task->y0 = screen_y0;
    if(p_task->y1 > screen_y1)
        p_task->y1 = screen_y1;

    /* Save current view so we can inherit the ones contained in the task. */
    float _zoom_holder = _zoom;
    int _view_x_holder = _view_x;
    int _view_y_holder = _view_y;
    _zoom = p_task->zoom;
    _view_x = p_task->view_x;
    _view_y = p_task->view_y;

    int retval = DrawSection(p_task->x0, p_task->y0, p_task->x1, p_task->y1, resolution, &(p_task->progress_x), &(p_task->progress_y));

    /* Restore original view. */
    _zoom = _zoom_holder;
    _view_x = _view_x_holder;
    _view_y = _view_y_holder;

    return retval;
}

int Map::DrawSection(int x0, int y0, int x1, int y1, int resolution, int* p_progress_x, int* p_progress_y){

    //if(resolution==1){
    //    cout << "x0:   " << x0 << "\ty0:   " << y0 << "\tx1:   " << x1 << "\ty1:   " << y1 << "\n";
    //}

    /* Must reserve more space in the vertex here because if it re-pages when openGL is reading from it later it causes openGL crash on some platforms. */
    std::mutex* _p_data_mutex;
    std::vector<GLint>* _p_data_points;
    std::vector<GLubyte>* _p_data_colour;
    if(resolution == _low_res){
        _p_data_mutex = &windows[_window_index].data_low_res_mutex;
        _p_data_points = &_data_points_low_res;
        _p_data_colour = &_data_colour_low_res;
    } else {
        _p_data_mutex = &windows[_window_index].data_mutex;
        _p_data_points = &_data_points;
        _p_data_colour = &_data_colour;
    }
    if(_p_data_points->capacity() < 4000000){
        _p_data_mutex->lock();
        _p_data_points->reserve(4000000);
        _p_data_mutex->unlock();
    }
    if(_p_data_colour->capacity() < 6000000){
        _p_data_mutex->lock();
        _p_data_points->reserve(6000000);
        _p_data_mutex->unlock();
    }
    if(_p_data_points->capacity() < _p_data_points->size() * 2){    // TODO * 2 is excessive here. use a more sensible scheme once we know it works.
        _p_data_mutex->lock();
        _p_data_points->reserve(_p_data_points->size() * 2);
        _p_data_mutex->unlock();
    }
    if(_p_data_colour->capacity() < _p_data_colour->size() * 2){
        _p_data_mutex->lock();
        _p_data_colour->reserve(_p_data_colour->size() * 2);
        _p_data_mutex->unlock();
    }

    /* step size will always be a power of 2.
     * This is important to ensure the display pixels coincide with the data structure recursion levels
     * which means un-necesary recursion through the datastructure need not occur. */
    int step_x = MAX_SIZE / (long int)pow(2, (int)log2((long int)_width * _zoom));
    int step_y = MAX_SIZE / (long int)pow(2, (int)log2((long int)_height * _zoom));
    step_x *= resolution;
    step_y *= resolution;

    /* The (x0 % step_x) part ensures the display is alligned with the datastructure, preventing un-necesary recursion. */
    x0 -= (x0 % step_x);
    y0 -= (y0 % step_y);
    if(x0 < 0){
        x0 = 0;
    }
    if(y0 < 0){
        y0 = 0;
    }
    if(x0 >= MAX_SIZE){
        x0 = MAX_SIZE-1;
    }
    if(y0 >= MAX_SIZE){
        y0 = MAX_SIZE-1;
    } 

    float z_multiplier_dry = 255.0f / (data.Height_z_max() - data.Waterlevel());

    MapPoint tl, tr, bl, br;

   //cout << "Map::DrawSection resolution: " << resolution << "\tx0,y0: " << x0 << " , " << y0 << "\tx1,y1: " << x1 << " , " << y1 << "\tstep_x: " << step_x << "\tstep_y: " << step_y << "\n";
    for(int row = y0; row + step_y < y1; row+=step_y){
        if(p_progress_y and *p_progress_y > row){
            continue;
        }
        if(row <= 0 or row >= MAX_SIZE)
            continue;
        if(_p_data_points->size() > 40000000){
          cout << "RUNAWAY!\n";
          return 0;
        }
        for(int col = x0; col + step_x < x1; col+=step_x){
            if(p_progress_x and p_progress_y and *p_progress_x > col and *p_progress_y > row){
                continue;
            }
            if(col <= 0 or col >= MAX_SIZE)
                continue;
            timestamp_t now = get_timestamp();
            if(now > finish_time){
                //cout << "TIMEOUT!!\n";
                if(p_progress_x)
                    *p_progress_x = col;
                if(p_progress_y)
                    *p_progress_y = row;
                return SIG_VAL_TIMEOUT;
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

            _p_data_mutex->lock();

            _p_data_points->push_back (tl.x);
            _p_data_points->push_back (tl.y);
            _p_data_points->push_back (bl.x);
            _p_data_points->push_back (bl.y);
            _p_data_points->push_back (tr.x);
            _p_data_points->push_back (tr.y);

            _p_data_points->push_back (tr.x);
            _p_data_points->push_back (tr.y);
            _p_data_points->push_back (bl.x);
            _p_data_points->push_back (bl.y);
            _p_data_points->push_back (br.x);
            _p_data_points->push_back (br.y);

            _p_data_colour->push_back (0);
            if(tl.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - tl.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z, resolution));
            }
            _p_data_colour->push_back (0);
            if(bl.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z, resolution));
            }
            _p_data_colour->push_back (0);
            if(tr.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z, resolution));
            }

            _p_data_colour->push_back (0);
            if(tr.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0); 
                _p_data_colour->push_back (WaterCol(bl.z, resolution));
            }
            _p_data_colour->push_back (0);
            if(bl.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z, resolution));
            }
            _p_data_colour->push_back (0);
            if(br.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - br.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z, resolution));
            }

            _p_data_mutex->unlock();

            // Mark window for re-draw.
            windows[_window_index].dirty = 1;
        }
    }
//    if(p_progress_x and p_progress_y){
//        *p_progress_x = MAX_SIZE;
//        *p_progress_y = MAX_SIZE;
//    }
    return 0;
}

inline GLubyte Map::WaterCol(float height, int resolution){
    
    const float z_multiplier_wet = 255.0f / (data.Height_z_max() - data.Height_z_min());
    int rel_height = (float)(height - data.Height_z_min());
    if(resolution==1){
        if(rel_height % (int)(MAX_SIZE / (_zoom * 64)) < 3200 / _zoom)
            return 0;
    }
    return rel_height * z_multiplier_wet;
}

bool Map::ScrubView(void){
    int x0, y0, x1, y1;
    if(_width > _height){
	x0 = _view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
        x1 = _view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
        y0 = _view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
        y1 = _view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
    } else {
        x0 = _view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
        x1 = _view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
        y0 = _view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2))  * ((long)_height / (long)_width);
        y1 = _view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2))  * ((long)_height / (long)_width);
    }
    return ScrubView(x0, y0, x1, y1);
}

bool Map::ScrubView(int x0, int y0, int x1, int y1){
    static int _x0 = x0, _y0 = y0, _x1 = x1, _y1 = y1;

    static std::vector<GLint> _data_points_tmp;
    static std::vector<GLubyte> _data_colour_tmp;
    static std::vector<GLint> _data_points_low_res_tmp;
    static std::vector<GLubyte> _data_colour_low_res_tmp;

    static std::vector<GLint>::iterator data_it = _data_points.begin();
    static std::vector<GLubyte>::iterator colour_it = _data_colour.begin();
    static std::vector<GLint>::iterator data_low_res_it = _data_points_low_res.begin();
    static std::vector<GLubyte>::iterator colour_low_res_it = _data_colour_low_res.begin();

    static unsigned int i = 0, ii = 0, done = 0, done_low_res = 0;

    if(_x0 != x0 or _y0 != y0 or _x1 != x1 or _y1 != y1){
        _x0 = x0, _y0 = y0, _x1 = x1, _y1 = y1;
        _data_points_tmp.clear();
        _data_colour_tmp.clear();
        _data_points_low_res_tmp.clear();
        _data_colour_low_res_tmp.clear();
        data_it = _data_points.begin();
        colour_it = _data_colour.begin();
        data_low_res_it = _data_points_low_res.begin();
        colour_low_res_it = _data_colour_low_res.begin();
        i = 0;
        ii = 0;
        done = 0;
        done_low_res = 0;
        cout << "Map::ScrubView has moved.\n";
    }

    /* The data size of a screen pixel. Use this to leave a small overlap in place. */
    int pix_size = max((x1 - x0),(y1 - y0))/min(_width,_height);

    if(!done){

        float x[6], y[6], z[18];
        //for (unsigned int i = 0; i < _data_points.size() / 12; ++i){
        while(i < _data_points.size() / 12){
            timestamp_t now = get_timestamp();
            if(now > finish_time){
                return 1;
            }
            int retval = TestInterupt(SIG_DEST_MAP);
            if(retval){
                cout << "INTERUPT 2!\n";
                return (bool)retval;
            }
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

            if(x[1] >= x0 - pix_size and x[0] <= x1 + pix_size and y[2] >= y0 - pix_size and y[0] <= y1 + pix_size){
                for(unsigned int d = 0; d < 6; ++d){
                    _data_points_tmp.push_back(x[d]);
                    _data_points_tmp.push_back(y[d]);
                }
                for(unsigned int c = 0; c < 18; ++c){
                    _data_colour_tmp.push_back(z[c]);
                }
            }
            ++i;
        }

        windows[_window_index].data_mutex.lock();
        _data_points.swap(_data_points_tmp);
        _data_colour.swap(_data_colour_tmp);
        windows[_window_index].data_mutex.unlock();
        done = 1;
    }

    if(!done_low_res){
        if(_low_res){
            float x[6], y[6], z[18];
            //for (unsigned int i = 0; i < _data_points_low_res.size() / 12; ++i)
            while(ii < _data_points_low_res.size() / 12){
                timestamp_t now = get_timestamp();
                if(now > finish_time){
                    return 1;
                }
                int retval = TestInterupt(SIG_DEST_MAP);
                if(retval){
                    cout << "INTERUPT 3!\n";
                    return (bool)retval;
                }
                for(unsigned int d = 0; d < 6; ++d){
                    x[d] = *data_low_res_it;
                    ++data_low_res_it;
                    y[d] = *data_low_res_it;
                    ++data_low_res_it;
                }
                for(unsigned int c = 0; c < 18; ++c){
                    z[c] = *colour_low_res_it;
                    ++colour_low_res_it;
                }

                if(x[1] >= x0 - pix_size * _low_res and x[0] <= x1 + pix_size * _low_res and 
                        y[2] >= y0 - pix_size * _low_res and y[0] <= y1 + pix_size * _low_res){
                    for(unsigned int d = 0; d < 6; ++d){
                        _data_points_low_res_tmp.push_back(x[d]);
                        _data_points_low_res_tmp.push_back(y[d]);
                    }
                    for(unsigned int c = 0; c < 18; ++c){
                        _data_colour_low_res_tmp.push_back(z[c]);
                    }
                }
                ++ii;
            }
            windows[_window_index].data_low_res_mutex.lock();
            _data_points_low_res.swap(_data_points_low_res_tmp);
            _data_colour_low_res.swap(_data_colour_low_res_tmp);
            windows[_window_index].data_low_res_mutex.unlock();
            done_low_res = 1;
        }
    }
    return 0;
}

void Map::ActOnSignal(signal sig){
    //cout << "Viewport::ActOnSignal\n";
    //cout << "Viewport::ActOnSignal " << "\tsig.source: " << sig.source << "\tsig.dest: " << 
    //sig.dest << "\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
    if(sig.dest == SIG_DEST_MAP or sig.source == _window_index){
        if(sig.type == SIG_TYPE_KEYBOARD or sig.type == SIG_TYPE_ICON_CLICK){
            int store_view_x = _view_x;
            int store_view_y = _view_y;

            Task task;

            switch(sig.val){
                case SIG_VAL_ZOOM_IN:
                    _zoom *= 1.5;
                    task.type = TASK_TYPE_ZOOM;
                    _task_list.clear();
                    _task_list_low_res.clear();
                    break;
                case SIG_VAL_ZOOM_OUT:
                    _zoom /= 1.5;
                    task.type = TASK_TYPE_ZOOM;
                    _task_list.clear();
                    _task_list_low_res.clear();
                    break;
                case SIG_VAL_UP:
                    _view_y += KEY_MOVMENT / _zoom;
                    task.type = TASK_TYPE_PAN;
                    break;
                case SIG_VAL_DOWN:
                    _view_y -= KEY_MOVMENT / _zoom;
                    task.type = TASK_TYPE_PAN;
                    break;
                case SIG_VAL_LEFT:
                    _view_x -= KEY_MOVMENT / _zoom;
                    task.type = TASK_TYPE_PAN;
                    break;
                case SIG_VAL_RIGHT:
                    _view_x += KEY_MOVMENT / _zoom;
                    task.type = TASK_TYPE_PAN;
                    break;
                case SIG_VAL_SNAP:
                    store_view_x = _view_x = sig.val2;
                    store_view_y = _view_y = sig.val3;
                    task.type = TASK_TYPE_ZOOM;
                    _task_list.clear();
                    _task_list_low_res.clear();
                    break;
            }
            if(_zoom > 40000)
                task.zoom = _zoom = 40000;      // TODO base this on MIN_RECURSION

            /* Different aspects depending on whether map is portrait or landscape. */
            if(_width > _height){
		task.x0 = store_view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
                task.x1 = store_view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
                task.y0 = store_view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
                task.y1 = store_view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
            } else {
                task.x0 = store_view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
                task.x1 = store_view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
		task.y0 = store_view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_height / (long)_width);
		task.y1 = store_view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_height / (long)_width);
            }
            task.view_x = _view_x;
            task.view_y = _view_y;
            task.zoom = _zoom;
            task.progress_x = 0;
            task.progress_y = 0;

            /* For adding a 1px overlap. */
            int pix_size = 2 * max((task.x1-task.x0),(task.y1-task.y0)) / min(_width,_height);

            if(store_view_x < _view_x){
                task.x0 = task.x1 - pix_size;
                if(_width > _height){
		    task.x1 = _view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
                } else {
                    task.x1 = _view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
                }
            } else if(store_view_x > _view_x){
                task.x1 = task.x0 + pix_size;
                if(_width > _height){
                    task.x0 = _view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_width / (long)_height);
                } else {
                    task.x0 = _view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
                }
            } else if(store_view_y < _view_y){
                task.y0 = task.y1 - pix_size;
                if(_width > _height){
                    task.y1 = _view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
                } else {
                    task.y1 = _view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2)) * ((long)_height / (long)_width);
                }
            } else if(store_view_y > _view_y){
                task.y1 = task.y0 + pix_size;
                if(_width > _height){
                    task.y0 = _view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
                } else {
                    task.y0 = _view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2)) * ((long)_height / (long)_width);
                }
            }


            SetView(_view_x, _view_y, _zoom, _rotation);
            _task_list.push_back (task);
            if(_low_res)
                _task_list_low_res.push_back (task);

            //ProcessAllTasks();
        } else if(sig.type == SIG_TYPE_MOUSE_BUT and sig.val == 1){
            cout << "SIG_TYPE_MOUSE_BUT\n";
            static MapPoint point;
            static MapPoint point_start, point_end;

            if(point.x != (float)((int)windows[_window_index].mouse_x - (int)_width/2) / _width / _zoom * MAX_SIZE + _view_x + MAX_SIZE/2 or
                    point.y != (float)((int)windows[_window_index].mouse_y - (int)_height/2) / _height / _zoom * MAX_SIZE + _view_y + MAX_SIZE/2){
                cout << "\n* ";
                point.x = (float)((int)windows[_window_index].mouse_x - (int)_width/2) / _width / _zoom * MAX_SIZE + _view_x + MAX_SIZE/2;
                point.y = (float)((int)windows[_window_index].mouse_y - (int)_height/2) / _height / _zoom * MAX_SIZE + _view_y + MAX_SIZE/2;
                point.calculateTile(data.p_mapData);

                if(point_start.tile == 0 or point_end.tile != 0){
                    point_end.tile = 0;

                    point.limitRecursion(point.tile, &point_start);
                    point_start.calculateTile(data.p_mapData);

                    Icon icon = IconSquare(64);
                    icon.pos_x = point_start.x;
                    icon.pos_y = point_start.y;
                    icon.scale = pow(2, point.tile);
                    Icon_key key ;
                    key.type = ICON_TYPE_TEST;
                    key.key = icon.key;
                    AddIcon(key, icon);
                    cout << "START\n";
                } else {
                    point.limitRecursion(point.tile, &point_end);
                    point_end.calculateTile(data.p_mapData);

                    Icon icon = IconSquare(64);
                    icon.pos_x = point_end.x;
                    icon.pos_y = point_end.y;
                    icon.scale = pow(2, point.tile);
                    icon.key = 2;
                    Icon_key key ;
                    key.type = ICON_TYPE_TEST;
                    key.key = icon.key;
                    AddIcon(key, icon);
                    cout << "END\n";

                    CalculatePath path(point_start, point_end, 1);
                    if(path.Process(get_timestamp() + 1000*1000*20)){
                        cout << "Done path.Process\n";
                        path.Simplify(get_timestamp() + 1000*1000*20);
                        cout << "Done path.Simplify\n";
                        path.Straighten(get_timestamp() + 1000*1000*20);
                        cout << "Done path.Straighten\n";
                    }
                    /*Line test;
                    test.x0 = point_start.x;
                    test.y0 = point_start.y;
                    test.x1 = point_end.x;
                    test.y1 = point_end.y;
                    test.thickness = icon.scale;
                    AddLine(test);*/

                    path.Display(this);
                    //path.CheckLine(point_start, point_end);
                    cout << "Done path.Display\n";
                }
            }
        }
    }
}

void Map::ProcessAllTasks(int time_to_work){
    //cout << "Map::ProcessAllTasks " << _task_list_low_res.size() << "\t" << _task_list.size() << "\n";
    finish_time = get_timestamp() + time_to_work * 1000;

    windows[_window_index].low_res = _low_res;
    if(!ProcessTasks(&_task_list_low_res)){             // Draw low-res as highest priority.
        //cout << "L\n";
        if(!ScrubView()){                               // If that completes without being interupted, remove any data that has panned off the screen.
            //cout << "S\n";
            if(!ProcessTasks(&_task_list)){              // Then draw high-res.
                //cout << "H\n";
                windows[_window_index].low_res = 0;     // And if that completes, set this to 0 which means the low res background 
                // does not get drawn in the display thread.
            }
        }
    }
}

bool Map::ProcessTasks(std::vector<Task>* p_task_list){
    int interupt_val = 0;
    int resolution = 1;
    if(p_task_list == &_task_list_low_res){
        resolution = _low_res;
    }

    //cout << "Map::ProcessTasks " << resolution << " " << p_task_list->size() << "\n";

    for(std::vector<Task>::iterator it = p_task_list->begin() ; it != p_task_list->end(); ++it){
        //cout << "  resolution: " << resolution << "\ttype: " << it->type << "\tprogress_y: " << it->progress_y << 
        //        "\tview_x,view_y: " << it->view_x << "," << it->view_y << "\n";
        if(it->type == TASK_TYPE_ZOOM){
            if(!it->progress_x and !it->progress_y){
                Clear();            // only do this if we are not continuing a previously started DrawSection().
            }
	    cout << "TASK_TYPE_ZOOM\n";
            interupt_val = DrawSection(&(*it), resolution);
            // "< SIG_VAL_UP" covers zoom interrupt and no interrupt.
            if(interupt_val < SIG_VAL_UP){
                // set type to 0 so the task gets removed from task_list.
                it->type = 0;       // only do this if next keypress will draw whole map.
            }
        } else if(it->type == TASK_TYPE_PAN){
            //ScrubView();
	    cout << "TASK_TYPE_PAN\n";
            interupt_val = DrawSection(&(*it), resolution);
            if(interupt_val == 0){
                // set type to 0 so the task gets removed from task_list.
                it->type = 0;
            }
        }
        if (interupt_val){
            return 1;
        }
    }
    while(p_task_list->size() > 0 and p_task_list->back().type == 0){
        p_task_list->pop_back();
    }
    return 0;
}

void Map::DrawBoats(void){
    vessels.StartIcon();
    Icon icon;
    icon.scale = 0;
    icon.key = 0;
    while(icon.key != -1){
        icon = vessels.NextIcon(_window_index);
        if(icon.key != -1){
            //cout << icon.key << "\t" << icon.pos_x << " , " << icon.pos_y << " $\n";
            Icon_key key;
            key.type = ICON_TYPE_VESSEL;
            key.key = icon.key;
            AddIcon(key, icon);
        }
    }
    RedrawIcons();
    windows[_window_index].dirty = 1;
}

void Map::DrawText(void){
    Text text;
    text.font = GLUT_BITMAP_TIMES_ROMAN_10;

    windows[_window_index].data_mutex.lock();
    sprintf(text.text, "Triangles: %ld", windows[_window_index]._p_data_points->size() / 6 );
    windows[_window_index].data_mutex.unlock();
    AddText(10, 10, text);

    int pixSize = MAX_SIZE / (_width * _zoom * MIN_RECURSION);
    sprintf(text.text, "PixelSize: %d m", pixSize );
    AddText(10, 20, text);

    sprintf(text.text, "zoom: %d", (int)_zoom );
    AddText(10, 30, text);

    windows[_window_index].dirty = 1;
}
