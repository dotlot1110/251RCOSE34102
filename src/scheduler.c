#include "main.h"

void FCFS(Queue *ready, ProcessData **running, int current_time) {
    if (*running == NULL && ready->front != NULL) {
        *running = Dequeue(ready);
    }
}

void SJF(Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state) {
    if (ready->front == NULL || (!preemptive && *running != NULL)) return;

    SortReadyQueue(ready, 0);

    if (preemptive && *running != NULL) {
        if (ready->front->p_process->remaining_time >= (*running)->remaining_time) return;
        // else preempted (running -> ready)
        Enqueue(ready, *running);
        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'P'; // preempted
        *running = NULL;
    }
    // ready -> running
    if (*running == NULL) {
        *running = Dequeue(ready);
    }
}

void Priority(Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state) {
    if (ready->front == NULL || (!preemptive && *running != NULL)) return;

    SortReadyQueue(ready, 1);

    if (preemptive && *running != NULL) {
        if (ready->front->p_process->priority >= (*running)->priority) return;
        // else preempted (running -> ready)
        Enqueue(ready, *running);
        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'P'; // preempted
        *running = NULL;
    }
    // ready -> running
    if (*running == NULL) {
        *running = Dequeue(ready);
    }
}

void RoundRobin(Queue *ready, ProcessData **running, int current_time,
                int time_quantum, int *occupancy_time, Queue *state) {
    // running -> ready (대기중인 프로세스가 없더라도 우선 중단)
    if (*running != NULL && *occupancy_time >= time_quantum) {
        Enqueue(ready, *running);
        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'E'; // time quantum expired
        *running = NULL;
    }
    // ready -> running
    if (*running == NULL && ready->front != NULL) {
        *running = Dequeue(ready);
        *occupancy_time = 0; // 초기화
    }
}

void LotteryScheduling(Queue *ready, ProcessData **running, int current_time,
                       int time_quantum, int *occupancy_time, Queue *state) {
    Node *winner = NULL;
    Node *prev = NULL;
    int total_tickets = 0;
    int ticket_count = 0;
    // running -> ready (대기중인 프로세스가 없더라도 우선 중단)
    if (*running != NULL && *occupancy_time >= time_quantum) {
        Enqueue(ready, *running);
        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'E'; // time quantum expired
        *running = NULL;
    }
    // ready queue가 비어 있지 않을 때 추첨 진행
    if (*running == NULL) {
        if (ready->front == NULL) return;
        else {
            winner = ready->front;
            // priority 값을 각 프로세스의 ticket 개수로 사용 (higher priority value higher probability)
            for (Node *node = ready->front; node != NULL; node = node->next) {
                total_tickets += node->p_process->priority;
            }
            int win = rand() % total_tickets;  

            for (Node *node = ready->front; node != NULL; node = node->next) {
                ticket_count += node->p_process->priority;
                if (win <= ticket_count) {
                    winner = node;
                    break;
                }
            prev = node;
            }
        }
    } 
    // winner를 ready queue에서 제거 후 연결 재조정
    if (*running == NULL) {
        if (winner == ready->front) {
            ready->front = winner->next;
            if (ready->front == NULL) ready->rear = NULL;
        } else if (prev != NULL) {
            prev->next = winner->next;
            if (winner == ready->rear) ready->rear = prev;
        }
        // ready -> running
        *running = winner->p_process;
        *occupancy_time = 0;
    }
}

void LongestIOFirst(Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state) {
    if (ready->front == NULL || (!preemptive && *running != NULL)) return;

    SortReadyQueue(ready, 2);

    if (preemptive && *running != NULL) {
        // 정렬된 ready queue 첫 번째 프로세스의 remaining I/O burst time이 실행중인 프로세스의 것보다 클 때 preempted
        if (CompareProcess(ready->front->p_process, (*running), 2) >= 0) return;
        // else preempted (running -> ready)
        Enqueue(ready, *running);
        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'P'; // 'p'reempted
        *running = NULL;
    }
    // ready -> running
    if (*running == NULL) {
        *running = Dequeue(ready);
    }
}