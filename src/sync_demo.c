#include "sync_demo.h"
#include "platform_thread.h"
#include <stdio.h>
#include <stdarg.h>

#define BUFFER_SIZE 5
#define PRODUCER_COUNT 2
#define CONSUMER_COUNT 2
#define ITEMS_PER_PRODUCER 5
#define PHIL_COUNT 5
#define PERF_THREAD_COUNT 4
#define PERF_ITERATIONS 1000000ULL

typedef struct { int id; } ThreadArg;

typedef struct {
    int id;
    unsigned long long iterations;
    unsigned long long local_count;
} PerfArg;

static os_mutex_t log_mutex;
static int log_ready = 0;

static void log_init(void) {
    os_mutex_init(&log_mutex);
    log_ready = 1;
}

static void log_destroy(void) {
    log_ready = 0;
    os_mutex_destroy(&log_mutex);
}

static void log_msg(const char *fmt, ...) {
    va_list args;
    if (log_ready) os_mutex_lock(&log_mutex);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    if (log_ready) os_mutex_unlock(&log_mutex);
}

static int buffer[BUFFER_SIZE];
static int in_pos, out_pos;
static os_mutex_t pc_mutex;
static os_sem_t pc_empty, pc_full;

static void *producer(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int item = a->id * 100 + i + 1;
        os_sem_wait(&pc_empty);
        os_mutex_lock(&pc_mutex);
        buffer[in_pos] = item;
        log_msg("生产者 %d -> 数据 %d 写入缓冲区[%d]\n", a->id, item, in_pos);
        in_pos = (in_pos + 1) % BUFFER_SIZE;
        os_mutex_unlock(&pc_mutex);
        os_sem_post(&pc_full);
        os_sleep_ms(60);
    }
    return 0;
}

static void *consumer(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    int need = PRODUCER_COUNT * ITEMS_PER_PRODUCER / CONSUMER_COUNT;
    for (int i = 0; i < need; ++i) {
        os_sem_wait(&pc_full);
        os_mutex_lock(&pc_mutex);
        int item = buffer[out_pos];
        log_msg("消费者 %d <- 从缓冲区[%d] 取出数据 %d\n", a->id, out_pos, item);
        out_pos = (out_pos + 1) % BUFFER_SIZE;
        os_mutex_unlock(&pc_mutex);
        os_sem_post(&pc_empty);
        os_sleep_ms(90);
    }
    return 0;
}

static void producer_consumer_demo(void) {
    os_thread_t producers[PRODUCER_COUNT], consumers[CONSUMER_COUNT];
    ThreadArg pargs[PRODUCER_COUNT], cargs[CONSUMER_COUNT];
    in_pos = out_pos = 0;
    log_init();
    os_mutex_init(&pc_mutex);
    os_sem_init(&pc_empty, BUFFER_SIZE);
    os_sem_init(&pc_full, 0);
    printf("\n生产者-消费者演示开始。\n");
    for (int i = 0; i < PRODUCER_COUNT; ++i) { pargs[i].id = i + 1; os_thread_create(&producers[i], producer, &pargs[i]); }
    for (int i = 0; i < CONSUMER_COUNT; ++i) { cargs[i].id = i + 1; os_thread_create(&consumers[i], consumer, &cargs[i]); }
    for (int i = 0; i < PRODUCER_COUNT; ++i) os_thread_join(&producers[i]);
    for (int i = 0; i < CONSUMER_COUNT; ++i) os_thread_join(&consumers[i]);
    os_sem_destroy(&pc_empty);
    os_sem_destroy(&pc_full);
    os_mutex_destroy(&pc_mutex);
    printf("生产者-消费者演示结束。\n");
    log_destroy();
}

static int read_count = 0;
static int shared_data = 0;
static os_mutex_t rw_count_mutex;
static os_sem_t rw_resource;

static void *reader(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    for (int i = 0; i < 3; ++i) {
        os_mutex_lock(&rw_count_mutex);
        read_count++;
        if (read_count == 1) os_sem_wait(&rw_resource);
        os_mutex_unlock(&rw_count_mutex);
        log_msg("读者 %d 读取 shared_data=%d\n", a->id, shared_data);
        os_sleep_ms(70);
        os_mutex_lock(&rw_count_mutex);
        read_count--;
        if (read_count == 0) os_sem_post(&rw_resource);
        os_mutex_unlock(&rw_count_mutex);
        os_sleep_ms(80);
    }
    return 0;
}

static void *writer(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    for (int i = 0; i < 3; ++i) {
        os_sem_wait(&rw_resource);
        shared_data += 10;
        log_msg("写者 %d 写入 shared_data=%d\n", a->id, shared_data);
        os_sleep_ms(100);
        os_sem_post(&rw_resource);
        os_sleep_ms(120);
    }
    return 0;
}

static void reader_writer_demo(void) {
    os_thread_t threads[4];
    ThreadArg args[4];
    read_count = 0;
    shared_data = 0;
    log_init();
    os_mutex_init(&rw_count_mutex);
    os_sem_init(&rw_resource, 1);
    printf("\n读者-写者演示开始。\n");
    args[0].id = 1; os_thread_create(&threads[0], reader, &args[0]);
    args[1].id = 2; os_thread_create(&threads[1], reader, &args[1]);
    args[2].id = 1; os_thread_create(&threads[2], writer, &args[2]);
    args[3].id = 3; os_thread_create(&threads[3], reader, &args[3]);
    for (int i = 0; i < 4; ++i) os_thread_join(&threads[i]);
    os_sem_destroy(&rw_resource);
    os_mutex_destroy(&rw_count_mutex);
    printf("读者-写者演示结束。\n");
    log_destroy();
}

