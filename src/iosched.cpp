#include <iostream>
using namespace std;
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <list>

#include <ioscheduler.h>

int current_track;
int total_time;

class IOSched{
private:
    list<ioreq_t*> iorequests;
    Scheduler* scheduler;
    int tot_movement;
    bool vFlag;
public:
    IOSched(ifstream& f_stream, char algo, bool vflag, bool qflag, bool fflag){
        total_time = 0;
        tot_movement = 0;
        current_track = 0;
        vFlag = vflag;

        //Read io requests
        string line;
        int io_no = 0;
        while(getline(f_stream, line)){
            if(line[0] == '#')
                continue;
            stringstream ssin(line);
            ioreq_t* x = new ioreq_t();
            x->io_no = io_no++;
            x->starttime = x->endtime = 0;
            ssin >> x->timestep >> x->track;
            if(x->track > 1999){
                cout<<"ioreq(0): track number out of range [0..1999]"<<endl;
                exit(1);
            }
            iorequests.push_back(x);
        }
        f_stream.close();

        //Plugin Scheduler
        switch(algo){
            case 'i':
                scheduler = new FIFO_Scheduler(qflag);
                break;
            case 'j':
                scheduler = new SSTF_Scheduler(qflag);
                break;
            case 's':
                scheduler = new LOOK_Scheduler(qflag);
                break;
            case 'c':
                scheduler = new CLOOK_Scheduler(qflag);
                break;
            case 'f':
                scheduler = new FLOOK_Scheduler(qflag, fflag);
                break;
            default:
                scheduler = new FIFO_Scheduler(qflag);
                break;
        }
    }

    void simulate(){
        if(vFlag) cout<<"TRACE"<<endl;
        list<ioreq_t*>::iterator new_req = iorequests.begin();
        ioreq_t* active_req = nullptr;
        short direction = 0;
        while(true){
            //if a new I/O arrived to the system at this current time → add request to IO-queue
            if(new_req != iorequests.end() && total_time == (*new_req)->timestep){
                if(vFlag) printf("%d: %5d add %d\n", total_time, (*new_req)->io_no, (*new_req)->track);
                scheduler->schedule(*new_req);
                new_req++;
            }
            //if an IO is active and completed at this time → Compute relevant info and store in IO request for final summary if no IO request active now
            if(active_req != nullptr && active_req->track == current_track){
                active_req->endtime = total_time;
                if(vFlag) printf("%d: %5d finish %d\n", total_time, active_req->io_no, active_req->endtime-active_req->timestep);
                active_req = nullptr;
                direction = 0;
            }
            //if requests are pending → Fetch the next request from IO-queue and start the new IO. else if all IO from input file processed
            //→ exit simulation
            if(active_req == nullptr){
                active_req = scheduler->strategy();
                if(active_req == nullptr){
                    if(new_req == iorequests.end())
                        break;
                }
                else{
                    active_req->starttime = total_time;
                    if(vFlag) printf("%d: %5d issue %d %d\n", total_time, active_req->io_no, active_req->track, current_track);
                    if(active_req->track == current_track)
                        continue;
                    if(current_track > active_req->track)
                        direction = -1;
                    else
                        direction = 1;
                }
            }
            //if an IO is active
            //→ Move the head by one unit in the direction its going (to simulate seek). Here direction = 0 if no io is active.
            current_track += direction;
            tot_movement += abs(direction);
            total_time++;
        }
    }

    void print_summary(){
        double avg_turnaround = 0.0;
        double avg_waittime = 0.0;
        int max_waittime = 0;
        double no_requests = iorequests.size();
        int i=0;
        for(ioreq_t* req : iorequests){
            printf("%5d: %5d %5d %5d\n",i++, req->timestep, req->starttime, req->endtime);
            avg_turnaround += req->endtime - req->timestep;
            avg_waittime += req->starttime - req->timestep;
            if(req->starttime - req->timestep > max_waittime)
                max_waittime = req->starttime - req->timestep;
        }
        avg_turnaround = avg_turnaround / no_requests;
        avg_waittime = avg_waittime / no_requests;
        printf("SUM: %d %d %.2lf %.2lf %d\n",
            total_time, tot_movement, avg_turnaround, avg_waittime, max_waittime);
    }
};

int main(int argc, char* argv[]){
    bool vFlag = false;
    bool qFlag = false;
    bool fFlag = false;
    char algo = 'r';
    char c;

    opterr = 0;

    while ((c = getopt(argc, argv, "vqfs:")) != -1){
        switch (c) {
            case 'v':
                vFlag = true;
                break;
            case 'q':
                qFlag = true;
                break;
            case 'f':
                fFlag = true;
                break;
            case 's':
                if(!optarg || (*optarg != 'i' && *optarg != 'j' && *optarg != 's' && *optarg != 'c' && *optarg != 'f')){
                    cout<<"Unknown Scheduling Algorithm: <"<<*optarg<<">"<<endl;
                    return 1;
                }
                algo = *optarg;
                break;
            case '?':
                if (optopt == 's')
                    cout<<"Unknown output option: <.>"<<endl;
                else if (isprint (optopt)){
                    cout << argv[0] << ": invalid option -- '" << (char) optopt << "'" << endl;
                    cout << "illegal option" << endl;
                }
                else{
                    cout << argv[0] << ": invalid option -- '" << std::hex << optopt << "'" << endl;
                    cout << "Usage: " << argv[0] << " [-v] inputfile randomfile" << endl;
                }
                return 1;
            default:
                abort();
        }
    }

    ifstream f_stream;
    char* inputfile = optind < argc ? argv[optind++] : 0;
    f_stream.open(inputfile);
    if(!f_stream){
        if(inputfile)
            cout<<"Cannot open inputfile <" << argv[optind-1] << ">" <<endl;
        else
            cout<<"inputfile name not supplied"<<endl;
        exit(1);
    }

    IOSched* iosched = new IOSched(f_stream, algo, vFlag, qFlag, fFlag);
    iosched->simulate();
    iosched->print_summary();
    return 0;
}