#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *do_work(void *arg) {
    int i = *((int *)arg);
    printf("helloworld from do work,%d\n", i);
    return NULL;
}

void *func(void *arg) {
    int *i = (int *)arg;
    printf("helloworld from func\n");
    printf("a=%d\n", *i);
    (*i)++;
    return NULL;
}

int main() {
    pthread_t thread1;
    int arg1 = 42;
    if (pthread_create(&thread1, NULL, do_work, &arg1) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // Detach the thread to allow it to run independently
    pthread_detach(thread1);

    int locat_state = 0;
    pthread_t thread2;
    if (pthread_create(&thread2, NULL, func, &locat_state) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    // Wait for thread2 to finish
    pthread_join(thread2, NULL);
    printf("locat_state=%d\n", locat_state);


    // Sleep for a short time to ensure the detached thread has time to execute
    sleep(1);

    return 0;
}
