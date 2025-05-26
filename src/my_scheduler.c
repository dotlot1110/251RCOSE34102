#include <stdio.h>
#include <stdlib.h> //for srand()
#include <time.h>  //for time()

#define MAX_PROCESSES 10
#define SCHEDULERS 6 //구현한 스케줄러 개수에 따라
#define MAX_IO_NUM 2

// 1. input process number, time quantum (main())
// 2. use input values to create processes (Create_Process())
// 3. configure(initialize) the system (Config())
// 4. schedule the processes with several algorithms then display Gantt chart (Schedule())
// 5. evaluate the performance (Evaluation())

typedef struct {
    int pid;
    int arrival_time;
    int priority;
    int cpu_burst;
    int io_timing[MAX_IO_NUM];
    int io_burst[MAX_IO_NUM];
} process;
/*==========================================================================================*/

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

void AssignProcessValues(process *p, int i) {
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

void DisplayProcessInfo(process *p, int pNum) {
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


void Create_Process(process p[], int pNum) {
    srand(time(NULL));
    for (int i = 0; i < pNum; i++) {
        AssignProcessValues(&p[i], i);
    }
    DisplayProcessInfo(p, pNum);
}
/*
int Config(process p, int pNum) {
    //초기 생성한 프로세스 정보 복사+불러오기

}

void sortReadyQueue() {

}

int FCFS() {

}

int SJF() {

}


int Gantt() {

}


void Schedule(int alg_id, process p, int pNum, int tq) {
    //알고리즘 이름 출력 및 스케줄링 실행
    switch (alg_id) {
        case 0:
            printf("FCFS algorithm /n");
            FCFS(p, pNum);
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

    //간트차트 그리는 부분
    Gantt();

}

void Evaluation() {

}
*/
/*==========================================================================================*/
int main(void) {
    int processNum, timeQuantum;
    process processes[MAX_PROCESSES];

    printf("total process (1~10) : ");
    scanf("%d", &processNum);
    if (processNum < 1 || processNum > MAX_PROCESSES) {
        printf("out of range\n");
        return 0;
    }

    Create_Process(processes, processNum);
    /*
    for (int i = 0; i < SCHEDULERS; i++) {
        Config();
        Schedule(i, process, processNum, timeQuantum)
        Evaluation();
    }
    */
    return 0;
}
