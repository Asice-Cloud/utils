#define _GNU_SOURCE
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define TOTAL_ITEMS 20

// 共享缓冲区结构
typedef struct {
    int *buffer; // 缓冲区数组
    int size;    // 缓冲区大小
    int in;      // 生产者插入位置
    int out;     // 消费者取出位置
    int count;   // 当前缓冲区中的项目数

    pthread_mutex_t mutex; // 互斥锁
    sem_t empty;           // 空槽位信号量
    sem_t full;            // 满槽位信号量
} shared_buffer_t;

// 生产者参数结构
typedef struct {
    int id;
    shared_buffer_t *buffer;
    int items_to_produce;
} producer_args_t;

// 消费者参数结构
typedef struct {
    int id;
    shared_buffer_t *buffer;
    int items_to_consume;
} consumer_args_t;

// 全局统计
int total_produced = 0;
int total_consumed = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// 初始化共享缓冲区
void buffer_init(shared_buffer_t *buf, int size) {
    buf->buffer = (int *)malloc(size * sizeof(int));
    buf->size = size;
    buf->in = 0;
    buf->out = 0;
    buf->count = 0;

    pthread_mutex_init(&buf->mutex, NULL);
    sem_init(&buf->empty, 0, size); // 初始时所有槽位都空
    sem_init(&buf->full, 0, 0);     // 初始时没有满槽位
}

// 销毁共享缓冲区
void buffer_destroy(shared_buffer_t *buf) {
    free(buf->buffer);
    pthread_mutex_destroy(&buf->mutex);
    sem_destroy(&buf->empty);
    sem_destroy(&buf->full);
}

// 生产者函数
void *producer(void *arg) {
    producer_args_t *args = (producer_args_t *)arg;
    shared_buffer_t *buf = args->buffer;
    int producer_id = args->id;

    for (int i = 0; i < args->items_to_produce; i++) {
        // 生产一个项目（这里用随机数模拟）
        int item = rand() % 1000 + producer_id * 1000;

        // 模拟生产时间
        usleep(rand() % 100000 + 50000); // 50-150ms

        // 等待空槽位
        sem_wait(&buf->empty);

        // 进入临界区
        pthread_mutex_lock(&buf->mutex);

        // 将项目放入缓冲区
        buf->buffer[buf->in] = item;
        printf("生产者 %d 生产了项目 %d，放入缓冲区位置 %d\n", producer_id,
               item, buf->in);

        buf->in = (buf->in + 1) % buf->size;
        buf->count++;

        // 更新统计
        pthread_mutex_lock(&stats_mutex);
        total_produced++;
        printf("  -> 缓冲区状态: %d/%d, 总生产: %d\n", buf->count, buf->size,
               total_produced);
        pthread_mutex_unlock(&stats_mutex);

        // 离开临界区
        pthread_mutex_unlock(&buf->mutex);

        // 通知有新的满槽位
        sem_post(&buf->full);
    }

    printf("生产者 %d 完成了所有生产任务\n", producer_id);
    return NULL;
}

// 消费者函数
void *consumer(void *arg) {
    consumer_args_t *args = (consumer_args_t *)arg;
    shared_buffer_t *buf = args->buffer;
    int consumer_id = args->id;

    for (int i = 0; i < args->items_to_consume; i++) {
        // 等待满槽位
        sem_wait(&buf->full);

        // 进入临界区
        pthread_mutex_lock(&buf->mutex);

        // 从缓冲区取出项目
        int item = buf->buffer[buf->out];
        printf("消费者 %d 消费了项目 %d，从缓冲区位置 %d 取出\n", consumer_id,
               item, buf->out);

        buf->out = (buf->out + 1) % buf->size;
        buf->count--;

        // 更新统计
        pthread_mutex_lock(&stats_mutex);
        total_consumed++;
        printf("  -> 缓冲区状态: %d/%d, 总消费: %d\n", buf->count, buf->size,
               total_consumed);
        pthread_mutex_unlock(&stats_mutex);

        // 离开临界区
        pthread_mutex_unlock(&buf->mutex);

        // 通知有新的空槽位
        sem_post(&buf->empty);

        // 模拟消费时间
        usleep(rand() % 80000 + 70000); // 70-150ms
    }

    printf("消费者 %d 完成了所有消费任务\n", consumer_id);
    return NULL;
}

// 打印缓冲区状态
void print_buffer_status(shared_buffer_t *buf) {
    pthread_mutex_lock(&buf->mutex);
    printf("\n当前缓冲区状态: [");
    for (int i = 0; i < buf->size; i++) {
        if (i >= buf->out && i < buf->out + buf->count) {
            printf("%d", buf->buffer[i]);
        } else {
            printf("_");
        }
        if (i < buf->size - 1)
            printf(", ");
    }
    printf("] (count=%d)\n", buf->count);
    pthread_mutex_unlock(&buf->mutex);
}

int main() {
    printf("生产者-消费者模型演示\n");
    printf("缓冲区大小: %d\n", BUFFER_SIZE);
    printf("生产者数量: %d\n", NUM_PRODUCERS);
    printf("消费者数量: %d\n", NUM_CONSUMERS);
    printf("总项目数: %d\n\n", TOTAL_ITEMS);

    // 初始化随机数种子
    srand(time(NULL));

    // 创建共享缓冲区
    shared_buffer_t buffer;
    buffer_init(&buffer, BUFFER_SIZE);

    // 创建线程数组
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    // 创建生产者参数
    producer_args_t producer_args[NUM_PRODUCERS];
    int items_per_producer = TOTAL_ITEMS / NUM_PRODUCERS;
    int remaining_items = TOTAL_ITEMS % NUM_PRODUCERS;

    // 创建消费者参数
    consumer_args_t consumer_args[NUM_CONSUMERS];
    int items_per_consumer = TOTAL_ITEMS / NUM_CONSUMERS;
    int remaining_consumer_items = TOTAL_ITEMS % NUM_CONSUMERS;

    // 启动生产者线程
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_args[i].id = i + 1;
        producer_args[i].buffer = &buffer;
        producer_args[i].items_to_produce =
            items_per_producer + (i < remaining_items ? 1 : 0);

        pthread_create(&producers[i], NULL, producer, &producer_args[i]);
        printf("启动生产者 %d (将生产 %d 个项目)\n", i + 1,
               producer_args[i].items_to_produce);
    }

    // 启动消费者线程
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_args[i].id = i + 1;
        consumer_args[i].buffer = &buffer;
        consumer_args[i].items_to_consume =
            items_per_consumer + (i < remaining_consumer_items ? 1 : 0);

        pthread_create(&consumers[i], NULL, consumer, &consumer_args[i]);
        printf("启动消费者 %d (将消费 %d 个项目)\n", i + 1,
               consumer_args[i].items_to_consume);
    }

    printf("\n开始生产和消费...\n\n");

    // 等待所有生产者线程结束
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    printf("\n所有生产者已完成工作\n");

    // 等待所有消费者线程结束
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    printf("所有消费者已完成工作\n");

    // 打印最终统计
    printf("\n=== 最终统计 ===\n");
    printf("总生产项目数: %d\n", total_produced);
    printf("总消费项目数: %d\n", total_consumed);
    printf("缓冲区剩余项目: %d\n", buffer.count);

    // 清理资源
    buffer_destroy(&buffer);
    pthread_mutex_destroy(&stats_mutex);

    printf("\n程序结束\n");
    return 0;
}
