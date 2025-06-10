#include "main.h"
// Fisher-Yates shuffle, 셔플 후 앞에서부터 I/O request 횟수만큼 취하여 요청 발생 시점으로 설정
void shuffle(int *arr, int n) { 
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}
// qsort()의 정렬 기준 설정을 위한 함수
int compare(const void* a, const void* b) { 
	return(*(int *)a - *(int *)b);
}
// 프로세스 구조체 멤버 중 전체 시뮬레이션 동안 변하지 않는 값에 대한 설정
// 프로세스 배열의 i번째 프로세스에 데이터 부여
void AssignProcessValues(ProcessData *p, int i) {
    p->pid = i;
    p->arrival_time = rand() % 11; // 도착 시각 : 0 ~ 10
    p->priority = rand() % 10 + 1; // 우선순위 : 1 ~ 10, 1에 가까울수록 높은 우선순위
    p->cpu_burst = rand() % 10 + 1; // CPU burs time : 1 ~ 10

    for (int j = 0; j < MAX_IO_NUM; j++) {
        p->io_timing[j] = -1; // 유효하지 않은 값 구분 위해 -1로 초기화
        p->io_burst[j] = 0; // 특정 I/O 요청에 대한 작업시간. 0으로 초기화
    }
    // I/O burst
    int maxIo = (MAX_IO_NUM < p->cpu_burst - 1) ? MAX_IO_NUM : p->cpu_burst - 1; // CPU burst time이 매우 짧은 경우를 고려
    int ioNum = rand() % (maxIo + 1); // I/O request 횟수 : 0 ~ maxIo
    // 1. request 발생 후보 시점 (1 ~ cpu_burst - 1)을 동적 배열로 생성
    int *timing = (int *)malloc(sizeof(int) * (p->cpu_burst - 1));
    for (int j = 0; j < p->cpu_burst - 1; j++) {
        timing[j] = j + 1;
    }
    // 2. timing 전체 셔플 후 앞에서부터 request 발생 횟수(ioNum)만큼만 오름차순 정렬
    shuffle(timing, p->cpu_burst - 1);
    qsort(timing, ioNum, sizeof(int), compare);
    // 3. timing 배열의 앞쪽 ioNum개 시점을 request 발생 시점으로 설정, 작업시간은 1 ~ 3의 값 중 랜덤으로 설정
    for (int j = 0; j < ioNum; j++) {
        p->io_timing[j] = timing[j];
        p->io_burst[j] = rand() % 3 + 1;      
    }
    free(timing);
}
// 입력 프로세스 배열의 입력 개수만큼의 프로세스에 데이터 부여 후 결과 출력
void CreateProcess(ProcessData p[], int pNum) {
    srand(time(NULL));
    for (int i = 0; i < pNum; i++) {
        AssignProcessValues(&p[i], i);
    }
    DisplayProcessInfo(p, pNum);
}
// 프로세스 구조체 멤버 중 시뮬레이션 동안 변하는 값을 초기화
void InitRuntimeData(ProcessData p[], int pNum) {
    // Process data 영역은 일종의 job queue
    for (int i = 0; i < pNum; i++) {
        p[i].remaining_time = p[i].cpu_burst; // 남은 시간 : CPU burst time으로 초기화
        p[i].remaining_io = -1; // 남은 I/O 작업 대기시간 : -1로 초기화 후 필요 시 유효한 값으로 설정
        p[i].waiting_time = 0; // ready queue에서 대기한 시간 : 0으로 초기화
        p[i].response_time = -1; // 응답시간 : -1로 초기화 후 최초 CPU 할당 시 유효한 값으로 결정
        p[i].completion_time = -1; // 완료시간 : -1로 초기화하여 프로세스가 종료되지 않았음을 표시
        p[i].total_remaining_io = 0; // I/O bound 우선 알고리즘에서 사용, p[i]의 유효 io_burst[j]합산
        for (int j = 0; j < MAX_IO_NUM; j++) {
            if (p[i].io_timing[j] == -1) {
                break;
            }
            p[i].total_remaining_io += p[i].io_burst[j];
        } 
    }
}
// 시뮬레이션 루프 탈출 조건
int IsAllTerminated(ProcessData p[], int pNum) {
    for (int i = 0; i < pNum; i++) {
        if (p[i].completion_time == -1) {
            return 0;
        }
    }
    return 1;
}