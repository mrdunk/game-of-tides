#include <iostream>
#include <GL/glut.h>

#include "map.h"
#include "data.h"
#include "time_code.c"

using namespace std;

Data Map::data;

Map::Map(unsigned int label, int pos_x, int pos_y, int width, int height, int low_res) : Viewport(label, pos_x, pos_y, width, height){
    while(!_window_index); // wait until window is initialised.
    _task_list.clear();
    _task_list_low_res.clear();
    _low_res = low_res;
    windows[_window_index].low_res = low_res;
    windows[_window_index]._p_data_points_low_res = &_data_points_low_res;
    windows[_window_index]._p_data_colour_low_res = &_data_colour_low_res;
    
    vessels.StartIcon();
    Icon icon;
    icon.scale = 0;
    icon.key = 1;
    while(icon.key != 0){
        icon = vessels.NextIcon(0,0,MAX_SIZE,MAX_SIZE);
        if(icon.scale){
            Icon_key key;
            key.type = ICON_TYPE_VESSEL;
            key.key = icon.key;
            AddIcon(key, icon);
        }
    }
};

void Map::Draw(void){
    while(!windows[_window_index].window);  // wait until window is initialised.
    cout << flush << "Map::Draw " << _window_index << " " << _width << "," << _height << "\n" << flush;
    timestamp_t t0 = get_timestamp();

    windows[_window_index].data_size = _data_size = MAX_SIZE;

    Clear();

    DrawSection(0, 0, MAX_SIZE, MAX_SIZE, 1);

    timestamp_t t1 = get_timestamp();
    cout << "Map::Draw took " << (double)(t1 - t0) / 1000000.0L << " seconds.\n";
}

int Map::DrawSection(Task* p_task, int resolution){
    float _zoom_holder = _zoom;
    int _view_x_holder = _view_x;
    int _view_y_holder = _view_y;
    _zoom = p_task->zoom;
    _view_x = p_task->view_x;
    _view_y = p_task->view_y;

    int retval = DrawSection(p_task->x0, p_task->y0, p_task->x1, p_task->y1, resolution, &(p_task->progress_x), &(p_task->progress_y));

    _zoom = _zoom_holder;
    _view_x = _view_x_holder;
    _view_y = _view_y_holder;

    return retval;
}

int Map::DrawSection(int x0, int y0, int x1, int y1, int resolution, int* p_progress_x, int* p_progress_y){
    std::vector<GLint>* _p_data_points;
    std::vector<GLubyte>* _p_data_colour;
    if(resolution == _low_res){
        _p_data_points = &_data_points_low_res;
        _p_data_colour = &_data_colour_low_res;
    } else {
        _p_data_points = &_data_points;
        _p_data_colour = &_data_colour;
    }

    /* step size will always be a power of 2.
     * This is important to ensure the display pixels coincide with the data structure recursion levels
     * which means un-necesary recursion through the datastructure need not occur. */
    int step_x = MAX_SIZE / (int)pow(2, (int)log2(_width * _zoom));
    int step_y = MAX_SIZE / (int)pow(2, (int)log2(_height * _zoom));
    step_x *= resolution;
    step_y *= resolution;

    /* The (x0 % step_x) part ensures the display is alligned with the datastructure, preventing un-necesary recursion. */
    x0 -= (x0 % step_x);
    y0 -= (y0 % step_y);
    if(x0 < 0){
        x0 = 0;
        cout << "!!!!!!!!! <\n";
    }
    if(y0 < 0){
        y0 = 0;
        cout << "!!!!!!!!! <\n";
    }
    if(x0 >= MAX_SIZE){
        x0 = MAX_SIZE-1;
        cout << "!!!!!!!!! >\n";
    }
    if(y0 >= MAX_SIZE){
        y0 = MAX_SIZE-1;
        cout << "!!!!!!!!! >\n";
    } 

    float z_multiplier_dry = 255.0f / (data.Height_z_max() - data.Waterlevel());

    MapPoint tl, tr, bl, br;

    cout << "Map::DrawSection x0,y0: " << x0 << " , " << y0 << "\tx1,y1: " << x1 << " , " << y1 << "\tstep_x: " << step_x << "\tstep_y: " << step_y << "\n";
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
                _p_data_colour->push_back (WaterCol(bl.z));
            }
            _p_data_colour->push_back (0);
            if(bl.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z));
            }
            _p_data_colour->push_back (0);
            if(tr.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z));
            }

            _p_data_colour->push_back (0);
            if(tr.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - tr.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0); 
                _p_data_colour->push_back (WaterCol(bl.z));
            }
            _p_data_colour->push_back (0);
            if(bl.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - bl.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z));
            }
            _p_data_colour->push_back (0);
            if(br.z > data.Waterlevel()){
                _p_data_colour->push_back ((float)(data.Height_z_max() - br.z) * z_multiplier_dry);
                _p_data_colour->push_back (0);
            } else {        
                _p_data_colour->push_back (0);
                _p_data_colour->push_back (WaterCol(bl.z));
            }
        }
    }
