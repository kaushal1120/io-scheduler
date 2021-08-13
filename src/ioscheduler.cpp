#include <iostream>
#include <ioscheduler.h>
#include <cmath>
#include <climits>

void Scheduler::schedule(ioreq_t* track){
    ioqueue.push_back(track);
}

FIFO_Scheduler::FIFO_Scheduler(bool qflag){
    qFlag = qflag;
}

ioreq_t* FIFO_Scheduler::strategy(){
    if(ioqueue.size() == 0)
        return nullptr;
    ioreq_t* next_req = ioqueue.front();
    ioqueue.pop_front();
    return next_req;
}

SSTF_Scheduler::SSTF_Scheduler(bool qflag){
    qFlag = qflag;
}

ioreq_t* SSTF_Scheduler::strategy() {
    if(ioqueue.size() == 0)
        return nullptr;
    int SST = INT_MAX;
    list<ioreq_t*>::iterator next_req_it;
    list<ioreq_t*>::iterator it = ioqueue.begin();
    ioreq_t* next_req = nullptr;
    if(qFlag) printf("\t");
    while(it != ioqueue.end()){
        if(qFlag) printf("%d:%d ",(*it)->io_no, abs((*it)->track - current_track));
        if(abs((*it)->track - current_track) < SST){
            SST = abs((*it)->track - current_track);
            next_req_it = it;
        }
        it++;
    }
    if(qFlag) printf("\n");
    next_req = *next_req_it;
    ioqueue.erase(next_req_it);
    return next_req;
}

LOOK_Scheduler::LOOK_Scheduler(bool qflag){
    direction = 1;
    qFlag = qflag;
}

ioreq_t* LOOK_Scheduler::strategy(){
    if(ioqueue.size() == 0)
        return nullptr;
    int closest = INT_MAX;
    list<ioreq_t*>::iterator next_req_it = ioqueue.end();
    list<ioreq_t*>::iterator it = ioqueue.begin();
    ioreq_t* next_req = nullptr;
    if(qFlag) printf("\tGet: (");
    if(direction == 1){
        while(it != ioqueue.end()){
            if((*it)->track >= current_track){
                if(qFlag) printf("%d:%d ", (*it)->io_no, (*it)->track-current_track);
                if((*it)->track - current_track < closest){
                    closest = (*it)->track - current_track;
                    next_req_it = it;
                }
            }
            it++;
        }
    }
    else{
        while(it != ioqueue.end()){
            if(current_track >= (*it)->track){
                if(qFlag) printf("%d:%d ", (*it)->io_no, current_track-(*it)->track);
                if(current_track - (*it)->track < closest){
                    closest = current_track - (*it)->track;
                    next_req_it = it;
                }
            }
            it++;
        }
    }
    if(qFlag) printf(") --> ");
    if(next_req_it == ioqueue.end()){
        direction = -direction;
        if(qFlag) printf("change direction to %d\n\tGet: (", direction);
        it = ioqueue.begin();
        if(direction == 1){
            while(it != ioqueue.end()){
                if((*it)->track >= current_track){
                    if(qFlag) printf("%d:%d ", (*it)->io_no, (*it)->track-current_track);
                    if((*it)->track - current_track < closest){
                        closest = (*it)->track - current_track;
                        next_req_it = it;
                    }
                }
                it++;
            }
        }
        else{
            while(it != ioqueue.end()){
                if(current_track >= (*it)->track){
                    if(qFlag) printf("%d:%d ", (*it)->io_no, current_track-(*it)->track);
                    if(current_track - (*it)->track < closest){
                        closest = current_track - (*it)->track;
                        next_req_it = it;
                    }
                }
                it++;
            }
        }
        if(qFlag) printf(") --> ");
    }
    if(qFlag) printf("%d dir=%d\n", (*next_req_it)->io_no, direction);
    next_req = *next_req_it;
    ioqueue.erase(next_req_it);
    return next_req;
}

CLOOK_Scheduler::CLOOK_Scheduler(bool qflag){
    qFlag = qflag;
}

