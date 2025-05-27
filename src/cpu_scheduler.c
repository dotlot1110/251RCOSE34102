#include <stdio.h>
#include <stdlib.h> //for srand()
#include <time.h>  //for time()

#define MAX_PROCESSES 10
#define MAX_TIME_QUANTUM 5
#define SCHEDULERS 6 //구현한 스케줄러 개수
#define MAX_IO_NUM 2

typedef struct {
    // Process data
    int pid;
    int arrival_time;
    int priority;
    int cpu_burst;
    int io_timing[MAX_IO_NUM];
    int io_burst[MAX_IO_NUM];
    // Runtime data
    int remaining_time; // cpu_burst로 초기화, 남은 cpu burst 시간. SJF에서 이걸 기준으로 정렬
    int remaining_io; // -1로 초기화, I/O 발생 시각에 burst 값으로 바뀌고 1씩 감소하여 0이 되면 ready로 복귀
    int waiting_time; // ready queue에서 머무른 시간, turnaround - cpu_burst - io_burst
    int response_time; // -1로 초기화, 최초로 running 상태가 될 때의 시각 - arrival time
    int completion_time; // -1로 초기화, 프로세스 완료 시각, turnaround = completion - arrival
} ProcessData;
/*============================================================================================================*/
void shuffle(int *arr, int n) { // I/O 발생 시간을 랜덤으로 선택하기 위한 함수
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

int compare(const void* a, const void* b) { //qsort의 정렬 기준을 오름차순으로 하기 위한 함수
	return(*(int *)a - *(int *)b);
}

void AssignProcessValues(ProcessData *p, int i) {
    p->pid = i;
    p->arrival_time = rand() % 11; // 0 ~ 10
    p->priority = rand() % 5 + 1; // 1 ~ 5, higher number the higher priority
    p->cpu_burst = rand() % 10 + 1; // 1 ~ 10

    for (int j = 0; j < MAX_IO_NUM; j++) {
        p->io_timing[j] = -1; // io 발생 횟수를 벗어난 값은 -1로 초기화
        p->io_burst[j] = 0; // io burst를 0으로 초기화
    }

    int maxIo = (MAX_IO_NUM < p->cpu_burst - 1) ? MAX_IO_NUM : p->cpu_burst - 1;
    int ioNum = rand() % (maxIo + 1); // 0 ~ maxIo
    //int timing[p->cpu_burst - 1]; 
    int *timing = (int *)malloc(sizeof(int) * (p->cpu_burst - 1));
    for (int j = 0; j < p->cpu_burst - 1; j++) {
        timing[j] = j + 1;
    }

    shuffle(timing, p->cpu_burst - 1);
    qsort(timing, ioNum, sizeof(int), compare);

    for (int j = 0; j < ioNum; j++) {
        p->io_timing[j] = timing[j];
        p->io_burst[j] = rand() % 3 + 1;      
    }
    free(timing);
}

void DisplayProcessInfo(ProcessData *p, int pNum) {
    printf("===============================================================\n");
    printf("| PID | Arrival | Priority | Burst |  I/O info (timing, burst) |\n");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < pNum; i++) {
        printf("| %3d |   %5d |     %2d   |  %4d |",
               p[i].pid, p[i].arrival_time, p[i].priority, p[i].cpu_burst);

        for (int j = 0; j < MAX_IO_NUM; j++) {
            if (p[i].io_timing[j] == -1) {
                if (j == 0) {
                    printf(" N/A");
                }
                break;
            }
            printf(" (%d,%d)", p[i].io_timing[j], p[i].io_burst[j]);
        }
        printf("\n");
    }
    printf("===============================================================\n\n");
}

void CreateProcess(ProcessData p[], int pNum) {
    srand(time(NULL));
    for (int i = 0; i < pNum; i++) {
        AssignProcessValues(&p[i], i);
    }
    DisplayProcessInfo(p, pNum);
}
/*============================================================================================================*/
typedef struct Node {
    ProcessData *p_process;
    struct Node *next;
    // Gantt차트 출력 보조
    int time;
    char data;
} Node;

typedef struct {
    Node *front;
    Node *rear;
} Queue;

void InitQueue(Queue *q) {
    q->front = q->rear = NULL;
}

void Enqueue(Queue *q, ProcessData *p) {
	Node *newNode = malloc(sizeof(Node));
    newNode->p_process = p;
	newNode->next = NULL;
	
	if (q->rear == NULL) { // Empty
		q->front = newNode;
		q->rear = newNode;
	}
	else {
		q->rear->next = newNode;
		q->rear = newNode;
	}
}

