#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10

typedef enum { TEMPERATURE, HUMIDITY } sensor_type_t;

const char *sensor_names[] = { "Temperature", "Humidity" };

/* Circular buffer and synchronization primitives */
int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t empty_slots;               // counts free slots in buffer
sem_t filled_slots;              // counts filled slots in buffer
pthread_mutex_t buffer_mutex;    // protects buffer indices

/* Utility: simulate sensor reading */
int read_sensor_data(sensor_type_t type) {
    if (type == TEMPERATURE) {
        return rand() % 40 + 10;  // 10–49 °C
    } else {
        return rand() % 100;      // 0–99 %RH
    }
}

/* Producer thread: generate ITEMS_PER_PRODUCER readings */
void *producer(void *arg) {
    sensor_type_t type = *(sensor_type_t *)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int data = read_sensor_data(type);
        sem_wait(&empty_slots);               // wait for free slot :contentReference[oaicite:4]{index=4}
        pthread_mutex_lock(&buffer_mutex);    // enter critical section :contentReference[oaicite:5]{index=5}

        buffer[in] = data;
        printf("[Producer-%s] Put %d at %d\n",
               sensor_names[type], data, in);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buffer_mutex);  // leave critical section
        sem_post(&filled_slots);              // signal data available

        usleep((rand() % 500) * 1000);        // simulate variable sensor rate
    }
    return NULL;
}

/* Consumer thread: process data indefinitely */
void *consumer(void *arg) {
    (void)arg;
    while (1) {
        sem_wait(&filled_slots);              // wait for available data :contentReference[oaicite:6]{index=6}
        pthread_mutex_lock(&buffer_mutex);    // enter critical section :contentReference[oaicite:7]{index=7}

        int data = buffer[out];
        printf("    [Consumer] Got %d from %d\n", data, out);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buffer_mutex);  // leave critical section
        sem_post(&empty_slots);               // signal free slot

        // Simulate processing time
        usleep((rand() % 800) * 1000);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t prod_threads[NUM_PRODUCERS], cons_threads[NUM_CONSUMERS];
    sensor_type_t types[NUM_PRODUCERS] = { TEMPERATURE, HUMIDITY, TEMPERATURE };

    /* Initialize semaphores and mutex */
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&filled_slots, 0, 0);
    pthread_mutex_init(&buffer_mutex, NULL);

    /* Launch producers */
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_create(&prod_threads[i], NULL, producer, &types[i]);
    }
    /* Launch consumers */
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        pthread_create(&cons_threads[i], NULL, consumer, NULL);
    }

    /* Wait for producers to finish */
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_join(prod_threads[i], NULL);
    }
    /* (In a real system, you'd signal consumers to exit here.) */

    printf("All producers have finished. Exiting.\n");
    return 0;
}
