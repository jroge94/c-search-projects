#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

int counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Timer thread function that increments the counter every second */
void* timer_thread(void* arg) {
    (void)arg; 

    while(1) {
        sleep(1);

        
        pthread_mutex_lock(&counter_mutex);
        counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

/* Second thread function that takes and returns a double */
void* thread_func_double(void* arg) {
    
    double value = *(double*)arg; // Get the value from the argument
    printf("Second thread received value: %f\n", value);

    
    double result = value * value;

    /* Allocate memory to store the result */
    double* ret_val = malloc(sizeof(double));
    if (ret_val == NULL) {
        perror("Failed to allocate memory");
        pthread_exit(NULL);
    }
    *ret_val = result;

    /* Return the result as a void pointer */
    return (void*)ret_val;
}

int main() {
    pthread_t tid_timer, tid_double;

    /* Start the timer thread */
    pthread_create(&tid_timer, NULL, timer_thread, NULL);

    /* Prepare argument for the second thread */
    double arg_value = 3.14; 

    /* Create the second thread, passing the address of arg_value */
    pthread_create(&tid_double, NULL, thread_func_double, &arg_value);

    /* Lock the mutex before reading the counter */
    pthread_mutex_lock(&counter_mutex);
    int before_counter = counter;
    pthread_mutex_unlock(&counter_mutex);

    char input[100];
    printf("Please enter input (ANY STRING): "); // Get user input
    scanf("%99s", input);  

    /* Lock the mutex before reading the counter */
    pthread_mutex_lock(&counter_mutex);
    int after_counter = counter;
    pthread_mutex_unlock(&counter_mutex);

    /* Calculate the elapsed time */
    int elapsed_time = after_counter - before_counter;

    /* Output the results */
    printf("You entered: %s\n", input);
    printf("Time elapsed while waiting for input: %d seconds\n", elapsed_time);

    /* Wait for the second thread to finish and retrieve the result */
    double* thread_result;
    pthread_join(tid_double, (void**)&thread_result);

    printf("Second thread returned value: %f\n", *thread_result);

    /* Free the allocated memory */
    free(thread_result);

    
    pthread_cancel(tid_timer);
    pthread_join(tid_timer, NULL);

    /* Destroy the mutex */
    pthread_mutex_destroy(&counter_mutex);

    return 0;
}
