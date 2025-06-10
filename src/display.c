#include "main.h"
// 프로세스 생성 결과 출력함수
void DisplayProcessInfo(ProcessData p[], int pNum) {
    printf("\n===============================================================\n");
    printf("| PID | Arrival | Priority | Burst |  I/O info (timing, burst) |\n");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < pNum; i++) {
        printf("| %2d  | %4d    | %5d    | %3d   |",
               p[i].pid, p[i].arrival_time, p[i].priority, p[i].cpu_burst);

        for (int j = 0; j < MAX_IO_NUM; j++) {
            if (p[i].io_timing[j] == -1) {
                if (j == 0) {
                    printf("  N/A");
                }
                break;
            }
            printf(" (%d,%d)", p[i].io_timing[j], p[i].io_burst[j]);
        }
        printf("\n");
    }
}
// 스케줄링 결과 간트 차트 출력함수
void PrintGanttChart(ProcessData p[], int pNum, Queue *state_queue) {
    int chartSize = 0;
    for (int i = 0; i < pNum; i++) {
        if (p[i].completion_time > chartSize) {
            chartSize = p[i].completion_time;
        }
    }

    int *pids = malloc(sizeof(int)*(chartSize + 1));
    char *states = malloc(sizeof(char)*(chartSize + 1));
    for (int i = 0; i <= chartSize; i++) {
        pids[i] = -1;
        states[i] = ' ';
    }

    for (Node *node = state_queue->front; node != NULL; node = node->next) {
        int t = node->time;
        if (t <= chartSize) {
            if (node->p_process == NULL) {
                pids[t] = -2;
                states[t] = node->data;
            } else {
                pids[t] = node->p_process->pid;
                states[t] = node->data;
            }
        }
    }
    printf("======================================================================================\n");
    printf("Gantt Chart     (T: terminated, I: I/O request, P: preempted, E: time quantum expired)\n");
    printf("Total scheduling time : %d\n", chartSize);
    // upper bar
    printf("       _");
    for (int t = 1; t <= chartSize; t++) {
        printf("__");
    }
    printf("\n");
    // pid
    printf("process|");
    for (int t = 1; t <= chartSize; t++) {
        if (pids[t] == -2) printf("X|");
        else if (pids[t] == -1) printf("  ");
        else printf("%d|", pids[t]);
    }
    printf("\n");
    // states and lower bar
    printf("event  |");
    for (int t = 1; t <= chartSize; t++) {
        if (states[t] == 'b') printf("_|");
        else if (states[t] == ' ') printf("__");
        else printf("%c|", states[t]);
    }
    printf("\n");
    // time
    printf("time   0");
    for (int t = 1; t <= chartSize; t++) {
        if (pids[t] != -1) printf("%2d", t);
        else printf("  ");
    }
    printf("\n\n");

    free(pids);
    free(states);
}
// 스케줄링 알고리즘 성능 출력함수
void Evaluation(int alg_id, ProcessData p[], int pNum) {
    float avg_waiting_time = 0;
    float avg_turnaround_time = 0;
    float avg_response_time = 0;

    for (int i = 0; i < pNum; i++) {
        avg_waiting_time += p[i].waiting_time;
        avg_turnaround_time += p[i].completion_time - p[i].arrival_time;
        avg_response_time += p[i].response_time;
    }
    avg_waiting_time = avg_waiting_time / (float)pNum;
    avg_turnaround_time = avg_turnaround_time / (float)pNum;
    avg_response_time = avg_response_time / (float)pNum;

    printf("Performance of ");
    switch (alg_id) {
        case 0:
            printf("FCFS");
            break;
        case 1:
            printf("Nonpreemptive SJF");
            break;
        case 2:
            printf("Preemptive SJF");
            break;
        case 3:
            printf("Nonpreemptive Priority");
            break;
        case 4:
            printf("Preemptive Priority");
            break;
        case 5:
            printf("RoundRobin");
            break;
        case 6:
            printf("Lottery Scheduling");
            break;
        case 7:
            printf("Nonpreemptive Longest I/O Burst First");
            break;
        case 8:
            printf("Preemptive Longest I/O Burst First");
            break;
    }
    printf(" algorithm\n");
    printf("average waiting time    : %.2f      (", avg_waiting_time);
    for (int i = 0; i < pNum; i++) {
        if (i == pNum - 1) {
            printf("%d)\n", p[i].waiting_time);
        } else printf("%d, ", p[i].waiting_time);
    }
    printf("average turnaround time : %.2f      (", avg_turnaround_time);
    for (int i = 0; i < pNum; i++) {
        if (i == pNum - 1) {
            printf("%d)\n", p[i].completion_time - p[i].arrival_time);
        } else printf("%d, ", p[i].completion_time - p[i].arrival_time);
    }
    printf("average response time   : %.2f      (", avg_response_time);
    for (int i = 0; i < pNum; i++) {
        if (i == pNum - 1) {
            printf("%d)\n\n\n", p[i].response_time);
        } else printf("%d, ", p[i].response_time);
    }
}