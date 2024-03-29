//Matthew Dominicis, Section 102

#include "process.h"
#include "ioModule.h"
#include "processMgmt.h"
#include<queue>
#include <chrono> // for sleep
#include <thread> // for sleep
bool notAllDone(list<Process> x);
int main(int argc, char* argv[])
{
    // single thread processor
    // it's either processing something or it's not
    bool processorAvailable = true;

    // vector of processes, processes will appear here when they are created by
    // the ProcessMgmt object (in other words, automatically at the appropriate time)
    list<Process> processList;
    
    vector<list<Process>::iterator> readyList;

    // this will orchestrate process creation in our system, it will add processes to 
    // processList when they are created and ready to be run/managed
    ProcessManagement processMgmt(processList);

    // this is where interrupts will appear when the ioModule detects that an IO operation is complete
    list<IOInterrupt> interrupts;   

    // this manages io operations and will raise interrupts to signal io completion
    IOModule ioModule(interrupts);  

    // Do not touch
    long time = 1;
    long sleepDuration = 50;
    string file;
    stringstream ss;
    enum stepActionEnum {noAct, admitNewProc, handleInterrupt, beginRun, continueRun, ioRequest, complete} stepAction;

    // Do not touch
    switch(argc)
    {
        case 1:
            file = "./procList.txt";  // default input file
            break;
        case 2:
            file = argv[1];         // file given from command line
            break;
        case 3:
            file = argv[1];         // file given
            ss.str(argv[2]);        // sleep duration given
            ss >> sleepDuration;
            break;
        default:
            cerr << "incorrect number of command line arguments" << endl;
            cout << "usage: " << argv[0] << " [file] [sleepDuration]" << endl;
            return 1;
            break;
    }

    processMgmt.readProcessFile(file);


    time = 0;
    processorAvailable = true; 
    //keep running the loop until all processes have been added and have run to completion
    while(processMgmt.moreProcessesComing() || notAllDone(processList)/* TODO add something to keep going as long as there are processes that arent done! */ )
    {
        ++time;

        //let new processes in if there are any
        processMgmt.activateProcesses(time);

        //update the status for any active IO requests
        ioModule.ioProcessing(time);

        //If the processor is tied up running a process, then continue running it until it is done or blocks
        //   note: be sure to check for things that should happen as the process continues to run (io, completion...)
        //If the processor is free then you can choose the appropriate action to take, the choices (in order of precedence) are:
        // - admit a new process if one is ready (i.e., take a 'newArrival' process and put them in the 'ready' state)
        // - address an interrupt if there are any pending (i.e., update the state of a blocked process whose IO operation is complete)
        // - start processing a ready process if there are any ready

        //init the stepAction, update below
        stepAction = noAct;

        
        //TODO add in the code to take an appropriate action for this time step!
        //you should set the action variable based on what you do this time step. you can just copy and paste the lines below and uncomment them, if you want.
        //stepAction = continueRun;  //runnning process is still running
        //stepAction = ioRequest;  //running process issued an io request
        //stepAction = complete;   //running process is finished
        //stepAction = admitNewProc;   //admit a new process into 'ready'
        //stepAction = handleInterrupt;   //handle an interrupt
        //stepAction = beginRun;   //start running a process
        //   <your code here>
        bool newp=false;
        //variable to see if a new process is in the list, if there is, it can skip the rest of the loop

        //checks if processor is not avaialable, and if readylist is not empty
        if(!processorAvailable&&!readyList.empty()){
          if(readyList.front()->processorTime==readyList.front()->reqProcessorTime){
            //check if process is done
            readyList.front()->state=done;
            readyList.erase(readyList.begin());
            stepAction=complete;
            processorAvailable=true;
          }
          else if(readyList.front()->ioEvents.size()>0&&readyList.front()->processorTime==readyList.front()->ioEvents.front().time){
            //check for io event, move process to blocked state
            readyList.front()->state=blocked;
            stepAction=ioRequest;
            ioModule.submitIORequest(time,readyList.front()->ioEvents.front(),*readyList.front());
            readyList.erase(readyList.begin());
            processorAvailable=true;
          }
          
          else{
            //continue running
            stepAction=continueRun;
            readyList.front()->processorTime++;
          }
        }
        else{
          for(list<Process>::iterator it2=processList.begin();it2!=processList.end()&&stepAction==noAct;++it2){
            //iterate through list if processor is available, if any processes are added, convert from new to ready, add to readylist
            if(it2->state==newArrival){
              stepAction=admitNewProc;
              it2->state=ready;
              readyList.push_back(it2);
              newp=true;
            }
          }
          if(newp==false && !interrupts.empty()){
            //if there is an interrupt, deal with it
            for(list<Process>::iterator it2=processList.begin();it2!=processList.end();++it2){
              if(it2->id==interrupts.front().procID){
                it2->state=ready;
                readyList.push_back(it2);
                it2->ioEvents.pop_front();
                break;
              }
            }
            interrupts.pop_front();
            stepAction=handleInterrupt;
          }
          else if(!newp&&readyList.size()!=0){
            //if no other events happened, load up a process and run it
            readyList.front()->state=processing;
            processorAvailable=false;
            stepAction=beginRun;
            readyList.front()->processorTime++;
          }
        }



        // Leave the below alone (at least for final submission, we are counting on the output being in expected format)
        cout << setw(5) << time << "\t"; 
        
        switch(stepAction)
        {
            case admitNewProc:
              cout << "[  admit]\t";
              break;
            case handleInterrupt:
              cout << "[ inrtpt]\t";
              break;
            case beginRun:
              cout << "[  begin]\t";
              break;
            case continueRun:
              cout << "[contRun]\t";
              break;
            case ioRequest:
              cout << "[  ioReq]\t";
              break;
            case complete:
              cout << "[ finish]\t";
              break;
            case noAct:
              cout << "[*noAct*]\t";
              break;
        }

        // You may wish to use a second vector of processes (you don't need to, but you can)
        printProcessStates(processList); // change processList to another vector of processes if desired

        this_thread::sleep_for(chrono::milliseconds(sleepDuration));
    }
    return 0;
}

bool notAllDone(list<Process> x){
  //function to go through process list and see if any processes are not done yet
  if(x.size()==0){
    return true;
  }
  for(list<Process>::iterator it2=x.begin();it2!=x.end();++it2){
    if(it2->state!=done){
      return true;
    }
  }
  return false;
}