//    if(p_progress_x and p_progress_y){
//        *p_progress_x = MAX_SIZE;
//        *p_progress_y = MAX_SIZE;
//    }
    return 0;
}

inline GLubyte Map::WaterCol(float height){
    const float z_multiplier_wet = _zoom * 255.0f / (data.Waterlevel() - data.Height_z_min());
    return (GLubyte)(255 - (float)(data.Waterlevel() - height) * z_multiplier_wet);
}

void Map::ScrubView(void){
    int x0, y0, x1, y1;
    if(_width > _height){
        x0 = -_view_x + (MAX_SIZE / 2) - (((long)_width * MAX_SIZE / (_zoom * 2 * _height)));
        x1 = -_view_x + (MAX_SIZE / 2) + (((long)_width * MAX_SIZE / (_zoom * 2 * _height)));
        y0 = -_view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
        y1 = -_view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
    } else {
        x0 = -_view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
        x1 = -_view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
        y0 = -_view_y + (MAX_SIZE / 2) - (((long)_height * MAX_SIZE / (_zoom * 2 * _width)));
        y1 = -_view_y + (MAX_SIZE / 2) + (((long)_height * MAX_SIZE / (_zoom * 2 * _width)));
    }
    ScrubView(x0, y0, x1, y1);
}

void Map::ScrubView(int x0, int y0, int x1, int y1){

    std::vector<GLint> _data_points_tmp;
    std::vector<GLubyte> _data_colour_tmp;
    std::vector<GLint> _data_points_low_res_tmp;
    std::vector<GLubyte> _data_colour_low_res_tmp;

    std::vector<GLint>::iterator data_it = _data_points.begin();
    std::vector<GLubyte>::iterator colour_it = _data_colour.begin();
    std::vector<GLint>::iterator data_low_res_it = _data_points_low_res.begin();
    std::vector<GLubyte>::iterator colour_low_res_it = _data_colour_low_res.begin();

    float x[6], y[6], z[18];
    for (unsigned int i = 0; i < _data_points.size() / 12; ++i){
        int retval = TestInterupt(SIG_DEST_MAP);
        if(retval){
            cout << "INTERUPT 2!\n";
            return;
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

        if(x[0] >= x0 and x[2] <= x1 and y[0] >= y0 and y[1] <= y1){
            for(unsigned int d = 0; d < 6; ++d){
                _data_points_tmp.push_back(x[d]);
                _data_points_tmp.push_back(y[d]);
            }
            for(unsigned int c = 0; c < 18; ++c){
                _data_colour_tmp.push_back(z[c]);
            }
        }
    }
    _data_points.swap(_data_points_tmp);                                                                                                                                       _data_colour.swap(_data_colour_tmp);

    if(_low_res){
        for (unsigned int i = 0; i < _data_points_low_res.size() / 12; ++i){
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

            if(x[0] >= x0 and x[2] <= x1 and y[0] >= y0 and y[1] <= y1){
                for(unsigned int d = 0; d < 6; ++d){
                    _data_points_low_res_tmp.push_back(x[d]);
                    _data_points_low_res_tmp.push_back(y[d]);
                }
                for(unsigned int c = 0; c < 18; ++c){
                    _data_colour_low_res_tmp.push_back(z[c]);
                }
            }
        }
        _data_points_low_res.swap(_data_points_low_res_tmp);
        _data_colour_low_res.swap(_data_colour_low_res_tmp);
    }
}

void Map::ActOnSignal(signal sig){
    //cout << "Viewport::ActOnSignal\n";
    //cout << "Viewport::ActOnSignal " << "\tsig.source: " << sig.source << "\tsig.dest: " << 
    //sig.dest << "\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
    if(sig.type == SIG_TYPE_KEYBOARD){
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
                _view_y -= KEY_MOVMENT / _zoom;
                task.type = TASK_TYPE_PAN;
                break;
            case SIG_VAL_DOWN:
                _view_y += KEY_MOVMENT / _zoom;
                task.type = TASK_TYPE_PAN;
                break;
            case SIG_VAL_LEFT:
                _view_x += KEY_MOVMENT / _zoom;
                task.type = TASK_TYPE_PAN;
                break;
            case SIG_VAL_RIGHT:
                _view_x -= KEY_MOVMENT / _zoom;
                task.type = TASK_TYPE_PAN;
                break;
        }
        if(_zoom > 16000)
            task.zoom = _zoom = 16000;      // TODO base this on MIN_RECURSION

        /* Different aspects depending on whether map is portrait or landscape. */
        if(_width > _height){
            task.x0 = -store_view_x + (MAX_SIZE / 2) - (((long)_width * MAX_SIZE / (_zoom * 2 * _height)));
            task.x1 = -store_view_x + (MAX_SIZE / 2) + (((long)_width * MAX_SIZE / (_zoom * 2 * _height)));
            task.y0 = -store_view_y + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
            task.y1 = -store_view_y + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
        } else {
            task.x0 = -store_view_x + (MAX_SIZE / 2) - (MAX_SIZE / (_zoom *2));
            task.x1 = -store_view_x + (MAX_SIZE / 2) + (MAX_SIZE / (_zoom *2));
            task.y0 = -store_view_y + (MAX_SIZE / 2) - (((long)_height * MAX_SIZE / (_zoom * 2 * _width)));
            task.y1 = -store_view_y + (MAX_SIZE / 2) + (((long)_height * MAX_SIZE / (_zoom * 2 * _width)));
        }
        task.view_x = _view_x;
        task.view_y = _view_y;
        task.zoom = _zoom;
        task.progress_x = 0;
        task.progress_y = 0;

        if(store_view_x > _view_x){
            task.x0 = task.x1 - KEY_MOVMENT;
        } else if(store_view_x < _view_x){
            task.x1 = task.x0 + KEY_MOVMENT;
        } else if(store_view_y > _view_y){
            task.y0 = task.y1 - KEY_MOVMENT;
        } else if(store_view_y < _view_y){
            task.y1 = task.y0 + KEY_MOVMENT;
        }


        SetView(_view_x, _view_y, _zoom, _rotation);
        _task_list.push_back (task);
        if(_low_res)
            _task_list_low_res.push_back (task);

        //RedrawIcons();
        //ScrubView();
        
        if(!ProcessTasks(&_task_list_low_res)){
            ScrubView();
            ProcessTasks(&_task_list);
        }
    }
}

bool Map::ProcessTasks(std::vector<Task>* p_task_list){
    int interupt_val = 0;
    int resolution = 1;
    if(p_task_list == &_task_list_low_res){
        resolution = _low_res;
    }

    RedrawIcons();

    for(std::vector<Task>::iterator it = p_task_list->begin() ; it != p_task_list->end(); ++it){
        //cout << "  resolution: " << resolution << "\ttype: " << it->type << "\tprogress_y: " << it->progress_y << 
        //        "\tview_x,view_y: " << it->view_x << "," << it->view_y << "\n";
        if(it->type == TASK_TYPE_ZOOM){
            if(!it->progress_x and !it->progress_y){
                Clear();            // only do this if we are not continuing a previously started DrawSection().
            }
            interupt_val = DrawSection(&(*it), resolution);
            // "< SIG_VAL_UP" covers zoom interrupt and no interrupt.
            if(interupt_val < SIG_VAL_UP){
                it->type = 0;       // only do this if next keypress will draw whole map.
            }
        } else if(it->type == TASK_TYPE_PAN){
            //ScrubView();
            interupt_val = DrawSection(&(*it), resolution);
            if(interupt_val == 0){
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



