#include <stdio.h>
#include <stdlib.h> //for srand()
#include <time.h>  //for time()

#define MAX_PROCESSES 10
#define SCHEDULERS 6 //구현한 스케줄러 개수에 따라
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

    int timing[p->cpu_burst-1]; 
    for (int j = 0; j < p->cpu_burst-1; j++) {
        timing[j] = j + 1;
    }

    shuffle(timing, p->cpu_burst - 1);
    qsort(timing, ioNum, sizeof(int), compare);
    for (int j = 0; j < ioNum; j++) {
        p->io_timing[j] = timing[j];
        p->io_burst[j] = rand() % 3 + 1;
    }      
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

typedef struct Node {
    ProcessData *p_process;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *rear;
} Queue;

// 1. input process number, time quantum (main())
// 2. use input values to create processes (Create_Process())
// 3. configure(initialize) the system (Config())
// 4. schedule the processes with several algorithms then display Gantt chart (Schedule())
// 5. evaluate the performance (Evaluation())

void InitQueue(Queue *q) {
    q->front = q->rear = NULL;
}

void Enqueue(Queue *q, ProcessData *p) {
	Node *newNode = (Node*)malloc(sizeof(Node));
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

void InitRuntimeData(ProcessData *p, int pNum) {
    // Process data 영역은 job queue라고 생각
    for (int i = 0; i < pNum; i++) {
        p[i].remaining_time = p[i].cpu_burst;
        p[i].remaining_io = -1;
        p[i].waiting_time = 0;
        p[i].response_time = -1;
        p[i].completion_time = -1;
    }
}

void Config(ProcessData *p, Queue *readyQ, Queue *waitQ, int pNum) {
    InitRuntimeData(p, pNum);
    InitQueue(readyQ);
    InitQueue(waitQ);
}


/*=====Schedule=====*/
/*
for (curr=0; !IsAllTerminated(*p, pnum); curr++) {
1. Runtime data 값 업데이트
1) remaining_time : running의 remaining_time --;
2) remaining_io : waiting의 remaining_io --;
3) waiting_time : ready의 waiting_time ++;

2. 다음 시간 동작을 위한 계산
- 매 반복(시간)마다 4가지 동작에 대한 해당사항을 체크
1) I/O completion (waiting -> ready) : CheckIOCompletion()
    waiting queue를 순회하면서 remaining_io가 0인 프로세스를 waiting에서 dequeue, ready에 enqueue
2) I/O interrupt (running -> waiting) : ISCPUAvailable()
3) termination (running -> terminated)
    2)와 3)은 하나의 함수로 체크, 종료 있으면 1, 없으면 0 반환. 종료하는 경우 completion_time 변경
    단 여기서는 dequeue하지 않고 running 결정 때 수행
4) admission (job queue -> ready) (enqueue) : CheckArrival()
    job queue를 순회하면서(pid 오름차순) current_time==arrival_time인 프로세스를 ready queue에 enqueue

- 레디큐를 정렬 (FCFS는 해당 없음) : SortReadyQueue()

- 다음 running 프로세스를 결정 : GetNextRunningProcess()
1) running이 바뀌는 경우 현재 ready queue의 front가 running이 되고 dequeue, 실행중이던 프로세스는 enqueue
2) running이 바뀌지 않는 경우 아무 일도 일어나지 않음

- current_time 증가

- 모든 프로세스 종료 확인 시 스케줄링 함수 종료
    IsAllTerminated 함수는 프로세스를 순회하면서 completion_time이 -1인 프로세스가 없으면 1, 있으면 0 반환.
}
*/
void Gantt(int time, int pid, int prev, int new) {
    printf("[%2d] process %d : %d -> %d\n", time, pid, prev, new);
}

int IsAllTerminated(ProcessData p[], int pNum) {
    for (int i = 0; i < pNum; i++) {
        if (p[i].completion_time == -1) {
            printf("%d\n", p[i].completion_time); // debugging
            return 0;
        }
    }
    return 1;
}
/*
void Preemption() {

}

void SortReadyQueue() { // FCFS는 따로 정렬이 필요 없음

}
*/

void FCFS(ProcessData p[], int pNum, Queue ready_queue, Queue wait_queue) {
    int running = -1; // 실행중인 프로세스의 pid, -1로 초기화

    for (int current_time = 0; !IsAllTerminated(p, pNum); current_time++) {
        
    // 값 업데이트
        // 1) remaining_time --
        if (running != -1) {
            p[running].remaining_time--;
        }
        // 2) remaining_io --
        for (Node *node = wait_queue.front; node != NULL; node = node->next) {
            if (node->p_process->pid != running && node->p_process->remaining_io > 0) {
                node->p_process->remaining_io --;
            }
        }
        // 3) waiting_time ++
        for (Node *node = ready_queue.front; node != NULL; node = node->next) {
            if (node->p_process->pid != running && node->p_process->completion_time == -1) {
                node->p_process->waiting_time ++;
            }
        }
    
        // I/O Completion
        for (Node *node = wait_queue.front; node != NULL; node = node->next) {
            if (node->p_process->pid != running && node->p_process->remaining_io == 0) {
                printf("[%2d] process %d : waiting -> ready\n", current_time, node->p_process->pid); // Gantt
                Dequeue(&wait_queue);
                Enqueue(&ready_queue, node->p_process);
            }
        }

    // Interrupt, termination 처리
        // I/O interrupt
        for (int i = 0; running != -1 && i < p[running].cpu_burst - 1; i++) {
            /*if (p[running].io_timing[i] == -1) {
                break;
            } else */
            if (p[running].io_timing[i] == p[running].cpu_burst - p[running].remaining_time) {
                p[running].remaining_io = p[running].io_burst[i];
                printf("[%2d] process %d : running -> waiting\n", current_time, p[running].pid); // Gantt
                // waiting queue로 이동
                Dequeue(&ready_queue);
                Enqueue(&wait_queue, &p[i]);
                running = -1;
            }
        }
        // termination
        if (running != -1 && p[running].remaining_time == 0) {
            p[running].completion_time = current_time; //completion_time 변경 (종료 표시)
            printf("[%2d] process %d : running -> terminated\n", current_time, p[running].pid); // Gantt
            // dequeue
            Dequeue(&ready_queue);
            running = -1;
        }

        // arrival time 체크
        for (int i = 0; i < pNum; i++) {
            if (p[i].arrival_time == current_time) {
                printf("[%2d] process %d : new -> ready\n", current_time, p[i].pid); // Gantt
                Enqueue(&ready_queue, &p[i]);
            }
        }

    // ready queue 정렬 : FCFS는 해당 없음

    // 다음 running 결정 (interrupt 또는 termination 시)
        if (running == -1 && ready_queue.front != NULL && ready_queue.front->p_process->completion_time == -1) {
            running = ready_queue.front->p_process->pid;
            printf("[%2d] process %d : ready -> running\n", current_time, p[running].pid); // Gantt
            // FCFS는 interrupt 없으므로 running -> ready 없음
            Dequeue(&ready_queue);
        }
        
        printf("Debug : %d", current_time); // debugging
    }
}

/*
int SJF() {
    // progress로 우선순위 큐(힙) 정렬

}

int Priority() {
    // priority로 우선순위 큐 정렬
}
*/
void Schedule(int alg_id, ProcessData p[], int pNum, int tq) {
    // ready queue, waiting queue 선언
    Queue ready_queue;
    Queue waiting_queue;
    // 프로세스 terminate 시 해당 노드 값들을 원래 프로세스 배열 값들에 저장하고 free
    // 큐 초기화
    Config(p, &ready_queue, &waiting_queue, pNum);

    //알고리즘 이름 출력 및 스케줄링 실행
    printf("FCFS algorithm \n");
    FCFS(p, pNum, ready_queue, waiting_queue);
    /*
    switch (alg_id) {
        case 0:
            printf("FCFS algorithm \n");
            FCFS(p, pNum, ready_queue, waiting_queue);
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        default:
            return;
    }
    */
}

/*=====Evaluation=====*/
void Evaluation(ProcessData *p, int pNum) {
    float avg_waiting_time = 0;
    float avg_turnaround_time = 0;

    for (int i = 0; i < pNum; i++) {
        avg_waiting_time += p[i].waiting_time;
        avg_turnaround_time += p[i].completion_time - p[i].arrival_time;
    }
    avg_waiting_time = avg_waiting_time / pNum;
    avg_turnaround_time = avg_turnaround_time / pNum;

    printf("average waiting time : %.2f\n", avg_waiting_time);
    printf("average turnaround time : %.2f\n", avg_turnaround_time);
}

/*=====main=====*/
int main(void) {
    int processNum, timeQuantum;
    ProcessData processes[MAX_PROCESSES];

    printf("total process (1~10) : ");
    scanf("%d", &processNum);
    if (processNum < 1 || processNum > MAX_PROCESSES) {
        printf("out of range\n");
        return 0;
    }

    CreateProcess(processes, processNum);
    Schedule(0, processes, processNum, timeQuantum);
    Evaluation(processes, processNum);


/*
    for (int i = 0; i < SCHEDULERS; i++) {
        Schedule(i, processes, processNum, timeQuantum);
        Evaluation(processes, processNum);
    }
*/
    return 0;
}