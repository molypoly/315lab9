#include <stdio.h>
#include <stdlib.h>

#define N 6
#define TOTALTIME 26
#define NONE -1
#define NOTARRIVED 0
#define ARRIVED 1
#define ONCPU 2
#define WAITING 3
#define DONE 4
#define TRUE 1
#define FALSE 0

typedef struct {    // Declaring the Process Struct
  int pid;
  int arrival;            // Arrival time
  int arrived;            // Boolean indicating arrival
  int start;              // Boolean indicating arrival
  int totalService;       // Total CPU time needed
  int remainingService;   // Remaining service time needed
  int totalWait;          // Total waiting time so far
  int status;             // NOTARRIVED/WAITING/ONCPU/DONE
  int priority;           // Higher value, higher priority (prio)
  int finish;             // Finish time
} Process;

/***************************************************************/

Process process[N];          // Array of processes
int current_time = 0;        // Current time
int current_pid  = NONE;     // PID of process on the CPU
                             //   - NONE if CPU is idle or when a
                             //     process is being removed.

char sched_type;         // f = FCFS (fcfs)
                         // s = Shortest Job First Preemptive (sjf)
                         // n = Shortest Job First NonPreemptive (nsjf)
                         // r = Round Robin (roro)
                         // p = Priority (prio)

// The algorithm being used. Define with a function pointer.
void (*sched_algorithm)(Process *);

/***************************************************************/

int main(int argc, char *argv[]) {

  int i;
  void fcfs(Process *);
  void roro(Process *);
  void prio(Process *);
  void nsjf(Process *);
  void sjf(Process *);

  void printProcess(int i);
  void instructions(char *);
  void reportAll();
  void updateStats(Process *);

  if (argc < 2) {
    instructions(argv[0]);
    exit(0);
  }
  sched_type = argv[1][0];
  if (! (sched_type == 'f' || 
	 sched_type == 's' || 
	 sched_type == 'n' || 
	 sched_type == 'r' ||
	 sched_type == 'p'    )) {
    instructions(argv[0]);
    exit(0);
  }
  switch(sched_type) {
  case 'f': sched_algorithm = fcfs; break;
  case 'n': sched_algorithm = nsjf; break;
  case 'r': sched_algorithm = roro; break;
  case 'p': sched_algorithm = prio; break;
  case 's': sched_algorithm = sjf; break;
  }

  // Read in data from the keyboard to the process array
  int arrive, totserv, priority;
  for (i = 0; i < N; i++) {
    process[i].pid = i;
    scanf("%d %d %d", &arrive, &totserv, &priority);
    process[i].arrival = arrive;
    process[i].totalService = totserv;
    process[i].priority = priority;
    process[i].remainingService = process[i].totalService;
    process[i].totalWait = 0;
    process[i].arrived = FALSE;
    process[i].status = NOTARRIVED;
    process[i].start  = -1;
    process[i].finish = -1;
  }
  //printf("Finished reading\n"); fflush(stdout);

  // Loop cycles once for every time unit.
  for (current_time = 0; current_time <= TOTALTIME; current_time++) {
    updateStats(process);
    sched_algorithm(process);
  }

  printf("Using ");
  switch (sched_type) {
    case 'f': printf("First Come First Serve:\n"); break;
    case 's': printf("Shortest Job First (Preemptive):\n"); break;
    case 'n': printf("Shortest Job First (Non-Preemptive):\n"); break;
    case 'r': printf("Round Robin:\n"); break;
    case 'p': printf("Priority:\n"); break;
  }
  reportAll(process);
}

/*****************************************************************************/
// This function updates the data for each process item that has arrived.
// WAITING processes - increase wait time by 1
// ONCPU process - decrement remainingService time
//               - change status to DONE if finished
//               - note the finished time in process[i].finish
//               - set the current_process to NONE.
// NOTARRIVED processes - set ARRIVED to true if arrival == current_time
//                      - set status to WAITING
//                      
void updateStats(Process *p) {
  int i; char foo;
  for(i=0; i<N; i++) {
    // If p[i] is WAITING increment totalWait
    if(p[i].arrived && p[i].status == WAITING) p[i].totalWait++;

    // If p[i] has just arrived change status to WAITING
    if(!p[i].arrived && p[i].arrival == current_time) {
       p[i].status = WAITING;
       p[i].arrived = TRUE;
    }

    // If p[i] is ONCPU decrement remainingService and check to see
    // if it is DONE. If DONE change status and note finish time.
    if(p[i].status == ONCPU) {
       p[i].remainingService--;
       if(p[i].remainingService == 0) {
          p[i].status = DONE;
          p[i].finish = current_time;
          current_pid = NONE;
       }
    }
    //printProcess(i);
  }
}
/*****************************************************************************/
// The scheduling algorithms follow.
// Each algorithm determines if scheduling should occur, and if so, chooses
// the process to be put on the CPU (by setting the value of current_pid).
// and updating the status of the affected processes.
/*****************************************************************************/

// First Come First Serve Algorithm
// Schedule only if no process is on the CPU. This situation will occur when
// at time 0 and when a process finishes.

void fcfs(Process *p) {
  if(current_pid == NONE) {   // Only schedule if needed
    int i;
    for(i=0; i<N; i++) {
      if(p[i].arrived && p[i].status == WAITING) {
        p[i].status = ONCPU;
        if(p[i].start == NONE) p[i].start = current_time;
        current_pid = i;
        i = N;
      }
    }
  }
}
/*****************************************************************************/

