#include "main.h"

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
// SortReadyQueue()의 정렬 기준 설정
int CompareProcess(ProcessData *a, ProcessData *b, int criteria) {
    switch (criteria) {
        case 0: // SJF : remaining_time 오름차순
            return a->remaining_time - b->remaining_time;
        case 1: // Priority : priority 오름차순
            return a->priority - b->priority;
        case 2: // I/O bound process first : total_remaining_io 내림차순
            return b->total_remaining_io - a->total_remaining_io;
    }
    return 0;
}
// 입력 기준값에 따라 일정한 기준으로 레디큐의 노드를 정렬
void SortReadyQueue(Queue *q, int criteria) {
    if (!q || !q->front || !q->front->next) return;
    Node *sorted = NULL; 
    // 앞에서부터 탐색, 크기가 같으면 앞에 삽입하므로 기준값이 같다면 FIFO
    while (q->front != NULL) {
        Node *insert = q->front;
        q->front = insert->next;
        insert->next = NULL;
        // 최초 시행 시 또는 (오름차순 기준) 자신이 가장 작은 경우
        if (!sorted || CompareProcess(insert->p_process, sorted->p_process, criteria) < 0) {
            insert->next = sorted; // insert가 sorted의 맨 앞으로 추가
            sorted = insert;
        } else {
            Node *prev = sorted;
            // (오름차순 기준) 자신보다 크거나 같은 값이 나올 때까지 뒤로 이동
            while (prev->next && CompareProcess(insert->p_process, prev->next->p_process, criteria) >= 0) 
                prev = prev->next;
            insert->next = prev->next;
            prev->next = insert;
        }
    }
    q->front = sorted;
    Node *rear = q->front;
    while (rear && rear->next) rear = rear->next;
    q->rear = rear;
}
