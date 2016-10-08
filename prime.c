/* 
 * Operating Systems   (2INCO)   Practical Assignment
 * Threaded Application
 *
 * Michiel Favier (0951737)
 * Diederik de Wit (0829667)
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>     // for usleep()
#include <time.h>       // for time()
#include <math.h>
#include <pthread.h>

#include "prime.h"

static void rsleep (int t);
void * sieveOnce(void * j);
void printAll();
bool checkBuffer(unsigned long i);
void createNewThread(int i, int threadNumber);
//void writeAll();
FILE *fp;

pthread_mutex_t mutexInitializer = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex[(NROF_SIEVE/64) + 1];
pthread_t thread_id[NROF_THREADS];

int main (void)
{
 	//Initialize all mutexes
	int i;
	for(i = 0; i < (NROF_SIEVE/64) + 1; i++) {
		mutex[i] = mutexInitializer;
	}
	//Set all bits to 1
	for(i=0; i < (NROF_SIEVE/64) + 1; i++){
		buffer[i] = ~0ULL;
	}

	//Alternately place down chucks of NROF_THREADS amount of threads, and wait for them to finish.
	unsigned long currInt = 2;
	while(currInt < sqrt(NROF_SIEVE)){
		int threadsPlaced = 0;
		for(i = 0; i < NROF_THREADS && currInt < sqrt(NROF_SIEVE); i++){
			if(checkBuffer(currInt)){ // This rapidly becomes more efficient the more chunks have been processed, since the primes not found will become rarer
				createNewThread(currInt, i);
				threadsPlaced++;
			} else {
				i--;
			}
			currInt++;
		}
		for(i = 0; i < NROF_THREADS && i < threadsPlaced; i++){
			int joinThread = pthread_join(thread_id[i], NULL);
			if(joinThread != 0){
				perror("Waiting for the thread resulted in an error");
				exit(1);
			}
		}
	}
	printAll();
	//writeAll();
    return (0);
}

void createNewThread(int i, int threadNumber){
	int * j = (int*)malloc(sizeof(int));
	*j = i;
	int newThread = pthread_create(&thread_id[threadNumber], NULL, sieveOnce, j);
	if(newThread == -1){
		perror("Creating a thread failed");
		exit(1);
	}
	return;
}

//The Thread
void * sieveOnce(void * j){
	int i = *((int*)j);
	free(j);
	int curr;
	for(curr = i * i; curr <= NROF_SIEVE; curr += i){
		rsleep(100);
		unsigned long location = curr / 64UL;
		unsigned long index = curr % 64UL;
		unsigned long long mask = ~0ULL;
		mask &= (unsigned long long)~(1ULL << index);
		pthread_mutex_lock(&mutex[location]);
		buffer[location] &= mask;
		pthread_mutex_unlock(&mutex[location]);
	}
	return(NULL);
}

void printAll(){
	unsigned long i;
	for(i = 2; i <= NROF_SIEVE; i++){
		if(checkBuffer(i)){
			printf("%lu\n", i);
		}
	}
}
/*
void writeAll(){
	fp = fopen("output.txt", "w");
	unsigned long i;
	for(i = 2; i <= NROF_SIEVE; i++){
		if(checkBuffer(i)){
			fprintf(fp, "%lu\n", i);
		}
	}
}
*/
/*
 * checkBuffer(unsigned long i)
 *
 * Returns if i is currently considered a prime number
 */
bool checkBuffer(unsigned long i){
	unsigned long location = i / 64;
	//printf("Buffer index is %lu\n", location);
	unsigned long index = i % 64;
	//printf("Index in the long long is %lu\n", index);
	pthread_mutex_lock(&mutex[location]);
	unsigned long long data = buffer[location];
	pthread_mutex_unlock(&mutex[location]);
	//printf("The long long is %llu\n", data);
	data &= (unsigned long long)(1ULL << index);
	//printf("The mask is %llu\n",(unsigned long long)(1ULL<<index));
	//printf("Checking buffer resulted in %llu\n", data);
	if(data > 0){
		//printf("%lu is still considered a prime number\n", i);
		return true;
	} else {
		//printf("%lu is not a prime number\n", i);
		return false;
	}
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}

