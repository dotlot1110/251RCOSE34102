#include "main.h"
// 스케줄링 결과값 초기화 및 입력받은 모든 큐 초기화
void Config(ProcessData p[], int pNum, int qNum, ...) {
    InitRuntimeData(p, pNum);

    va_list args;
    va_start(args, qNum);

    for (int i = 0; i < qNum; i++) {
        Queue *q = va_arg(args, Queue*);
        InitQueue(q);
    }
    va_end(args);
}
// 시뮬레이션 환경 설정 및 시간 단위 루프. 루프 탈출 후 스케줄링 결과를 간트차트로 출력
void Schedule(int alg_id, ProcessData p[], int pNum, int timeQuantum) {
    Queue ready_queue, wait_queue, state_queue;
    ProcessData *running = NULL;
    Config(p, pNum, 3, &ready_queue, &wait_queue, &state_queue);
    int occupancy_time = 0;
    int prev_running = 0;

    for (int current_time = 0; !IsAllTerminated(p, pNum); current_time++) {
        // 1. update values
            // waiting
        for (Node *node = wait_queue.front; node != NULL; node = node->next) {
            node->p_process->remaining_io--;
        }
            // running
        if (running != NULL) {
            running->remaining_time--;
            occupancy_time++;
        }
            // ready
        for (Node *node = ready_queue.front; node != NULL; node = node->next) {
            node->p_process->waiting_time++;
        }

        // 2. update states
            // job scheduling (new -> ready)
        for (int i = 0; i < pNum; i++) {
            if (p[i].arrival_time == current_time) {
                Enqueue(&ready_queue, &p[i]); // pid 순으로 enqueue (동시 도착 시 작은 pid 우대)
            }
        }
            // waiting
        Queue new_wait_queue;
        InitQueue(&new_wait_queue);

        for (Node *node = wait_queue.front; node != NULL; ) {
            Node *next = node->next;
            if (node->p_process->remaining_io == 0) {
                // -> ready : ready queue에 노드 추가 후 현재 노드 삭제
                Enqueue(&ready_queue, node->p_process);
                free(node); 
            } else { 
                // still waiting : 새 waiting queue에 노드 추가 후 기존 노드 삭제
                node->next = NULL;
                Enqueue(&new_wait_queue, node->p_process);  
                free(node); 
            }
            node = next;
        }
        wait_queue = new_wait_queue;
            // running
        if (running != NULL) {
                // -> terminated
            if (running->remaining_time == 0) { 
                running->completion_time = current_time;

                Enqueue(&state_queue, running);
                state_queue.rear->time = current_time;
                state_queue.rear->data = 'T'; // terminated

                running = NULL;
            } else {
                // -> waiting
                int progress = running->cpu_burst - running->remaining_time;
                for (int i = 0; i < MAX_IO_NUM; i++) { 
                    if (running->io_timing[i] == -1) {
                        break; 
                    }
                    if (running->io_timing[i] == progress) {
                        running->remaining_io = running->io_burst[i];
                        running->total_remaining_io -= running->io_burst[i];
                        Enqueue(&wait_queue, running);

                        Enqueue(&state_queue, running);
                        state_queue.rear->time = current_time;
                        state_queue.rear->data = 'I'; // I/O request

                        running = NULL;
                        break;
                    }
                }
            }
        }

            // running -> ready, ready -> running
        switch (alg_id) {
            case 0:
                FCFS(&ready_queue, &running, current_time);
                break;
            case 1: // nonpreemptive
                SJF(&ready_queue, &running, current_time, 0, &state_queue);
                break;
            case 2: // preemptive
                SJF(&ready_queue, &running, current_time, 1, &state_queue);
                break;
            case 3: // nonpreemptive
                Priority(&ready_queue, &running, current_time, 0, &state_queue);
                break;
            case 4: // preemptive
                Priority(&ready_queue, &running, current_time, 1, &state_queue);
                break;
            case 5:
                RoundRobin(&ready_queue, &running, current_time, timeQuantum, &occupancy_time, &state_queue);
                break;
            case 6:
                LotteryScheduling(&ready_queue, &running, current_time, timeQuantum, &occupancy_time, &state_queue);
                break;
            case 7: // nonpreemptive
                LongestIOFirst(&ready_queue, &running, current_time, 0, &state_queue);
                break;
            case 8: // preemptive
                LongestIOFirst(&ready_queue, &running, current_time, 1, &state_queue);
            default:
                break;
        }

        // state tracking
        if (running != NULL) {
            if (running->response_time == -1) {
                running->response_time = current_time - running->arrival_time;
            }
            if (!prev_running) {
                Enqueue(&state_queue, NULL);
                state_queue.rear->time = current_time;
                state_queue.rear->data = 'b'; // CPU going Busy
            }   
        }
        prev_running = (running != NULL);
    }
    PrintGanttChart(p, pNum, &state_queue);
}