static os_mutex_t forks[PHIL_COUNT];

static void *philosopher(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    int id = a->id;
    int left = id;
    int right = (id + 1) % PHIL_COUNT;
    int first = left < right ? left : right;
    int second = left < right ? right : left;
    for (int i = 0; i < 2; ++i) {
        log_msg("哲学家 %d 正在思考。\n", id);
        os_sleep_ms(70);
        os_mutex_lock(&forks[first]);
        os_mutex_lock(&forks[second]);
        log_msg("哲学家 %d 正在使用筷子 %d 和 %d 进餐。\n", id, left, right);
        os_sleep_ms(100);
        os_mutex_unlock(&forks[second]);
        os_mutex_unlock(&forks[first]);
    }
    return 0;
}

static void dining_philosophers_demo(void) {
    os_thread_t threads[PHIL_COUNT];
    ThreadArg args[PHIL_COUNT];
    log_init();
    printf("\n哲学家进餐演示开始。\n");
    for (int i = 0; i < PHIL_COUNT; ++i) os_mutex_init(&forks[i]);
    for (int i = 0; i < PHIL_COUNT; ++i) { args[i].id = i; os_thread_create(&threads[i], philosopher, &args[i]); }
    for (int i = 0; i < PHIL_COUNT; ++i) os_thread_join(&threads[i]);
    for (int i = 0; i < PHIL_COUNT; ++i) os_mutex_destroy(&forks[i]);
    printf("哲学家进餐演示结束。\n");
    log_destroy();
}

static unsigned long long perf_shared_count = 0;
static os_mutex_t perf_mutex;

static void *global_lock_counter(void *arg) {
    PerfArg *a = (PerfArg *)arg;
    for (unsigned long long i = 0; i < a->iterations; ++i) {
        os_mutex_lock(&perf_mutex);
        perf_shared_count++;
        os_mutex_unlock(&perf_mutex);
    }
    return 0;
}

static void *local_counter(void *arg) {
    PerfArg *a = (PerfArg *)arg;
    volatile unsigned long long local = 0;
    for (unsigned long long i = 0; i < a->iterations; ++i) local++;
    a->local_count = local;
    return 0;
}

static void print_perf_result(const char *name, unsigned long long count,
                              unsigned long long expected, unsigned long long elapsed) {
    printf("%-22s 计数=%-12llu 期望=%-12llu 耗时=%llums %s\n",
           name, count, expected, elapsed, count == expected ? "正确" : "异常");
}

static void concurrency_performance_demo(void) {
    os_thread_t threads[PERF_THREAD_COUNT];
    PerfArg args[PERF_THREAD_COUNT];
    unsigned long long expected = PERF_THREAD_COUNT * PERF_ITERATIONS;
    unsigned long long start, elapsed;

    printf("\n并发性能优化实验开始。\n");
    printf("实验参数：线程数=%d，每线程循环次数=%llu，总计数=%llu。\n",
           PERF_THREAD_COUNT, PERF_ITERATIONS, expected);

    volatile unsigned long long single_count = 0;
    start = os_time_ms();
    for (unsigned long long i = 0; i < expected; ++i) single_count++;
    elapsed = os_time_ms() - start;
    print_perf_result("单线程基线", single_count, expected, elapsed);

    perf_shared_count = 0;
    os_mutex_init(&perf_mutex);
    start = os_time_ms();
    for (int i = 0; i < PERF_THREAD_COUNT; ++i) {
        args[i].id = i + 1;
        args[i].iterations = PERF_ITERATIONS;
        args[i].local_count = 0;
        os_thread_create(&threads[i], global_lock_counter, &args[i]);
    }
    for (int i = 0; i < PERF_THREAD_COUNT; ++i) os_thread_join(&threads[i]);
    elapsed = os_time_ms() - start;
    os_mutex_destroy(&perf_mutex);
    print_perf_result("多线程全局锁计数", perf_shared_count, expected, elapsed);

    start = os_time_ms();
    for (int i = 0; i < PERF_THREAD_COUNT; ++i) {
        args[i].id = i + 1;
        args[i].iterations = PERF_ITERATIONS;
        args[i].local_count = 0;
        os_thread_create(&threads[i], local_counter, &args[i]);
    }
    unsigned long long local_total = 0;
    for (int i = 0; i < PERF_THREAD_COUNT; ++i) {
        os_thread_join(&threads[i]);
        local_total += args[i].local_count;
    }
    elapsed = os_time_ms() - start;
    print_perf_result("多线程局部汇总", local_total, expected, elapsed);

    printf("分析：全局锁计数每次循环都进入临界区，锁竞争开销明显；局部计数后汇总减少共享写入，通常耗时更低。\n");
    printf("并发性能优化实验结束。\n");
}

static void clear_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void sync_menu(void) {
    while (1) {
        int choice;
        printf("\n进程同步与并发控制\n1. 生产者-消费者\n2. 读者-写者\n3. 哲学家进餐\n4. 运行经典同步演示\n5. 并发性能优化实验\n0. 返回上级菜单\n请输入选项: ");
        if (scanf("%d", &choice) != 1) { clear_line(); continue; }
        if (choice == 0) return;
        if (choice == 1) producer_consumer_demo();
        else if (choice == 2) reader_writer_demo();
        else if (choice == 3) dining_philosophers_demo();
        else if (choice == 4) { producer_consumer_demo(); reader_writer_demo(); dining_philosophers_demo(); }
        else if (choice == 5) concurrency_performance_demo();
        else printf("无效选项。\n");
    }
}
