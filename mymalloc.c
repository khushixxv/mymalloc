#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

void myinit(int allocAlg);
void* mymalloc(size_t size);
void myfree(void* ptr);
void* myrealloc(void* ptr, size_t size);
void mycleanup();
double utilization();


int fit; // 0 -> first fit, 1 -> next fit, 2 -> best fit
long int* heap;
long int mallocCounter;
long int lastIndex;

void myinit(int allocAlg){
    //allocAlg is: 0 - first fit, 1 - next fit, 2 - best fit

    heap = calloc(131072, 8);
    fit = allocAlg;
    mallocCounter=0;
    lastIndex=0;
    heap[0] = 1048576 - 24; //bc the first block is free and its the size of the whole minus the header & footer.
    heap[1] = -1; //index of next FREE block
    heap[131071] = 1048576 - 24; //footer
}

void* mymalloc(size_t size){


    // FIRST FIT 

    if(fit==0){
        long int index = 0;
        int count=0;
        long int len = (size + 7) & (-8);

        if(len>(1048576-24)) {
            //printf("there is no block that fits\n"); //because it is too big
            return 0;
        }

        while( (index < 131071) && ( (heap[index] & 1) || ( heap[index] < len))){
            //printf("index (%ld) fails. finding new free block\n", index);
            index = heap[index+1]; // this takes it to the next free block
            count++;
            if(index>131071 || index == -1 || index <= 0 || count>500 ){
                //printf("there is no block that fits\n");
                return NULL;
            }
        }

        // SPLITTING
        // if necessary (if original size of block minus len is not super small)
        long int blockFooterIndex = index + (heap[index]/8) + 2;
        
        if((heap[index] - (len))>=32) {
            //printf("splitting index(%ld) of size %ld into size %ld\n", index, heap[index], len);

            long int nextFreeBlockIndex = index+((len+24)/8);
            //printf("next Free block index is: %ld\n", nextFreeBlockIndex);
            heap[nextFreeBlockIndex] = heap[index] - len - 24;
            heap[index]=len;
            heap[nextFreeBlockIndex+1] = heap[index+1]; // sets the next free block
            heap[index+1] = nextFreeBlockIndex; // links original to new one
            
            //set the footer
            blockFooterIndex = index + (len/8) + 2;

        }

        //mark as allocated
        heap[blockFooterIndex] = heap[index];
        heap[index]++;
        //printf("block index(%ld) header: %ld\npointer: %ld\nfooter: %ld\n", index, heap[index], heap[index+1], heap[blockFooterIndex]);


        //return pointer to block
        //printf("FINAL RETURN: %p\n", &heap[index+2]);

        mallocCounter+=size;
        return &heap[index+2];

    }

    // NEXT FIT

    if(fit==1){
        if(lastIndex==-1) return NULL;
        long int index = lastIndex;
        int count = 0;
        long int len = (size + 7) & (-8);

        if(len>(1048576-24)) {
            //printf("there is no block that fits\n"); //because it is too big
            return 0;
        }

        while( (index < 131071) && ( (heap[index] & 1) || ( heap[index] <= len))){
            //printf("index (%ld) fails. finding new free block\n", index);
            index = heap[index+1]; // this takes it to the next free block
            count++;
            if(index==-1) index =0; //basically this wraps it around
            if(index>131072 || index == lastIndex || count>500){
                //printf("there is no block that fits\n");
                return NULL;
            }
        }

        //make sure ur index is ok (it fits and is free)
        if(heap[index]<len || (heap[index] & 1) ) return NULL;

        long int blockFooterIndex = index + (heap[index]/8) + 2;
        if(blockFooterIndex>131071 || blockFooterIndex<0) return NULL;

        lastIndex = index;

        // SPLITTING
        // if necessary (if original size of block minus len is not super small)
        
        if((heap[index] - (len))>=32) {
            //printf("splitting index(%ld) of size %ld into size %ld\n", index, heap[index], len);

            long int nextFreeBlockIndex = index+((len+24)/8);
            //printf("next Free block index is: %ld\n", nextFreeBlockIndex);
            if(nextFreeBlockIndex>131068) return NULL;
            heap[nextFreeBlockIndex] = heap[index] - len - 24;
            heap[index]=len;
            heap[nextFreeBlockIndex+1] = heap[index+1]; // sets the next free block
            heap[index+1] = nextFreeBlockIndex; // links original to new one
            
            //set the footer
            blockFooterIndex = nextFreeBlockIndex-1;

        }

        //mark as allocated
        heap[blockFooterIndex] = heap[index];
        heap[index]++;
        //printf("block index(%ld) header: %ld\npointer: %ld\nfooter: %ld\n", index, heap[index], heap[index+1], heap[blockFooterIndex]);


        //return pointer to block
        //printf("FINAL RETURN: %p\n", &heap[index+2]);
        mallocCounter+=size;
        return &heap[index+2];

    }

    
    // BEST FIT

    if(fit==2){
        long int index = 0;
        int count =0;
        long int len = (size + 7) & (-8);
        long int bestIndex = -1;
        long int bestLen = 1048576 ;

        if(len>(1048576-24)) {
            //printf("there is no block that fits\n"); //because it is too big
            return 0;
        }


        while(index < 131071 && index!=-1){

            if(((heap[index]&1) == 0) && (heap[index]>=len)){
                if(heap[index]<bestLen){
                    bestLen = heap[index];
                    bestIndex = index;
                }
            }
            //printf("index (%ld) fails. finding new free block\n", index);
            index = heap[index+1]; // this takes it to the next free block
            count++;
            if(index == 0 || count>500)break;
        }

        if(bestIndex==-1 || count>500){
            //printf("there is no block that fits\n");
            return NULL;
        }

        //make sure ur index is ok (it fits and is free)
        if(index>131068 || index<0 || heap[index]<len || (heap[index] & 1) ) return NULL;


        long int blockFooterIndex = index + (heap[index]/8) + 2;
        if(blockFooterIndex>131071 || blockFooterIndex<0) return NULL;

        index = bestIndex;

        // SPLITTING
        // if necessary (if original size of block minus len is not super small)
        
        if((heap[index] - (len))>=32) {
            //printf("splitting index(%ld) of size %ld into size %ld\n", index, heap[index], len);

            long int nextFreeBlockIndex = index+((len+24)/8);
            //printf("next Free block index is: %ld\n", nextFreeBlockIndex);
            heap[nextFreeBlockIndex] = heap[index] - len - 24;
            heap[index]=len;
            heap[nextFreeBlockIndex+1] = heap[index+1]; // sets the next free block
            heap[index+1] = nextFreeBlockIndex; // links original to new one
            
            //set the footer
            blockFooterIndex = index + (len/8) + 2;

        }

        //mark as allocated
        heap[blockFooterIndex] = heap[index];
        heap[index]++;
        //printf("block index(%ld) header: %ld\npointer: %ld\nfooter: %ld\n", index, heap[index], heap[index+1], heap[blockFooterIndex]);


        //return pointer to block
        //printf("FINAL RETURN: %p\n", &heap[index+2]);

        mallocCounter+=size;
        return &heap[index+2];

    }

    

    return NULL;
}

