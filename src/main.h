#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h> 
#include <time.h> 
#include <stdarg.h> 

#define MAX_PROCESSES 10
#define MAX_TIME_QUANTUM 5
#define SCHEDULERS 9 //구현한 스케줄러 개수
#define MAX_IO_NUM 2

/* struct */ 
typedef struct {
    // Process data (1번만 초기화)
    int pid;
    int arrival_time;
    int priority;
    int cpu_burst;
    int io_timing[MAX_IO_NUM];
    int io_burst[MAX_IO_NUM];
    // Scheduling data (시뮬레이션마다 초기화)
    int remaining_time; 
    int remaining_io; 
    int waiting_time; 
    int response_time; 
    int completion_time;
    int total_remaining_io;
} ProcessData;
typedef struct Node {
    ProcessData *p_process;
    struct Node *next;
    // state queue 전용
    int time;
    char data;
} Node;
typedef struct {
    Node *front;
    Node *rear;
} Queue;

/* functions */
/* process.c    프로세스 생성 및 초기화 함수*/
void CreateProcess(ProcessData p[], int pNum);
void AssignProcessValues(ProcessData *p, int i);
void shuffle(int *arr, int n);
int compare(const void* a, const void* b);
void InitRuntimeData(ProcessData p[], int pNum);
int IsAllTerminated(ProcessData p[], int pNum);

/* simulator.c  시뮬레이션 환경 초기화 및 시간 단위 루프 반복 관련 함수 */
void Config(ProcessData p[], int pNum, int qNum, ...);
void Schedule(int alg_id, ProcessData p[], int pNum, int timeQuantum);

/* queue.c      큐 생성 및 조작 관련 함수 */
void InitQueue(Queue *q);
void Enqueue(Queue *q, ProcessData *p);
ProcessData* Dequeue(Queue *q);
int CompareProcess(ProcessData *a, ProcessData *b, int criteria);
void SortReadyQueue(Queue *q, int criteria);

/* scheduler.c  스케줄링 알고리즘 함수 */
void FCFS(Queue *ready, ProcessData **running, int current_time);
void SJF(Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state);
void Priority(Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state);
void RoundRobin(Queue *ready, ProcessData **running, int current_time, int time_quantum, int *occupancy_time, Queue *state);
void LotteryScheduling(Queue *ready, ProcessData **running, int current_time, int time_quantum, int *occupancy_time, Queue *state);
void LongestIOFirst(Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state);

/* display.c    프로세스 생성 결과, 스케줄링 결과(간트 차트 및 성능) 등 출력 관련 함수 */
void DisplayProcessInfo(ProcessData p[], int pNum);
void PrintGanttChart(ProcessData p[], int pNum, Queue *state_queue);
void Evaluation(int alg_id, ProcessData p[], int pNum);

#endif
