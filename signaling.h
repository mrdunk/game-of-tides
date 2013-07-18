#ifndef SIGNALING_H
#define SIGNALING_H

#include <vector>

enum {
    SIG_TYPE_KEYBOARD = 1,
    SIG_TYPE_MOUSE_X  = 2,
    SIG_TYPE_MOUSE_Y  = 3,
    SIG_TYPE_MOUSE_BUT = 4,
};

struct signal{
    unsigned int type;      // Type of event. 
    unsigned int source;    // Unique id of the code generating signal.
    unsigned int sequence;  // Increases every time a new signal is added.
    unsigned int dest;      // Destination id.
    unsigned int key;
    int val;
};

class Signal{
private:
    static std::vector<signal> _sig_buf_A;
    static std::vector<signal> _sig_buf_B;
    static std::vector<signal>* _p_active_sig_buf;
    static std::vector<signal>* _p_inactive_sig_buf;
    static unsigned int _write_counter;
public:
    unsigned int _read_counter;
//public:
    Signal(void);
    void SwapBuf(void);
    void PushEvent(signal sig);
    bool PopEvent(signal* sig);
};



#endif // SIGNALING_H
