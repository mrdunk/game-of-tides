#ifndef SIGNALING_H
#define SIGNALING_H

#include <vector>
#include <unordered_map>


#define    SIG_TYPE_KEYBOARD 1
#define    SIG_TYPE_MOUSE_X 2
#define    SIG_TYPE_MOUSE_Y 3
#define    SIG_TYPE_MOUSE_BUT 4

#define    SIG_DEST_ALL 0
#define    SIG_DEST_TEST 1
#define    SIG_DEST_MAP 2

#define    SIG_VAL_ZOOM_IN 1
#define    SIG_VAL_ZOOM_OUT 2
#define    SIG_VAL_UP       3
#define    SIG_VAL_DOWN     4
#define    SIG_VAL_LEFT     5
#define    SIG_VAL_RIGHT    6

struct signal{
    unsigned int type;      // Type of event. 
    unsigned int source;    // Unique id of the code generating signal.
    unsigned int sequence;  // Increases every time a new signal is added.
    unsigned int dest;      // Destination id.
    unsigned int key;
    int val;
};

class Viewport;             // Declared here to prevent circular dependancy isssue if we included hiewport.h.

class Signal{
private:
    static std::unordered_map<unsigned int, Viewport*> _registered_endpoints;
    static std::vector<signal> _sig_buf_A;
    static std::vector<signal> _sig_buf_B;
    static std::vector<signal>* _p_active_sig_buf;
    static std::vector<signal>* _p_inactive_sig_buf;
    static unsigned int _write_counter;
    unsigned int _read_counter;
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
};



#endif // SIGNALING_H