ProcessData* Dequeue(Queue *q) {
    if (q->front == NULL) { // Empty
        return NULL;
    } 

    Node *temp = q->front;
    ProcessData *returnProcess = temp->p_process;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
        
    free(temp);
    return returnProcess;
}

void InitRuntimeData(ProcessData p[], int pNum) {
    // Process data 영역은 job queue라고 생각
    for (int i = 0; i < pNum; i++) {
        p[i].remaining_time = p[i].cpu_burst;
        p[i].remaining_io = -1;
        p[i].waiting_time = 0;
        p[i].response_time = -1;
        p[i].completion_time = -1;
    }
}

void Config(ProcessData p[], Queue *readyQ, Queue *waitQ, int pNum) {
    InitRuntimeData(p, pNum);
    InitQueue(readyQ);
    InitQueue(waitQ);
}

int IsAllTerminated(ProcessData p[], int pNum) {
    for (int i = 0; i < pNum; i++) {
        if (p[i].completion_time == -1) {
            return 0;
        }
    }
    return 1;
}
/*============================================================================================================*/
void FCFS(ProcessData p[], int pNum, Queue *ready, ProcessData **running, int current_time) {
    if (*running == NULL && ready->front != NULL) { // 레디큐에 프로세스가 존재하고 실행중인 프로세스 없음
        *running = Dequeue(ready);
        //printf("[%2d] process %d : ready -> running\n", current_time, (*running)->pid); // log for debugging
    }
}

void SJF(ProcessData p[], int pNum, Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state) { // remaining_time 사용
    // ready queue가 비어 있는 경우
    if (ready->front == NULL) return;

    // ready queue에서 shortest job 탐색
    Node *shortest = ready->front;
    Node *prev = NULL, *now = ready->front, *prev_shortest = NULL;

    while (now != NULL) {
        if (now->p_process->remaining_time < shortest->p_process->remaining_time) {
            shortest = now;
            prev_shortest = prev;
        }
        prev = now;
        now = now->next;
    }

    // preemptive
    if (preemptive && *running != NULL) {
        if (shortest->p_process->remaining_time >= (*running)->remaining_time) return; // running이 더 짧으면 x

        // shortest가 더 짧을 경우
        Enqueue(ready, *running);
        //printf("[%2d] process %d : running -> ready (preempted)\n", current_time, (*running)->pid); // log for debugging

        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'p';

        *running = NULL;
    }

    // next running
    if (*running == NULL) {
        // ready queue에서 shortest 노드 제거
        if (shortest == ready->front) { // shortest가 front면 간단
            ready->front = shortest->next;
            if (ready->front == NULL) ready->rear = NULL;
        } else {
            prev_shortest->next = shortest->next; // 저장중인 다른 노드를 활용하여 재연결
            if (shortest == ready->rear) ready->rear = prev_shortest;
        }

        *running = shortest->p_process;
        //printf("[%2d] process %d : ready -> running\n", current_time, (*running)->pid); // log for debugging
        free(shortest);
    }
}

void Priority(ProcessData p[], int pNum, Queue *ready, ProcessData **running, int current_time, int preemptive, Queue *state) { // priority 사용
    if (ready->front == NULL) return;

    // highest priority 탐색
    Node *highest = ready->front;
    Node *prev = NULL, *curr = ready->front, *prev_highest = NULL;

    while (curr != NULL) {
        if (curr->p_process->priority < highest->p_process->priority) { // 우선순위 같으면 front에 가까운 쪽이 계속 가져감
            highest = curr;
            prev_highest = prev;
        }
        prev = curr;
        curr = curr->next;
    }

    
    if (preemptive && *running != NULL) {
        if (highest->p_process->priority >= (*running)->priority) return;

        // preemption
        Enqueue(ready, *running);
        //printf("[%2d] process %d : running -> ready (preempted)\n", current_time, (*running)->pid); // log for debugging

        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'p';

        *running = NULL;
    }

    if (*running == NULL) {
        // ready 큐에서 highest 제거
        if (highest == ready->front) {
            ready->front = highest->next;
            if (ready->front == NULL) ready->rear = NULL;
        } else {
            prev_highest->next = highest->next;
            if (highest == ready->rear) ready->rear = prev_highest;
        }

        *running = highest->p_process;
        //printf("[%2d] process %d : ready -> running\n", current_time, (*running)->pid); // log for debugging
        free(highest);
    }
}

