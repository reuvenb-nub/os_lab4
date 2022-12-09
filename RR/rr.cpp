#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
using namespace std;

struct PCB{
    int pID;
    int arrival_time;
    int burst_time;
    int finnish_time;
    int remaining_time;
    PCB* pNext;
};

struct list {
    PCB* pHead;
    PCB* pTail;
};

list statistics_list;
list ready_queue;
list new_queue;
int system_time = 0;
int previous_time = 0;
int quantum_time = 0;
fstream myFile;

void init(list &l) {
    l.pHead = NULL;
    l.pTail = NULL;
}

PCB* create_task(int pID,int arrival_time, int burst_time, int remaining_time = 0) {
    PCB* p = new PCB;
    if (p == NULL) exit(1);
    p->pID = pID;
    p->arrival_time = arrival_time;
    p->burst_time = burst_time;
    p->remaining_time = burst_time;
    p->pNext = NULL;
    return p;
}

void add_task(list &l, PCB *p) {
    if (l.pHead == NULL) {
        l.pHead = p;
        l.pTail = p;
    }
    else {
        l.pTail->pNext = p;
        l.pTail = p;
    }
}

PCB* find_task(list &l, int pID) {
    PCB* p = l.pHead;
    for (; p; p = p->pNext) {
        if (p->pID == pID) return p;
    }
    return NULL;
}

void remove_task(list &l, PCB *pa) {
    PCB* p = l.pHead;
    if (p != NULL && pa != NULL) {
        if (l.pHead == pa) {
            PCB *p_temp = l.pHead;
            l.pHead = l.pHead->pNext;
            p_temp->pNext = NULL;
            delete p_temp;
            if (l.pHead == NULL)
                l.pTail = NULL;
        } 
        for (; p; p = p->pNext) {
            if (p->pNext == pa) {
                PCB *p_temp = p->pNext;
                p->pNext = p->pNext->pNext;
                p_temp->pNext = NULL;
                delete p_temp;
                // if (p->pNext == l.pTail) {
                //     l.pTail = p;
                // }
            }
        }
    }
    delete p;
}

void create_list(list &l) {
    int pID;
    int arrival_time;
    int burst_time;
    char continued;
    do{
        myFile >> pID;
        myFile >> arrival_time;
        myFile >> burst_time;
        add_task(l, create_task(pID, arrival_time, burst_time));
        myFile >> continued;
    }while(continued == 'y');
}

bool is_increasing() {
    if (system_time > previous_time) {
		previous_time++;
		return true;
	}
	else return false;
}

void statistics() {
    for (PCB *p = statistics_list.pHead; p != NULL; p = p->pNext) {
        cout << p->pID << " " << p->arrival_time << " " << p->burst_time << " " << p->finnish_time << endl;
    }
}

void* wait(void* message) {
    while (1) {
        sleep(1);
        system_time++;
        if (ready_queue.pHead == NULL && new_queue.pHead == NULL) {
            statistics();
            return message;
        }
    }
}

void terminate_task(PCB *task) {
    for (PCB *p = statistics_list.pHead; p != NULL; p = p->pNext) {
        if (p->pID == task->pID) {
            p->finnish_time = system_time;
        }
    }
    remove_task(ready_queue, task);
}

void execute(PCB *p) {
    if (p->remaining_time == 0)
        terminate_task(p);
    if (is_increasing()) {
        cout << p->pID << endl;
        p->remaining_time--;
    }
}

void* short_scheduler(void *message) {
    int timeout = 0;
    while (1) {
        if (ready_queue.pHead != NULL) {
            if (timeout == quantum_time) {
                timeout = 0;
                add_task(ready_queue, create_task(ready_queue.pHead->pID,
                                                ready_queue.pHead->arrival_time,
                                                ready_queue.pHead->burst_time, 
                                                ready_queue.pHead->remaining_time));
                remove_task(ready_queue, ready_queue.pHead);
            }
        }
        if (is_increasing()) {
            timeout++;
        }
        execute(ready_queue.pHead);
    }
}

void* long_scheduler(void *message) {
    while (1) {
        for (PCB *p = new_queue.pHead; p != NULL; p = p->pNext) {
            if (p->arrival_time == system_time) {
                add_task(ready_queue, create_task(p->pID, p->arrival_time, p->burst_time));
                add_task(statistics_list, create_task(p->pID, p->arrival_time, p->burst_time));
                remove_task(new_queue, p);
            }
        }
    }
}


int main(){
    pthread_t tid1;
    pthread_t tid2;
    init(new_queue);
    init(ready_queue);
    init(statistics_list);
    myFile.open("input.txt", ios::in);
    myFile >> quantum_time;
    create_list(new_queue);
    myFile.close();
    cout << "Grant diagram:\n";

    pthread_create(&tid1, NULL, &long_scheduler, NULL);
    pthread_create(&tid1, NULL, &wait, NULL);
    pthread_create(&tid2, NULL, &short_scheduler, NULL);

    pthread_join(tid1, NULL);
    return 0;
}