#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"

void test(int fitType);
void** pointers;
int pointersIndex;

int randomNumber(int max){
    if(max==0) return 0;
    struct timeval tv;
    gettimeofday(&tv,NULL);
    srand(tv.tv_usec);
    return (rand() % max);
}

void printPointers(int index){
    //printf("pointers\n");
    for(int i=0; i<index; i++){
        printf("(%d) %p\n", i, pointers[i]);
    }
}

int main(){

    pointers = malloc(1000000*sizeof(void*));
    pointersIndex=0;
    test(0);
    pointersIndex=0;
    test(1);
    pointersIndex=0;
    test(2);

    return 0;
}

void test(int fitType){

    myinit(fitType);

    //time calculator
    struct timeval start, end;
    gettimeofday(&start, NULL);

    for(int i=0; i<50; i++){
        //just malloc a bunch
        int randomNum = randomNumber(256)+1;
        //printf("malloc ptr(%d) of size %d\n", pointersIndex, randomNum);
        void* temp = mymalloc(randomNum);
        if(temp!=NULL){
            pointers[pointersIndex]=temp;
            pointersIndex++;
        }
    }

    for(int i=0; i<1000000; i++){
        //if 0 - malloc, 1- free, 2 - realloc
        int randTask = randomNumber(3);
        if(pointersIndex<=0 || randTask==0){
            int randomNum = randomNumber(256)+1;
            //printf("[%d] malloc ptr(%d) of size %d\n", fitType, pointersIndex, randomNum);
            void* temp = mymalloc(randomNum);
            if(temp!=NULL){
                pointers[pointersIndex]=temp;
                pointersIndex++;
            }

        }else if(randTask==1 && pointersIndex>0){

            int temp = randomNumber(pointersIndex);
            //printf("[%d] free ptr(%d)\n", fitType, temp);
            myfree(pointers[temp]);
            pointers[temp] = pointers[pointersIndex-1];
            pointersIndex--;

        }else if(pointersIndex>0){
            int temp = randomNumber(pointersIndex);
            int randomNum = randomNumber(256)+1;
            //printf("[%d] realloc ptr(%d) to size %d\n", fitType, temp, randomNum);
            void* ptr = myrealloc(pointers[temp], randomNum);
            if(ptr!=NULL){
                pointers[temp] = ptr;
            }

        }
        //printPointers(pointersIndex);
    }

    gettimeofday(&end, NULL);

 
    double timeElapsed = ((end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec)); // how many useconds passed
    timeElapsed = timeElapsed/1000000;
    double opsPerSecond = 1000010/timeElapsed;
    char* c = "First fit";
    if(fitType==1) c = "Next fit";
    if(fitType==2) c = "Best fit";

    printf("%s throughput: %f ops/sec\n", c, opsPerSecond);
    printf("%s utilization: %f\n", c, utilization());

    mycleanup();

    return;
}
