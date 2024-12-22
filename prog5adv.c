#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 8  

/* this type is used to pass data to the thread */
typedef struct {
    int *arr;
    int left;
    int right;
} thread_data_t;

pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int thread_count = 0;

// Function to swap two elements
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to partition the array
int partition(int *arr, int left, int right) {
    int pivot = arr[right];  // Pivot element
    int i = left - 1;        // Index of smaller element

    for (int j = left; j <= right - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[right]);
    return i + 1;
}


// Function to perform quicksort
void *quicksort_thread(void *arg);


// Function to perform quicksort
void quicksort(int *arr, int left, int right) {
    if (left < right) {
        int pi = partition(arr, left, right);

        pthread_t tid_left;
        int create_thread = 0;

        // Decide whether to create a new thread
        pthread_mutex_lock(&thread_count_mutex);
        if (thread_count < MAX_THREADS) {
            thread_count++;
            create_thread = 1;
        }
        pthread_mutex_unlock(&thread_count_mutex);

        if (create_thread) {
            // Create a new thread for the left subarray
            thread_data_t *data = malloc(sizeof(thread_data_t));
            data->arr = arr;
            data->left = left;
            data->right = pi - 1;

            if (pthread_create(&tid_left, NULL, quicksort_thread, data) != 0) {
                perror("Failed to create thread");
                exit(EXIT_FAILURE);
            }

            // Sort the right subarray in the current thread
            quicksort(arr, pi + 1, right);

            // Wait for the left subarray thread to finish
            pthread_join(tid_left, NULL);

            pthread_mutex_lock(&thread_count_mutex);
            thread_count--;
            pthread_mutex_unlock(&thread_count_mutex);
        } else {
           
            quicksort(arr, left, pi - 1);
            quicksort(arr, pi + 1, right);
        }
    }
}

void *quicksort_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    quicksort(data->arr, data->left, data->right);
    free(data);
    return NULL;
}

int main() {
    int size = 20;
    int arr[size];

    // Generate random array
    printf("Unsorted array:\n");
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 100;  // Random numbers between 0 and 99
        printf("%d ", arr[i]);
    }
    printf("\n");

    quicksort(arr, 0, size - 1);

    // Print sorted array
    printf("Sorted array:\n");
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    pthread_mutex_destroy(&thread_count_mutex);

    return 0;
}
