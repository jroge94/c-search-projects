#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4        // Number of worker threads
#define NUM_NODES 20         // Number of nodes in the graph
#define MAX_EDGES_PER_NODE 2 // Maximum edges per node



// Node structure

typedef struct Node {
    int id;
    int num_neighbors;
    int neighbors[MAX_EDGES_PER_NODE];
} Node;

Node graph[NUM_NODES];
int visited[NUM_NODES] = {0};

// Frontier queue
int frontier[NUM_NODES];
int frontier_size = 0;

// Synchronization primitives
pthread_mutex_t frontier_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t frontier_cond = PTHREAD_COND_INITIALIZER;

// Active thread counter
int active_threads = 0;

void generate_graph() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_NODES; i++) {
        graph[i].id = i;
        graph[i].num_neighbors = rand() % MAX_EDGES_PER_NODE + 1;
        for (int j = 0; j < graph[i].num_neighbors; j++) {
            int neighbor = rand() % NUM_NODES;
            // Avoid self-loops
            while (neighbor == i) {
                neighbor = rand() % NUM_NODES;
            }
            graph[i].neighbors[j] = neighbor;
        }
    }
}

void print_graph() {
    printf("Graph representation:\n");
    for (int i = 0; i < NUM_NODES; i++) {
        printf("Node %d: ", i);
        for (int j = 0; j < graph[i].num_neighbors; j++) {
            printf("%d ", graph[i].neighbors[j]);
        }
        printf("\n");
    }
}

void *bfs_thread(void *arg) {
    (void)arg; 

    while (1) {
        int current_node = -1;

        // Critical section to access the frontier
        pthread_mutex_lock(&frontier_mutex);

        // Wait until there are nodes in the frontier
        while (frontier_size == 0) {
            if (active_threads == 0) {
                // No active threads and frontier is empty, BFS is done
                pthread_mutex_unlock(&frontier_mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&frontier_cond, &frontier_mutex);
        }

        // Dequeue a node from the frontier
        current_node = frontier[--frontier_size];
        active_threads++; // Increment active thread count

        pthread_mutex_unlock(&frontier_mutex);

        // Process the current node
        printf("Thread %ld processing node %d\n", (long)pthread_self(), current_node);

        // Explore neighbors
        for (int i = 0; i < graph[current_node].num_neighbors; i++) {
            int neighbor = graph[current_node].neighbors[i];

            // Atomically check and update the visited array
            int expected = 0;
            if (__sync_bool_compare_and_swap(&visited[neighbor], expected, 1)) {
                // Enqueue the neighbor
                pthread_mutex_lock(&frontier_mutex);
                frontier[frontier_size++] = neighbor;
                pthread_cond_broadcast(&frontier_cond);
                pthread_mutex_unlock(&frontier_mutex);
            }
        }

        // After processing the node
        pthread_mutex_lock(&frontier_mutex);
        active_threads--; // Decrement active thread count
        if (frontier_size == 0 && active_threads == 0) {
            // Last thread to finish processing
            pthread_cond_broadcast(&frontier_cond);
            pthread_mutex_unlock(&frontier_mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&frontier_mutex);
    }
    return NULL;
}

int main() {
    generate_graph();
    print_graph();

    // Initialize visited array
    for (int i = 0; i < NUM_NODES; i++) {
        visited[i] = 0;
    }

    // Start BFS from node 0 if it exists
    if (NUM_NODES > 0) {
        visited[0] = 1;
        frontier[frontier_size++] = 0;
    }

    pthread_t threads[NUM_THREADS];

    // Create worker threads
    for (long i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, bfs_thread, NULL) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&frontier_mutex);
    pthread_cond_destroy(&frontier_cond);

    printf("BFS traversal completed.\n");
    return 0;
}