void myfree(void* ptr){

    long int freeIndex = ((long int *)(ptr)) - heap;
    freeIndex-=2;
    //printf("free index value is : %ld\n", freeIndex);


    if(freeIndex<0 || freeIndex >= 131072){
        //pointer not in range
        printf("Error: invalid pointer\n");
        return;
    }

    if((heap[freeIndex]&1) == 0){
        //this means it is already free - double free error!
        //printf("Error: invalid pointer!\n");
		return;
    }

    heap[freeIndex]--; //clear the allocated flag bc its free now
    mallocCounter = mallocCounter - heap[freeIndex]; //and update mallocCounter

    //combining FREE blocks to make jumbo big free block
    // Coalescing !! (if needed)
    long int freeBlockFooterIndex = freeIndex + (heap[freeIndex]/8) + 2;
    long int nextBlockIndex = heap[freeIndex+1];
    //printf("next block index is %ld\n", nextBlockIndex);
    if((nextBlockIndex!=-1 && nextBlockIndex<131072) && ((heap[nextBlockIndex]&1) == 0)){

        freeBlockFooterIndex = nextBlockIndex + (heap[nextBlockIndex]/8) + 2;
        if(freeBlockFooterIndex<131072){
            //printf("coalescing with next block\n");
            heap[freeIndex] = heap[freeIndex] + heap[nextBlockIndex] + 24;
            heap[freeBlockFooterIndex] = heap[freeIndex];
            heap[freeIndex+1] = heap[nextBlockIndex+1];
        }
    }

    if(freeIndex>0){
        //printf("the prev block is of size: %ld\n", heap[freeIndex-1]);
        long int prevBlockIndex = freeIndex - ((heap[freeIndex-1])/8)-3;
        freeBlockFooterIndex = freeIndex + (heap[freeIndex]/8) + 2;
        //printf("the prev block index is: %ld\n", prevBlockIndex);
        if((freeBlockFooterIndex<=131071)&&(prevBlockIndex>=0) && (heap[prevBlockIndex]&1) == 0){
            //printf("coalescing with prev block\n");
            //meaning there is a free block behind it
            heap[prevBlockIndex] = heap[freeIndex] + heap[prevBlockIndex] + 24;
            //basically adding the entirety of middle block WITH its padding
            freeBlockFooterIndex = freeIndex + (heap[freeIndex]/8) + 2;
            heap[freeBlockFooterIndex] = heap[prevBlockIndex];
            heap[prevBlockIndex+1] = heap[freeIndex+1];
        }
        //printf("prev block index value is: %ld\n", heap[prevBlockIndex]);

    }
    //printf("you just freed index: %ld\n", freeIndex);

}

void* myrealloc(void* ptr, size_t size){

    void* newPtr = mymalloc(size);

    if(newPtr==NULL) {
        //printf("no block can fit request\n");
        return NULL;
    }

    myfree(ptr);
    return newPtr;
}

void mycleanup(){
    free(heap);
}

double utilization(){
    double denominator =0;
    long int index =0;
    int count=0;
    if((heap[index]&1) == 1) denominator= denominator + (heap[index]-1);
    index = heap[index+1];

    while(index<131072 && index != -1){
        if((heap[index]&1) == 1) denominator= denominator + (heap[index]-1);
        index = heap[index+1];
        count++;
        if(index == 0 ||count>500) break;
    }

    if(denominator<=0) {
        //printf("no memory is in use\n");
        return 0;
    }
    //printf("mallocCounter is: %ld, denominator is %f\n", mallocCounter, denominator);
    double utilization = -mallocCounter/denominator;
    utilization = ((int) utilization % 100)/100.0;
    return utilization;
}

