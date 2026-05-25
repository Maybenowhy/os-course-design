#include "memory.h"
#include <stdio.h>
#include <string.h>

#define MAX_BLOCKS 64
#define MAX_FRAMES 16
#define MAX_REFS 128

typedef struct {
    int start;
    int size;
    int free;
    char owner[16];
} MemBlock;

static MemBlock blocks[MAX_BLOCKS];
static int block_count = 0;

static void clear_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void init_memory(int total) {
    block_count = 1;
    blocks[0].start = 0;
    blocks[0].size = total;
    blocks[0].free = 1;
    strcpy(blocks[0].owner, "空闲");
}

static void show_blocks(void) {
    printf("\n%-8s %-8s %-8s %-12s\n", "起址", "大小", "状态", "所属作业");
    for (int i = 0; i < block_count; ++i) {
        printf("%-8d %-8d %-8s %-12s\n", blocks[i].start, blocks[i].size, blocks[i].free ? "空闲" : "已用", blocks[i].owner);
    }
}

static void merge_free_blocks(void) {
    for (int i = 0; i < block_count - 1;) {
        if (blocks[i].free && blocks[i + 1].free) {
            blocks[i].size += blocks[i + 1].size;
            for (int j = i + 1; j < block_count - 1; ++j) blocks[j] = blocks[j + 1];
            block_count--;
        } else {
            i++;
        }
    }
}

static void allocate_block(int best_fit) {
    char name[16];
    int size;
    int chosen = -1;
    printf("请输入作业名和大小: ");
    if (scanf("%15s %d", name, &size) != 2 || size <= 0) { clear_line(); printf("输入无效。\n"); return; }
    for (int i = 0; i < block_count; ++i) {
        if (blocks[i].free && blocks[i].size >= size) {
            if (!best_fit) { chosen = i; break; }
            if (chosen == -1 || blocks[i].size < blocks[chosen].size) chosen = i;
        }
    }
    if (chosen == -1) { printf("分配失败：没有足够大的空闲分区。\n"); return; }
    if (blocks[chosen].size > size && block_count < MAX_BLOCKS) {
        for (int j = block_count; j > chosen + 1; --j) blocks[j] = blocks[j - 1];
        blocks[chosen + 1].start = blocks[chosen].start + size;
        blocks[chosen + 1].size = blocks[chosen].size - size;
        blocks[chosen + 1].free = 1;
        strcpy(blocks[chosen + 1].owner, "空闲");
        block_count++;
    }
    blocks[chosen].size = size;
    blocks[chosen].free = 0;
    strncpy(blocks[chosen].owner, name, sizeof(blocks[chosen].owner) - 1);
    blocks[chosen].owner[sizeof(blocks[chosen].owner) - 1] = '\0';
    printf("已为作业 %s 分配内存，起始地址为 %d。\n", name, blocks[chosen].start);
    show_blocks();
}

static void free_block(void) {
    char name[16];
    printf("请输入要回收的作业名: ");
    if (scanf("%15s", name) != 1) { clear_line(); return; }
    for (int i = 0; i < block_count; ++i) {
        if (!blocks[i].free && strcmp(blocks[i].owner, name) == 0) {
            blocks[i].free = 1;
            strcpy(blocks[i].owner, "空闲");
            merge_free_blocks();
            printf("已回收作业 %s，并合并相邻空闲分区。\n", name);
            show_blocks();
            return;
        }
    }
    printf("未找到该作业。\n");
}

static void partition_demo(void) {
    int total;
    printf("请输入内存总大小: ");
    if (scanf("%d", &total) != 1 || total <= 0) { clear_line(); printf("内存大小无效。\n"); return; }
    init_memory(total);
    while (1) {
        int choice;
        printf("\n动态分区管理\n1. 首次适应分配(FF)\n2. 最佳适应分配(BF)\n3. 回收分区\n4. 显示分区表\n0. 返回上级菜单\n请输入选项: ");
        if (scanf("%d", &choice) != 1) { clear_line(); continue; }
        if (choice == 0) return;
        if (choice == 1) allocate_block(0);
        else if (choice == 2) allocate_block(1);
        else if (choice == 3) free_block();
        else if (choice == 4) show_blocks();
        else printf("无效选项。\n");
    }
}

