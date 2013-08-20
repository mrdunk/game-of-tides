#ifndef SIGNALING_H
#define SIGNALING_H

#include <vector>
#include <unordered_map>

#define    MAX_DEST_ADDRESSES 256

#define    SIG_TYPE_KEYBOARD 1
#define    SIG_TYPE_MOUSE_X 2
#define    SIG_TYPE_MOUSE_Y 3
#define    SIG_TYPE_MOUSE_BUT 4
#define    SIG_TYPE_ICON_CLICK 5

#define    SIG_DEST_ALL 0
#define    SIG_DEST_TEST 1
#define    SIG_DEST_MAP 2
//#define    SIG_DEST_LAST MAX_DEST_ADDRESSES

#define    SIG_VAL_ZOOM_IN 1
#define    SIG_VAL_ZOOM_OUT 2
#define    SIG_VAL_UP       3
#define    SIG_VAL_DOWN     4
#define    SIG_VAL_LEFT     5
#define    SIG_VAL_RIGHT    6
#define    SIG_VAL_SNAP     7
#define    SIG_VAL_TIMEOUT  8

struct signal{
    unsigned int type;      // Type of event. 
    unsigned int source;    // Unique id of the code generating signal.
    unsigned int sequence;  // Increases every time a new signal is added.
    unsigned int dest;      // Destination id.
    int val;
    int val2;
    int val3;
    int key;
};

class Viewport;             // Declared here to prevent circular dependancy isssue if we included hiewport.h.

class Signal{
private:
    static std::unordered_map<unsigned int, Viewport*> _registered_endpoints;
public:
    static std::vector<signal> _sig_buf_A;
    static std::vector<signal> _sig_buf_B;
private:
    static std::vector<signal>* _p_active_sig_buf;
    static std::vector<signal>* _p_inactive_sig_buf;
    static unsigned int _write_counter;
    unsigned int _read_counter;
    static int _interupt[MAX_DEST_ADDRESSES];
public:
    Signal(void);
    void SwapBuf(void);
    void PushEvent(signal sig);
    bool PopEvent(signal* sig);

    /* RegisterEndpoint -           This should be called by any class that wants informed about incoming signals.
     *                              The class should have a (void)ActOnSignal(signal sig) method.
     * Args: (int)label:            This label will match signals .dest attribute.
     *       (Viewport*)p_class:    A matching signal will call p_class->ActOnSignal(signal sig).
     */
    void RegisterEndpoint(unsigned int label, Viewport* p_class);

    /* ServiceSignals - Compares the list of regestered objects and the list of incoming signals.
     *                  If the label used to register the object matches the signals .dest attribute, 
     *                  the object's .ActOnSignal(signal sig) method is called
     */
    void ServiceSignals(void);

    /* TestInterupt - If a signal has been registered for an address, this returns the interupt.val.
    */
    int TestInterupt(unsigned int address);
};



#endif // SIGNALING_H
