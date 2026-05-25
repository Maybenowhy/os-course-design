#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "scheduler.h"
#include "memory.h"
#include "sync_demo.h"
#include "filesystem.h"

static void clear_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    while (1) {
        int choice;
        printf("\n==== 操作系统课程设计 ===="
               "\n1. 处理机调度"
               "\n2. 内存管理"
               "\n3. 进程同步与并发控制"
               "\n4. 模拟文件系统"
               "\n0. 退出"
               "\n请输入选项: ");
        if (scanf("%d", &choice) != 1) {
            clear_line();
            printf("请输入数字选项。\n");
            continue;
        }
        if (choice == 0) {
            printf("程序已退出。\n");
            break;
        }
        if (choice == 1) scheduler_menu();
        else if (choice == 2) memory_menu();
        else if (choice == 3) sync_menu();
        else if (choice == 4) filesystem_menu();
        else printf("无效选项。\n");
    }
    return 0;
}