static int frame_index(int frames[], int frame_count, int page) {
    for (int i = 0; i < frame_count; ++i) if (frames[i] == page) return i;
    return -1;
}

static void print_frames(int step, int page, int frames[], int frame_count, int fault) {
    printf("第 %2d 步 访问页 %3d | ", step, page);
    for (int i = 0; i < frame_count; ++i) {
        if (frames[i] == -1) printf("  - ");
        else printf("%3d ", frames[i]);
    }
    printf("| %s\n", fault ? "缺页" : "命中");
}

static void run_fifo(int refs[], int ref_count, int frame_count) {
    int frames[MAX_FRAMES];
    int next = 0, faults = 0;
    for (int i = 0; i < frame_count; ++i) frames[i] = -1;
    printf("\nFIFO 页面置换\n");
    for (int i = 0; i < ref_count; ++i) {
        int fault = frame_index(frames, frame_count, refs[i]) == -1;
        if (fault) {
            frames[next] = refs[i];
            next = (next + 1) % frame_count;
            faults++;
        }
        print_frames(i + 1, refs[i], frames, frame_count, fault);
    }
    printf("缺页次数: %d，缺页率: %.2f%%\n", faults, 100.0 * faults / ref_count);
}

static void run_lru(int refs[], int ref_count, int frame_count) {
    int frames[MAX_FRAMES], last[MAX_FRAMES];
    int faults = 0;
    for (int i = 0; i < frame_count; ++i) { frames[i] = -1; last[i] = -1; }
    printf("\nLRU 页面置换\n");
    for (int i = 0; i < ref_count; ++i) {
        int pos = frame_index(frames, frame_count, refs[i]);
        int fault = pos == -1;
        if (fault) {
            int victim = 0;
            for (int j = 0; j < frame_count; ++j) {
                if (frames[j] == -1) { victim = j; break; }
                if (last[j] < last[victim]) victim = j;
            }
            frames[victim] = refs[i];
            last[victim] = i;
            faults++;
        } else {
            last[pos] = i;
        }
        print_frames(i + 1, refs[i], frames, frame_count, fault);
    }
    printf("缺页次数: %d，缺页率: %.2f%%\n", faults, 100.0 * faults / ref_count);
}

static void page_demo(void) {
    int frames, count, refs[MAX_REFS], choice;
    printf("请输入页框数 (1-%d): ", MAX_FRAMES);
    if (scanf("%d", &frames) != 1 || frames < 1 || frames > MAX_FRAMES) { clear_line(); printf("页框数无效。\n"); return; }
    printf("请输入页面访问串长度 (1-%d): ", MAX_REFS);
    if (scanf("%d", &count) != 1 || count < 1 || count > MAX_REFS) { clear_line(); printf("访问串长度无效。\n"); return; }
    printf("请输入页面访问串: ");
    for (int i = 0; i < count; ++i) {
        if (scanf("%d", &refs[i]) != 1) { clear_line(); printf("页面号无效。\n"); return; }
    }
    printf("1. FIFO\n2. LRU\n3. 两种算法都运行\n请输入选项: ");
    if (scanf("%d", &choice) != 1) { clear_line(); return; }
    if (choice == 1) run_fifo(refs, count, frames);
    else if (choice == 2) run_lru(refs, count, frames);
    else if (choice == 3) { run_fifo(refs, count, frames); run_lru(refs, count, frames); }
    else printf("无效选项。\n");
}

void memory_menu(void) {
    while (1) {
        int choice;
        printf("\n内存管理\n1. 动态分区分配 FF/BF\n2. 页面置换 FIFO/LRU\n0. 返回上级菜单\n请输入选项: ");
        if (scanf("%d", &choice) != 1) { clear_line(); continue; }
        if (choice == 0) return;
        if (choice == 1) partition_demo();
        else if (choice == 2) page_demo();
        else printf("无效选项。\n");
    }
}