// Shortest Job First Algorithm (Non-Preemptive)
// Schedule only if no process is on the CPU. This situation will occur when
// at time 0 and when a process finishes. If no process is on the CPU,
// put the WAITING process that has the shortest remainingTime.

void nsjf(Process *p) {        // Non-Premptive

  if(current_pid == NONE) {    // Only schedule if needed
    int i;
    int minPid = NONE;         // pid of current shortest process
    int min    = NONE;         // Remaining service time of shortest process

    // Find the process with the shortest totalService
    for(i=0; i<N; i++) {
      if(p[i].arrived && p[i].status != DONE) {
        if(min == NONE) { minPid = i; min = p[i].totalService; }
        else if(p[i].totalService < min) {
          minPid = i;
          min = p[i].totalService;  
        }
      }
    }

    if(minPid != NONE) {
      p[minPid].status = ONCPU;
      current_pid      = minPid;
      p[minPid].start  = current_time;
    }
  }
}

/*****************************************************************************/

// Shortest Job First Algorithm (Preemptive)
// Check the queue of WAITING processes for the one with the shortest
// remainingTime. If this process is not already on the CPU, put it on
// the CPU, by changing the status of the currently running process to
// WAITING and updating the new process' status to ONCPU.

void sjf(Process *p) {        // Premptive
  int i;
  int minPid = NONE;          // pid of current shortest process
  int min    = NONE;          // Remaining service time of shortest process

  // Find the process with the shortest remainingService
  for(i=0; i<N; i++) {
    if(p[i].arrived && p[i].status != DONE) {
      if(min == NONE) { minPid = i; min = p[i].remainingService; }
      else if(p[i].remainingService < min) {
        minPid = i;
        min = p[i].remainingService;  
      }
    }
  }

  if(minPid != NONE) {
    if(current_pid != NONE) {
      p[current_pid].status = WAITING;
    }
    p[minPid].status = ONCPU;
    current_pid      = minPid;
    if(p[minPid].start == NONE) p[minPid].start = current_time;
  }
}

/*****************************************************************************/

// Priority Scheduling Algorithm (Preemptive)
// Check the queue of WAITING processes for the one with the highest 
// priority. If this process is not already on the CPU, put it on
// the CPU, by changing the status of the currently running process to
// WAITING and updating the new process' status to ONCPU.

void prio(Process *p) {    // Preemptive (by definition)
  int i;
  int maxPid = NONE;       // pid of current highest priority process
  int max    = NONE;       // Priority of current highest prioriity process


  /******** Start of Your Code **********/

    // Find the process with the highest priority
  for(i=0; i<N; i++) {
    if(p[i].arrived && p[i].status != DONE) {
      if(max == NONE) { maxPid = i; max = p[i].priority; }
      else if(p[i].priority < max) {
        maxPid = i;
        max = p[i].priority;  
      }
    }
  }


  // Update the affected processes statuses (and start if needed) as well as current_pid
  if(maxPid != NONE) {
    if(current_pid != NONE) {
      p[current_pid].status = WAITING;
    }
    p[maxPid].status = ONCPU;
    current_pid      = maxPid;
    if(p[maxPid].start == NONE) p[maxPid].start = current_time;
  }

  /********* End of Your Code ***********/

}

/*****************************************************************************/
// Round Robin Scheduling Algorithm
// *** Not yet implemented ***
void roro(Process *p) {
  printf("Inside roro ");
  printf(" Leaving roro\n");
}
/*****************************************************************************/
/*****************************************************************************/
int schedRequired(){
  if(sched_type == 'f') ;
  if(sched_type == 'r') ;
  if(sched_type == 's') ;
  if(sched_type == 'p') ;
  if(sched_type == 'f') ;
  return TRUE;
}
void printProcess(int i) {
  char a = 'N';
  if(process[i].arrived) a = 'Y';
  printf("P[%2d]: arvl:%2d  arvd:%c  ", i, process[i].arrival, a);
  printf("st:%2d  ts:%2d  ", process[i].start, process[i].totalService);
  printf("rs:%2d  tw:%2d  ", process[i].remainingService, process[i].totalWait);
  printf("stat:%2d  pr:%1d  ", process[i].status, process[i].priority);
  printf("fin:%2d\n",process[i].finish);
}

/*****************************************************************************/
void instructions(char *command) {
  printf("Usage: %s sched-type\n", command);
  printf("  where sched-type is\n");
  printf("     f   for First Come First Serve\n");
  printf("     s   for Shortest Job First\n");
  printf("     r   for Round Robin\n");
  printf("     p   for Priority\n");
  printf("     l   for Longest Job First\n");
}
/*****************************************************************************/
// This function calculates the statistics for each process as well as
// average statistics of all processes.
void reportAll(Process *p) {
  int i;
  double totalWait = 0;
  double totalTurnaround = 0;
  double totalResponse = 0;
  for(i=0; i<N; i++) {
    totalWait       += p[i].totalWait;
    totalTurnaround += p[i].finish - p[i].arrival;
    totalResponse   += p[i].start  - p[i].arrival;
    printf("P[%2d]:  ",i);
    printf("Total Service: %-3d ", p[i].totalService);
    printf("Total Wait: %-3d ", p[i].totalWait);
    printf("Turnaround: %-3d ", p[i].finish - p[i].arrival);
    printf("Response: %-3d\n", p[i].start  - p[i].arrival);
  }
  printf("==========================\n");
  printf("Average Wait:       %6.2f\n", totalWait/N);
  printf("Average Turnaround: %6.2f\n", totalTurnaround/N);
  printf("Average Response:   %6.2f\n", totalResponse/N);
}


