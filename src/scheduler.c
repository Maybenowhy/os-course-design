#include "scheduler.h"
#include <stdio.h>
#include <string.h>

#define MAX_PROC 32
#define MAX_SEG 512

typedef struct {
    char name[16];
    int arrival;
    int burst;
    int priority;
    int remaining;
    int completion;
    int index;
} Process;

typedef struct {
    char name[16];
    int start;
    int end;
} Segment;

static void clear_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int input_processes(Process p[]) {
    int n;
    printf("请输入进程数量 (1-%d): ", MAX_PROC);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PROC) {
        clear_line();
        printf("进程数量无效。\n");
        return 0;
    }
    for (int i = 0; i < n; ++i) {
        printf("请输入第 %d 个进程：名称 到达时间 运行时间 优先级(数值越小越高): ", i + 1);
        if (scanf("%15s %d %d %d", p[i].name, &p[i].arrival, &p[i].burst, &p[i].priority) != 4 ||
            p[i].arrival < 0 || p[i].burst <= 0) {
            clear_line();
            printf("进程数据无效。\n");
            return 0;
        }
        p[i].remaining = p[i].burst;
        p[i].completion = 0;
        p[i].index = i;
    }
    return n;
}

static void copy_processes(Process dst[], const Process src[], int n) {
    for (int i = 0; i < n; ++i) dst[i] = src[i];
}

static void add_segment(Segment segs[], int *count, const char *name, int start, int end) {
    if (start == end) return;
    if (*count > 0 && strcmp(segs[*count - 1].name, name) == 0 && segs[*count - 1].end == start) {
        segs[*count - 1].end = end;
        return;
    }
    if (*count < MAX_SEG) {
        strncpy(segs[*count].name, name, sizeof(segs[*count].name) - 1);
        segs[*count].name[sizeof(segs[*count].name) - 1] = '\0';
        segs[*count].start = start;
        segs[*count].end = end;
        (*count)++;
    }
}

static void sort_by_arrival(Process p[], int n) {
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (p[j].arrival < p[i].arrival ||
                (p[j].arrival == p[i].arrival && p[j].index < p[i].index)) {
                Process t = p[i]; p[i] = p[j]; p[j] = t;
            }
        }
    }
}

static void print_result(const char *title, Process p[], int n, Segment segs[], int seg_count) {
    double total_turn = 0.0;
    double total_weighted = 0.0;
    printf("\n=== %s ===\n", title);
    printf("运行顺序: ");
    for (int i = 0; i < seg_count; ++i) {
        printf("[%d-%d:%s]", segs[i].start, segs[i].end, segs[i].name);
        if (i + 1 < seg_count) printf(" -> ");
    }
    printf("\n\n%-10s %-8s %-8s %-10s %-12s %-14s\n", "进程", "到达", "运行", "完成", "周转时间", "带权周转");
    for (int i = 0; i < n; ++i) {
        int turnaround = p[i].completion - p[i].arrival;
        double weighted = (double)turnaround / p[i].burst;
        total_turn += turnaround;
        total_weighted += weighted;
        printf("%-10s %-8d %-8d %-10d %-12d %-14.2f\n",
               p[i].name, p[i].arrival, p[i].burst, p[i].completion, turnaround, weighted);
    }
    printf("平均周转时间: %.2f\n", total_turn / n);
    printf("平均带权周转时间: %.2f\n", total_weighted / n);
}

static void fcfs(const Process src[], int n) {
    Process p[MAX_PROC];
    Segment segs[MAX_SEG];
    int seg_count = 0;
    copy_processes(p, src, n);
    sort_by_arrival(p, n);
    int time = 0;
    for (int i = 0; i < n; ++i) {
        if (time < p[i].arrival) time = p[i].arrival;
        add_segment(segs, &seg_count, p[i].name, time, time + p[i].burst);
        time += p[i].burst;
        p[i].completion = time;
    }
    print_result("FCFS", p, n, segs, seg_count);
}

static int all_done(Process p[], int n) {
    for (int i = 0; i < n; ++i) if (p[i].remaining > 0) return 0;
    return 1;
}

