using namespace std;
#include <list>
#include <ioreq_t.h>

extern int current_track;
extern int total_time;

class Scheduler {
public:
    bool qFlag;
    list<ioreq_t*> ioqueue;
    virtual void schedule(ioreq_t* req);
    virtual ioreq_t* strategy() = 0; // virtual base class
};

class FIFO_Scheduler : public Scheduler {
public:
    FIFO_Scheduler(bool qflag);
    ioreq_t* strategy();
};

class SSTF_Scheduler : public Scheduler {
public:
    SSTF_Scheduler(bool qflag);
    ioreq_t* strategy();
};

class LOOK_Scheduler : public Scheduler {
public:
    short direction;
    LOOK_Scheduler(bool qflag);
    ioreq_t* strategy();
};

class CLOOK_Scheduler : public Scheduler {
public:
    CLOOK_Scheduler(bool qflag);
    ioreq_t* strategy();
};

class FLOOK_Scheduler : public LOOK_Scheduler {
private:
    list<ioreq_t*> queues[2];
    int queue_no;
    bool fFlag;
public:
    FLOOK_Scheduler(bool qflag, bool fflag);
    void schedule(ioreq_t* req);
    ioreq_t* strategy();
};