void RoundRobin(ProcessData p[], int pNum, Queue *ready, ProcessData **running, int current_time, int time_quantum, int *time_slice, Queue *state) {
    // Schedule의 for문 내부에서 tilme_slice를 통해 어떤 프로세스가 연속으로 점유한 시간을 계산
    // RoundRobin은 매번 time_slice를 받아 그 값이 time quantum을 초과하였는지 확인

    // 새로 running에 들어간 프로세스는 time_slice를 초기화해야 함
    if (*running == NULL && ready->front != NULL) {
        *running = Dequeue(ready);
        *time_slice = 0;
        //printf("[%2d] process %d : ready -> running\n", current_time, (*running)->pid); // log for debugging
    }

    // running 프로세스가 할당된 시간보다 오래 실행되었으면 선점
    if (*running != NULL && *time_slice >= time_quantum) {
        //printf("[%2d] process %d : running -> ready (time slice expired)\n", current_time, (*running)->pid); // log for debugging
        Enqueue(ready, *running);

        Enqueue(state, *running);
        state->rear->time = current_time;
        state->rear->data = 'e';

        *running = NULL;

        // 실행 순서는 FCFS랑 동일
        if (ready->front != NULL) {
            *running = Dequeue(ready);
            *time_slice = 0;
            //printf("[%2d] process %d : ready -> running\n", current_time, (*running)->pid); // log for debugging
        }
    }
}



void PrintGanttChart(ProcessData p[], int pNum, Queue *state_queue) {
    Node *node = state_queue->front;
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
            } else {
                pids[t] = node->p_process->pid;
                states[t] = node->data;
            }
        }
    }
    printf("===========================================================================\n");
    printf("Gantt Chart     (t: terminated, i: I/O interrupt, p: preempted, e: expired)\n");
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
        if (pids[t] == -2) printf("i|");
        else if (pids[t] == -1) printf("  ");
        else printf("%d|", pids[t]);
    }
    printf("\n");
    // states and lower bar
    printf("state  |");
    for (int t = 1; t <= chartSize; t++) {
        if (pids[t] == -2) printf("_|");
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

    // free
    free(pids);
    free(states);
}

