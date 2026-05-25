#include "filesystem.h"
#include <stdio.h>
#include <string.h>

#define MAX_FILES 16
#define BLOCK_COUNT 64
#define BLOCK_SIZE 64
#define MAX_FILE_BLOCKS 8

typedef struct {
    int used;
    char name[16];
    int size;
    int block_count;
    int blocks[MAX_FILE_BLOCKS];
} FileEntry;

static FileEntry files[MAX_FILES];
static int bitmap[BLOCK_COUNT];
static char disk[BLOCK_COUNT][BLOCK_SIZE + 1];
static int initialized = 0;

static void clear_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void read_line(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
}

static void fs_init(void) {
    for (int i = 0; i < MAX_FILES; ++i) files[i].used = 0;
    for (int i = 0; i < BLOCK_COUNT; ++i) { bitmap[i] = 0; disk[i][0] = '\0'; }
    initialized = 1;
}

static int find_file(const char *name) {
    for (int i = 0; i < MAX_FILES; ++i) if (files[i].used && strcmp(files[i].name, name) == 0) return i;
    return -1;
}

static int find_free_entry(void) {
    for (int i = 0; i < MAX_FILES; ++i) if (!files[i].used) return i;
    return -1;
}

static int allocate_block(void) {
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        if (!bitmap[i]) {
            bitmap[i] = 1;
            disk[i][0] = '\0';
            return i;
        }
    }
    return -1;
}

static void release_file_blocks(FileEntry *file) {
    for (int i = 0; i < file->block_count; ++i) {
        int b = file->blocks[i];
        bitmap[b] = 0;
        disk[b][0] = '\0';
    }
    file->block_count = 0;
    file->size = 0;
}

static void create_file(void) {
    char name[16];
    printf("请输入文件名: ");
    if (scanf("%15s", name) != 1) { clear_line(); return; }
    if (find_file(name) != -1) { printf("文件已存在。\n"); return; }
    int entry = find_free_entry();
    if (entry == -1) { printf("目录项已满。\n"); return; }
    files[entry].used = 1;
    strncpy(files[entry].name, name, sizeof(files[entry].name) - 1);
    files[entry].name[sizeof(files[entry].name) - 1] = '\0';
    files[entry].size = 0;
    files[entry].block_count = 0;
    printf("已创建文件 %s。\n", name);
}

static void write_file(void) {
    char name[16];
    char content[BLOCK_SIZE * MAX_FILE_BLOCKS + 1];
    printf("请输入文件名: ");
    if (scanf("%15s", name) != 1) { clear_line(); return; }
    int idx = find_file(name);
    if (idx == -1) { printf("未找到文件。\n"); return; }
    printf("请输入文件内容（最多 %d 个字符）: ", BLOCK_SIZE * MAX_FILE_BLOCKS);
    clear_line();
    read_line(content, sizeof(content));
    int len = (int)strlen(content);
    int need = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (need == 0) need = 1;
    if (need > MAX_FILE_BLOCKS) { printf("文件内容过大。\n"); return; }
    int free_count = 0;
    for (int i = 0; i < BLOCK_COUNT; ++i) if (!bitmap[i]) free_count++;
    if (free_count + files[idx].block_count < need) { printf("空闲磁盘块不足。\n"); return; }
    release_file_blocks(&files[idx]);
    for (int i = 0; i < need; ++i) {
        int b = allocate_block();
        files[idx].blocks[i] = b;
        int offset = i * BLOCK_SIZE;
        strncpy(disk[b], content + offset, BLOCK_SIZE);
        disk[b][BLOCK_SIZE] = '\0';
        files[idx].block_count++;
    }
    files[idx].size = len;
    printf("已写入 %d 字节到文件 %s，占用 %d 个磁盘块。\n", len, name, need);
}

static void read_file_cmd(void) {
    char name[16];
    printf("请输入文件名: ");
    if (scanf("%15s", name) != 1) { clear_line(); return; }
    int idx = find_file(name);
    if (idx == -1) { printf("未找到文件。\n"); return; }
    printf("文件 %s 的内容: ", name);
    for (int i = 0; i < files[idx].block_count; ++i) printf("%s", disk[files[idx].blocks[i]]);
    printf("\n");
}

static void delete_file(void) {
    char name[16];
    printf("请输入文件名: ");
    if (scanf("%15s", name) != 1) { clear_line(); return; }
    int idx = find_file(name);
    if (idx == -1) { printf("未找到文件。\n"); return; }
    release_file_blocks(&files[idx]);
    files[idx].used = 0;
    printf("已删除文件 %s，并释放占用磁盘块。\n", name);
}

static void list_files(void) {
    printf("\n%-12s %-8s %-8s %s\n", "文件名", "大小", "块数", "块号列表");
    for (int i = 0; i < MAX_FILES; ++i) {
        if (files[i].used) {
            printf("%-12s %-8d %-8d ", files[i].name, files[i].size, files[i].block_count);
            for (int j = 0; j < files[i].block_count; ++j) printf("%d ", files[i].blocks[j]);
            printf("\n");
        }
    }
}

static void show_bitmap(void) {
    printf("\n空闲块位图（1=已用，0=空闲）:\n");
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        printf("%d", bitmap[i]);
        if ((i + 1) % 8 == 0) printf(" ");
        if ((i + 1) % 32 == 0) printf("\n");
    }
}

void filesystem_menu(void) {
    if (!initialized) fs_init();
    while (1) {
        int choice;
        printf("\n模拟文件系统\n1. 创建文件\n2. 写入文件\n3. 读取文件\n4. 删除文件\n5. 列出目录\n6. 显示位图\n7. 格式化模拟文件系统\n0. 返回上级菜单\n请输入选项: ");
        if (scanf("%d", &choice) != 1) { clear_line(); continue; }
        if (choice == 0) return;
        if (choice == 1) create_file();
        else if (choice == 2) write_file();
        else if (choice == 3) read_file_cmd();
        else if (choice == 4) delete_file();
        else if (choice == 5) list_files();
        else if (choice == 6) show_bitmap();
        else if (choice == 7) { fs_init(); printf("模拟文件系统已格式化。\n"); }
        else printf("无效选项。\n");
    }
}