ioreq_t* CLOOK_Scheduler::strategy(){
    if(ioqueue.size() == 0)
        return nullptr;
    int closest = INT_MAX;
    int leftmost = INT_MAX;
    list<ioreq_t*>::iterator next_req_it = ioqueue.end();
    list<ioreq_t*>::iterator next_leftmost_req_it = ioqueue.end();
    list<ioreq_t*>::iterator it = ioqueue.begin();
    ioreq_t* next_req = nullptr;
    if(qFlag) printf("\tGet: (");
    while(it != ioqueue.end()){
        if((*it)->track < leftmost){
            leftmost = (*it)->track;
            next_leftmost_req_it = it;
        }
        if((*it)->track >= current_track){
            if(qFlag) printf("%d:%d ", (*it)->io_no, (*it)->track-current_track);
            if((*it)->track - current_track < closest){
                closest = (*it)->track - current_track;
                next_req_it = it;
            }
        }
        it++;
    }
    if(qFlag) printf(") --> ");
    if(next_req_it != ioqueue.end()){
        if(qFlag) printf("%d\n",(*next_req_it)->io_no);
        next_req = *next_req_it;
        ioqueue.erase(next_req_it);
    }
    else{
        if(qFlag) printf("go to bottom and pick %d\n", (*next_leftmost_req_it)->io_no);
        next_req = *next_leftmost_req_it;
        ioqueue.erase(next_leftmost_req_it);
    }
    return next_req;
}

FLOOK_Scheduler::FLOOK_Scheduler(bool qflag, bool fflag) : LOOK_Scheduler(qflag){
    qFlag = qflag;
    fFlag = fflag;
    direction = 1;
    queue_no = 0;
}

void FLOOK_Scheduler::schedule(ioreq_t* req){
    if(qFlag) printf("   Q=%d ", 1-queue_no);
    queues[1-queue_no].push_back(req);
    list<ioreq_t*>::iterator it = queues[1-queue_no].begin();
    if(qFlag) printf("(");
    while(it != queues[1-queue_no].end()){
        if(qFlag) printf(" %d:%d", (*it)->io_no, (*it)->track);
        it++;
    }
    if(qFlag) printf(" )\n");
}

ioreq_t* FLOOK_Scheduler::strategy(){
    if(queues[queue_no].size() == 0)
        queue_no = 1-queue_no;
    if(queues[queue_no].size() == 0)
        return nullptr;

    list<ioreq_t*>::iterator it;

    if(qFlag) printf("AQ=%d dir=%d curtrack=%d: ", queue_no, direction, current_track);
    for(int i=0; i < 2; i++){
        if(qFlag) printf(" Q[%d] = ",i);
        it = queues[i].begin();
        if(qFlag) printf("( ");
        while(it != queues[i].end()){
            if(qFlag) printf("%d:%d:%d ", (*it)->io_no, (*it)->track, (*it)->track-current_track);
            it++;
        }
        if(qFlag) printf(") ");
    }
    if(qFlag) printf("\n");

    int closest = INT_MAX;
    list<ioreq_t*>::iterator next_req_it = queues[queue_no].end();
    it = queues[queue_no].begin();
    ioreq_t* next_req = nullptr;
    if(qFlag) printf("\tGet: (");
    if(direction == 1){
        while(it != queues[queue_no].end()){
            if((*it)->track >= current_track){
                if(qFlag) printf("%d:%d:%d ", (*it)->io_no, (*it)->track, (*it)->track-current_track);
                if((*it)->track - current_track < closest){
                    closest = (*it)->track - current_track;
                    next_req_it = it;
                }
            }
            it++;
        }
    }
    else{
        while(it != queues[queue_no].end()){
            if(current_track >= (*it)->track){
                if(qFlag) printf("%d:%d:%d ", (*it)->io_no, (*it)->track, current_track-(*it)->track);
                if(current_track - (*it)->track < closest){
                    closest = current_track - (*it)->track;
                    next_req_it = it;
                }
            }
            it++;
        }
    }
    if(qFlag) printf(") --> ");
    if(next_req_it == queues[queue_no].end()){
        direction = -direction;
        if(qFlag) printf("change direction to %d\n\tGet: (", direction);
        it = queues[queue_no].begin();
        if(direction == 1){
            while(it != queues[queue_no].end()){
                if((*it)->track >= current_track){
                    if(qFlag) printf("%d:%d:%d ", (*it)->io_no, (*it)->track, (*it)->track-current_track);
                    if((*it)->track - current_track < closest){
                        closest = (*it)->track - current_track;
                        next_req_it = it;
                    }
                }
                it++;
            }
        }
        else{
            while(it != queues[queue_no].end()){
                if(current_track >= (*it)->track){
                    if(qFlag) printf("%d:%d:%d ", (*it)->io_no, (*it)->track, current_track-(*it)->track);
                    if(current_track - (*it)->track < closest){
                        closest = current_track - (*it)->track;
                        next_req_it = it;
                    }
                }
                it++;
            }
        }
        if(qFlag) printf(") --> ");
    }
    if(qFlag) printf("%d dir=%d\n", (*next_req_it)->io_no, direction);
    if(fFlag) printf("%d:   %5d get Q=%d\n", total_time, (*next_req_it)->io_no, queue_no);
    next_req = *next_req_it;
    queues[queue_no].erase(next_req_it);
    return next_req;
}