void Schedule(int alg_id, ProcessData p[], int pNum, int tq) {
    Queue ready_queue;
    Queue wait_queue;
    ProcessData *running = NULL;
    Config(p, &ready_queue, &wait_queue, pNum);
    int time_slice = 0;
    int is_idle[2] = {1, 1};

    //출력 보조
    Queue state_queue;
    InitQueue(&state_queue);

    for (int current_time = 0; !IsAllTerminated(p, pNum); current_time++) {
        // 이번 시간 시작
            // new arrival
        for (int i = 0; i < pNum; i++) {
            if (p[i].arrival_time == current_time) {
                //printf("[%2d] process %d arrived\n", current_time, p[i].pid); // Arrival // log for debugging
                Enqueue(&ready_queue, &p[i]);
            }
        }

        // 이번 시간 진행 중
            // ready
        for (Node *node = ready_queue.front; node != NULL; node = node->next) {
            node->p_process->waiting_time++;
        }
            // running
        if (running != NULL) {
            running->remaining_time--;
            time_slice++;
            is_idle[0] = 0;
        } 
        if (running == NULL) {
            is_idle[0] = 1;
        }
            // waiting
        for (Node *node = wait_queue.front; node != NULL; node = node->next) {
            node->p_process->remaining_io--;
        }

        // 이번 시간 결과 정산 + 다음 시간 상태 준비
            // running
        if (running != NULL) {
                // termination
            if (running->remaining_time == 0) { 
                running->completion_time = current_time;
                //printf("[%2d] process %d terminated\n", current_time, running->pid); // Termination // log for debugging

                Enqueue(&state_queue, running);
                state_queue.rear->time = current_time;
                state_queue.rear->data = 't';

                running = NULL;
                time_slice = 0;
            } else {
                // io event 
                int progress = running->cpu_burst - running->remaining_time;
                for (int i = 0; i < MAX_IO_NUM; i++) { // progress가 io timing 도달했는지 확인
                    if (running->io_timing[i] == -1) {
                        break;
                    }
                    if (running->io_timing[i] == progress) {
                        running->remaining_io = running->io_burst[i];
                        //printf("[%2d] process %d : running -> waiting\n", current_time, running->pid); // I/O interrupt // log for debugging
                        Enqueue(&wait_queue, running);

                        Enqueue(&state_queue, running);
                        state_queue.rear->time = current_time;
                        state_queue.rear->data = 'i';

                        running = NULL;
                        time_slice = 0;
                        break;
                    }
                }
            }
        }
            // waiting
        Queue new_wait_queue; // wait queue 최신화
        InitQueue(&new_wait_queue);

        for (Node *node = wait_queue.front; node != NULL; ) { // 현재 wait queue 순회
            Node *next = node->next; // next값 미리 저장
            if (node->p_process->remaining_io == 0) { // io 종료 프로세스는 ready queue에 enque하고 원래 노드 삭제
                //printf("[%2d] process %d : waiting -> ready\n", current_time, node->p_process->pid); // I/O completion // log for debugging
                Enqueue(&ready_queue, node->p_process);
                free(node); 
            } else { // 계속 남아있을 노드들은 기존 연결을 제거 후 새로운 큐에 enque하고 원래 노드 삭제
                node->next = NULL;
                Enqueue(&new_wait_queue, node->p_process);  
                free(node); 
            }
            node = next; // 다음 노드 지정
        }
        wait_queue = new_wait_queue; // wait queue의 front와 rear 변경
            // next running
        switch (alg_id) {
            case 0:
                FCFS(p, pNum, &ready_queue, &running, current_time);
                break;
            case 1:
                SJF(p, pNum, &ready_queue, &running, current_time, 0, &state_queue);
                break;
            case 2:
                SJF(p, pNum, &ready_queue, &running, current_time, 1, &state_queue);
                break;
            case 3:
                Priority(p, pNum, &ready_queue, &running, current_time, 0, &state_queue);
                break;
            case 4:
                Priority(p, pNum, &ready_queue, &running, current_time, 1, &state_queue);
                break;
            case 5:
                RoundRobin(p, pNum, &ready_queue, &running, current_time, tq, &time_slice, &state_queue);
                break;
            default:
                break;
        }

        if (running == NULL && !IsAllTerminated(p, pNum)) {
            //printf("[%2d] idle\n", current_time); // log for debugging
            is_idle[1] = 1;
        } else is_idle[1] = 0;
        if (is_idle[0] == 1 && is_idle[1] == 0) {
            Enqueue(&state_queue, NULL);
            state_queue.rear->time = current_time;
        }
    }
    PrintGanttChart(p, pNum, &state_queue);

    //debugging
    /*
    for (int i = 0; i < pNum; i++) {
        printf("completion time : %d\n", p[i].completion_time);
    }
    */
    
}
/*============================================================================================================*/
/*=====Evaluation=====*/
void Evaluation(int alg_id, ProcessData p[], int pNum) {
    float avg_waiting_time = 0;
    float avg_turnaround_time = 0;

    for (int i = 0; i < pNum; i++) {
        avg_waiting_time += p[i].waiting_time;
        avg_turnaround_time += p[i].completion_time - p[i].arrival_time;
    }
    avg_waiting_time = avg_waiting_time / (float)pNum;
    avg_turnaround_time = avg_turnaround_time / (float)pNum;
    printf("Performance of ");
    switch (alg_id) {
        case 0:
            printf("FCFS\n");
            break;
        case 1:
            printf("Nonpreemptive SJF\n");
            break;
        case 2:
            printf("Preemptive SJF\n");
            break;
        case 3:
            printf("Nonpreemptive Priority\n");
            break;
        case 4:
            printf("Preemptive Priority\n");
            break;
        case 5:
            printf("RoundRobin\n");
            break;
    }
    printf("average waiting time    : %.2f\n", avg_waiting_time);
    printf("average turnaround time : %.2f\n\n\n", avg_turnaround_time);
}
/*============================================================================================================*/
/*=====main=====*/
int main(void) {
    int processNum, timeQuantum, alg_id;
    ProcessData processes[MAX_PROCESSES];

    printf("total process (1~10) : ");
    scanf("%d", &processNum);
    if (processNum < 1 || processNum > MAX_PROCESSES) {
        printf("out of range\n");
        return 0;
    }
    printf("time quantum (1~5) : ");
    scanf("%d", &timeQuantum);
    if (timeQuantum < 1 || timeQuantum > MAX_TIME_QUANTUM) {
        printf("out of range\n");
        return 0;
    }
    /* by mode (debugging) 
    printf("scheduling algorithm id (0~5) : ");
    scanf("%d", &alg_id);
    if (alg_id < 0 || alg_id >= SCHEDULERS) {
        printf("out of range\n");
        return 0;
    }

    CreateProcess(processes, processNum);
    Schedule(alg_id, processes, processNum, timeQuantum);
    Evaluation(alg_id, processes, processNum);
    */

    CreateProcess(processes, processNum);
    for (int i = 0; i < SCHEDULERS; i++) {
        Schedule(i, processes, processNum, timeQuantum);
        Evaluation(i, processes, processNum);
    }
    return 0;
}