static void sjf(const Process src[], int n) {
    Process p[MAX_PROC];
    Segment segs[MAX_SEG];
    int seg_count = 0;
    copy_processes(p, src, n);
    int time = 0;
    while (!all_done(p, n)) {
        int best = -1;
        for (int i = 0; i < n; ++i) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 || p[i].burst < p[best].burst ||
                    (p[i].burst == p[best].burst && p[i].arrival < p[best].arrival)) best = i;
            }
        }
        if (best == -1) {
            int next = -1;
            for (int i = 0; i < n; ++i) if (p[i].remaining > 0 && (next == -1 || p[i].arrival < next)) next = p[i].arrival;
            time = next;
            continue;
        }
        add_segment(segs, &seg_count, p[best].name, time, time + p[best].burst);
        time += p[best].burst;
        p[best].remaining = 0;
        p[best].completion = time;
    }
    print_result("SJF 短作业优先(非抢占)", p, n, segs, seg_count);
}

static void priority_schedule(const Process src[], int n) {
    Process p[MAX_PROC];
    Segment segs[MAX_SEG];
    int seg_count = 0;
    copy_processes(p, src, n);
    int time = 0;
    while (!all_done(p, n)) {
        int best = -1;
        for (int i = 0; i < n; ++i) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 || p[i].priority < p[best].priority ||
                    (p[i].priority == p[best].priority && p[i].arrival < p[best].arrival)) best = i;
            }
        }
        if (best == -1) {
            int next = -1;
            for (int i = 0; i < n; ++i) if (p[i].remaining > 0 && (next == -1 || p[i].arrival < next)) next = p[i].arrival;
            time = next;
            continue;
        }
        add_segment(segs, &seg_count, p[best].name, time, time + p[best].burst);
        time += p[best].burst;
        p[best].remaining = 0;
        p[best].completion = time;
    }
    print_result("Priority 优先级调度(非抢占)", p, n, segs, seg_count);
}

static void rr(const Process src[], int n) {
    Process p[MAX_PROC];
    Segment segs[MAX_SEG];
    int seg_count = 0;
    int queue[MAX_SEG];
    int head = 0, tail = 0;
    int in_queue[MAX_PROC] = {0};
    int completed = 0;
    int quantum;
    copy_processes(p, src, n);
    printf("请输入时间片大小: ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        clear_line();
        printf("时间片无效。\n");
        return;
    }
    int time = 0;
    while (completed < n) {
        for (int i = 0; i < n; ++i) {
            if (!in_queue[i] && p[i].remaining > 0 && p[i].arrival <= time) {
                queue[tail++ % MAX_SEG] = i;
                in_queue[i] = 1;
            }
        }
        if (head == tail) {
            int next = -1;
            for (int i = 0; i < n; ++i) if (p[i].remaining > 0 && (next == -1 || p[i].arrival < next)) next = p[i].arrival;
            time = next;
            continue;
        }
        int idx = queue[head++ % MAX_SEG];
        in_queue[idx] = 0;
        int run = p[idx].remaining < quantum ? p[idx].remaining : quantum;
        add_segment(segs, &seg_count, p[idx].name, time, time + run);
        time += run;
        p[idx].remaining -= run;
        for (int i = 0; i < n; ++i) {
            if (!in_queue[i] && p[i].remaining > 0 && i != idx && p[i].arrival <= time) {
                queue[tail++ % MAX_SEG] = i;
                in_queue[i] = 1;
            }
        }
        if (p[idx].remaining > 0) {
            queue[tail++ % MAX_SEG] = idx;
            in_queue[idx] = 1;
        } else {
            p[idx].completion = time;
            completed++;
        }
    }
    print_result("RR 时间片轮转", p, n, segs, seg_count);
}

void scheduler_menu(void) {
    Process p[MAX_PROC];
    int n = input_processes(p);
    if (n == 0) return;
    while (1) {
        int choice;
        printf("\n处理机调度算法\n");
        printf("1. FCFS 先来先服务\n2. SJF 短作业优先\n3. RR 时间片轮转\n4. Priority 优先级调度\n5. 运行全部算法\n0. 返回上级菜单\n请输入选项: ");
        if (scanf("%d", &choice) != 1) { clear_line(); continue; }
        if (choice == 0) return;
        if (choice == 1) fcfs(p, n);
        else if (choice == 2) sjf(p, n);
        else if (choice == 3) rr(p, n);
        else if (choice == 4) priority_schedule(p, n);
        else if (choice == 5) { fcfs(p, n); sjf(p, n); rr(p, n); priority_schedule(p, n); }
        else printf("无效选项。\n");
    }
}
