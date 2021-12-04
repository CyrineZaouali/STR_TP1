#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define NB_TRUCKS 5
#define NB_CARS 5
#define NB_VEHICULES (NB_TRUCKS + NB_CARS)

#define WAIT 1
#define NONE 2
#define CROSS 3

void wait(double max);
int random_draw(double max);

void* car(void* arg);
void* truck(void* arg);

pthread_mutex_t mutex;
sem_t sem[NB_VEHICULES];
int state[NB_VEHICULES];
int nb_waiting_trucks = 0;
int current_weight = 0;

void wait(double max){
    struct timespec delay;
    delay.tv_sec=random_draw(max);
    delay.tv_nsec=0;
    nanosleep(&delay,NULL);
}

int random_draw(double max){
    int j=(int) (max*rand()/(RAND_MAX+1.0));
    if (j<1)
        j=1;
    return j;
}

void cross_bridge(int tonnes, int id) {
    pthread_mutex_lock(&mutex);

    if(current_weight + tonnes <= 15) {
        current_weight += tonnes;
        state[id] = CROSS;
        sem_post(&sem[id]);
    } else {
        state[id] = WAIT;
        if(tonnes == 15) {
            nb_waiting_trucks++;
        }
    }
    pthread_mutex_unlock(&mutex);
    sem_wait(&sem[id]);
}

void leave_bridge(int tonnes, int id) {
    int i;

    pthread_mutex_lock(&mutex);

    state[id] = NONE;
    current_weight -= tonnes;

    for(i=0; i<NB_TRUCKS; i++) {
        if( (state[i] == WAIT) && (current_weight == 0)) {
            sem_post(&sem[i]);
            current_weight = 15;
            nb_waiting_trucks--;
        }
    }

    for(i=NB_TRUCKS; i<NB_VEHICULES; i++) {
        if( (current_weight < 15) && (nb_waiting_trucks == 0) && (state[i] == WAIT)) {
            current_weight += 5;
            sem_post(&sem[i]);
        }
    }

    pthread_mutex_unlock(&mutex);
}

void* car(void* arg){
    int id=*((int*)arg);
    wait(5.0);
    cross_bridge(5, id);
    printf("Car %d crosses the bridge\n",id);
    wait(5.0);
    printf("Car %d leaves the bridge\n",id);
    leave_bridge(5, id);
    pthread_exit(NULL);
}

void* truck (void* arg){
    int id=*((int*)arg);
    wait(5.0);
    cross_bridge(15, id);
    printf("Truck %d crosses the bridge\n",id);
    wait(5.0);
    printf("Truck %d leaves the bridge\n",id);
    leave_bridge(15, id);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int i;
    pthread_t id;

    for(i=0; i<NB_VEHICULES; i++) {
        state[i] = NONE;
        sem_init(&sem[i],0,0);
    }
    pthread_mutex_init(&mutex,0);

    for(i=0; i<NB_VEHICULES; i++) {
        int* j = (int*)malloc(sizeof(int));
        *j=i;
        if(i<NB_TRUCKS)
            pthread_create(&id,NULL,truck,j);
        else
            pthread_create(&id,NULL,car,j);
    }
  
    pthread_exit(NULL);
};