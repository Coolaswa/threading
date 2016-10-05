/* 
 * Operating Systems   (2INCO)   Practical Assignment
 * Threaded Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * ”Extra” steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
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

pthread_mutex_t mutexInitializer = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex[(NROF_SIEVE/64) + 1];

int main (void)
{
    // TODO: start threads generate all primes between 2 and NROF_SIEVE and output the results
    // (see thread_malloc_free_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)

	int i;
	for(i = 0; i < (NROF_SIEVE/64) + 1; i++) {
		mutex[i] = mutexInitializer;
	}
	//Set all bits to 1
	for(i=0; i < (NROF_SIEVE/64) + 1; i++){
		buffer[i] = ~0ULL;
		//printf("%llu\n", buffer[i]);
	}
	//Check for non primes
	pthread_t thread_id[NROF_THREADS];
	//for(i = 2; i < sqrt(NROF_SIEVE); i++){
	for(i = 2; i < NROF_THREADS && i < sqrt(NROF_SIEVE); i++){
		if(checkBuffer(i)){
			//printf("Trying to create a new thread\n");
			int * j = (int*)malloc(sizeof(int));
			*j = i;
			int newThread = pthread_create(&thread_id[*j - 2], NULL, sieveOnce, j);
			if(newThread == -1){
				perror("Creating a thread failed");
				exit(1);
			}
			//sieveOnce(i);
			//printf("Removed all multiples of %d\n", i);
		}
	}/*
	while(i < sqrt(NROF_SIEVE)){
		int joinThread = pthread_join(thread_id[], NULL);
		if(joinThread != 0){
			perror("Waiting for the thread resulted in an error");
			exit(1);
		}
		printf("Removed all multiples of %d\n", i);
		int newThread = pthread_create(&thread_id[], NULL, sieveOnce, (void*)&i);
		if(newThread != 0){
			perror("Creating a thread failed");
			exit(1);
		}
		i++;
	}*/
	sleep(1); //This is a temporary solution to let all threads finish
	printAll();
    return (0);
}



void * sieveOnce(void * j){
	int i = *((int*)j);
	//printf("i is: %d\n",i);
	free(j);
	int curr;
	for(curr = i * i; curr < NROF_SIEVE; curr += i){
		unsigned long location = curr / 64UL;
		unsigned long index = curr % 64UL;
		//printf("Sieving index is %lu\n", index);
		unsigned long long mask = ~0ULL;
		mask &= (unsigned long long)~(1ULL << index);
		//printf("The index was set to %llu\n", (unsigned long long)(1 << index));
		//printf("The mask was set to %llu\n", mask);
		pthread_mutex_lock(&mutex[location]);
		buffer[location] &= mask;
		pthread_mutex_unlock(&mutex[location]);
		//printf("Just removed %d from the list\n", curr);
	}
	return(NULL);
}

void printAll(){
	unsigned long i;
	for(i = 2; i < NROF_SIEVE; i++){
		if(checkBuffer(i)){
			printf("%lu\n", i);
		}
	}
}

/*
 * checkBuffer(int i)
 *
 * Returns if i is currently considered a prime number
 */
bool checkBuffer(unsigned long i){
	unsigned long location = i / 64;
	//printf("Buffer index is %lu\n", location);
	unsigned long index = i % 64;
	//printf("Index in the long long is %lu\n", index);
	unsigned long long data = buffer[location];
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

