#include "main.h"

int main(void) {
    int processNum, timeQuantum = 0, mode, alg_id = -1;
    ProcessData processes[MAX_PROCESSES];
    // 실행 모드 선택 (전체 알고리즘 또는 특정 알고리즘)
    printf("Simulation mode (0: Schedule by all algorithms, 1: Schedule by specific algorithm) : ");
    scanf("%d", &mode);
    // 단일 알고리즘 실행 시 선택
    if (mode == 1) {
        printf("(0: FCFS, 1: N_SJF, 2: P_SJF, 3: N_Pri, 4: P_Pri, 5: RR, 6: Lottery, 7: N_LongestIOFirst, 8: P_LongestIOFirst)\n");
        printf("scheduling algorithm id(0 ~ %d) : ", SCHEDULERS - 1);
        scanf("%d", &alg_id);
        if (alg_id < 0 || alg_id >= SCHEDULERS) {
            printf("out of range\n");
            return 0;
        }
    }
    // 생성할 프로세스 개수 입력
    printf("total process (1~10) : ");
    scanf("%d", &processNum);
    if (processNum < 1 || processNum > MAX_PROCESSES) {
        printf("out of range\n");
        return 0;
    }
    // time quantum이 필요한 경우 입력
    if (mode == 0 || alg_id == 5 || alg_id == 6) {
        printf("time quantum (1~5) : ");
        scanf("%d", &timeQuantum);
        if (timeQuantum < 1 || timeQuantum > MAX_TIME_QUANTUM) {
            printf("out of range\n");
            return 0;
        }
    }
    // 프로세스 생성
    CreateProcess(processes, processNum);
    // 시뮬레이션 수행
    if (mode == 0) { 
        for (int i = 0; i < SCHEDULERS; i++) {
            Schedule(i, processes, processNum, timeQuantum);
            Evaluation(i, processes, processNum);
        }
    } else if (mode == 1) { 
        Schedule(alg_id, processes, processNum, timeQuantum);
        Evaluation(alg_id, processes, processNum);
    } else {
        printf("something went wrong\n");
        return 0;
    }
    return 0;
}