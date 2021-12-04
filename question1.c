#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define NB_TRUCKS 5
#define NB_CARS 5
#define NB_VEHICULES (NB_TRUCKS + NB_CARS)

void wait(double max);
int random_draw(double max);
void* car(void* arg);
void* truck(void* arg);

sem_t sem_count;
pthread_mutex_t truck_critical;

void wait(double max){
    struct timespec delay;
    delay.tv_sec=random_draw(max);
    delay.tv_nsec=0;
    nanosleep(&delay,NULL);
}

void cross_bridge(int tonnes){
    if (tonnes==15) {
        pthread_mutex_lock(&truck_critical);
        while(tonnes>0){
            sem_wait(&sem_count);
            tonnes=tonnes-5;
        };
        pthread_mutex_unlock(&truck_critical);
    } else sem_wait(&sem_count);
}

void leave_bridge(int tonnes){
    while(tonnes>0){
        sem_post(&sem_count);
        tonnes=tonnes-5;
    };
}

int random_draw(double max){
    int j=(int) (max*rand()/(RAND_MAX+1.0));
    if (j<1)
        j=1;
    return j;
}

void* car(void* arg){
    int pid=*((int*)arg);
    wait(5.0);
    cross_bridge(5);
    printf("Car %d crosses the bridge\n",pid);
    wait(5.0);
    printf("Car %d leaves the bridge\n",pid);
    leave_bridge(5);
    pthread_exit(NULL);
}

void* truck (void* arg){
    int pid=*((int*)arg);
    wait(5.0);
    cross_bridge(15);
    printf("Truck %d crosses the bridge\n",pid);
    wait(5.0);
    printf("Truck %d leaves the bridge\n",pid);
    leave_bridge(15);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int i;
    pthread_t id;
    sem_init(&sem_count,0,3);
    for (i=0;i<NB_VEHICULES;i++){
        int* j=(int*)malloc(sizeof(int));
        *j=i;
        if(i<NB_TRUCKS)
            pthread_create(&id,NULL,truck,j);
        else
            pthread_create(&id,NULL,car,j);
    }
    pthread_exit(NULL);
};