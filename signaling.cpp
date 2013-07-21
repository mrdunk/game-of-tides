#include <iostream>
#include <vector>
#include <unordered_map>

#include "signaling.h"
#include "viewport.h"

using namespace std;

/* Need to re-define as a hint to the linker.
 * http://stackoverflow.com/questions/272900/undefined-reference-to-static-class-member
 */
std::unordered_map<unsigned int, Viewport*> Signal::_registered_endpoints;
std::vector<signal> Signal::_sig_buf_A;
std::vector<signal> Signal::_sig_buf_B;
std::vector<signal>* Signal::_p_active_sig_buf;
std::vector<signal>* Signal::_p_inactive_sig_buf;
unsigned int Signal::_write_counter = 0;

Signal::Signal(void){
    cout << "Signal::Signal\n";
    _read_counter = 0;
    if (!_p_active_sig_buf){
        _p_active_sig_buf = &_sig_buf_A;
        _p_inactive_sig_buf = &_sig_buf_B;
    }
}

void Signal::SwapBuf(void){
    if(_p_active_sig_buf == &_sig_buf_A){
        _p_active_sig_buf = &_sig_buf_B;
        _p_inactive_sig_buf = &_sig_buf_A;
        _sig_buf_B.clear();
    }
    else {
        _p_active_sig_buf = &_sig_buf_A;
        _p_inactive_sig_buf = &_sig_buf_B;
        _sig_buf_A.clear();
    }
}

void Signal::PushEvent(signal sig){
    cout << "Signal::PushEvent " << _read_counter << "," << _write_counter << "\n";
    sig.sequence = _write_counter++;
    _p_active_sig_buf->push_back(sig);
}

bool Signal::PopEvent(signal* sig){
    sig->sequence = 0;
    for (std::vector<signal>::iterator it = _p_inactive_sig_buf->begin() ; it != _p_inactive_sig_buf->end(); ++it){
        if(it->sequence > _read_counter){
            _read_counter = it->sequence;
            *sig = *it;
            break;
        }
    }
    if(!sig->sequence){
        for (std::vector<signal>::iterator it = _p_active_sig_buf->begin() ; it != _p_active_sig_buf->end(); ++it){
            if(it->sequence > _read_counter){
                _read_counter = it->sequence;
                *sig = *it;
                break;
            }
        }
    }
    return (bool)sig->sequence;
}

void Signal::RegisterEndpoint(unsigned int label, Viewport* p_class){
    std::pair<unsigned int, Viewport*> entry(label, p_class);
    _registered_endpoints.insert(entry);
}

void Signal::ServiceSignals(void){
    signal sig;
    while(PopEvent(&sig)){
        //cout << "Signal::ServiceSignals " << "\tsig.source: " << sig.source << "\tsig.dest: " << sig.dest << 
        //"\tsig.key: " << sig.key << "\tsig.val: " << sig.val << "\n";
        for ( auto it = _registered_endpoints.begin(); it != _registered_endpoints.end(); ++it ){
            if(it->first == sig.dest){
                (it->second)->ActOnSignal(sig);
            }
        }
    }
}
