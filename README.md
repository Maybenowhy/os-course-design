# 操作系统课程设计 

本工程使用 C 语言完成《操作系统》课程设计中的基础必做部分，包含处理机调度、内存管理、进程同步与并发控制、模拟文件系统四个模块，并在进程同步模块补充课程设计提升部分：并发性能优化实验。

## 已完成内容

### 1. 处理机调度

实现了四种典型调度算法：

- FCFS：先来先服务。
- SJF：短作业优先，非抢占式。
- RR：时间片轮转。
- Priority：优先级调度，数值越小优先级越高。

支持动态输入进程名、到达时间、运行时间、优先级和时间片。程序会输出运行顺序、完成时间、周转时间、带权周转时间、平均周转时间和平均带权周转时间。

### 2. 内存管理

实现了两类内存管理实验：

- 动态分区分配：首次适应 FF、最佳适应 BF。
- 页面置换：FIFO、LRU。

动态分区会展示分配、回收、空闲分区合并后的分区表。页面置换会展示每一步页框状态、是否缺页、缺页次数和缺页率。

### 3. 进程同步与并发控制

实现了三个经典同步问题和一个提升实验：

- 生产者-消费者问题。
- 读者-写者问题。
- 哲学家进餐问题。
- 并发性能优化实验：对比单线程基线、多线程全局互斥锁计数、多线程局部计数后汇总。

程序使用线程、互斥锁和信号量模拟并发执行。为了兼容 Windows MinGW 和 Linux，线程相关代码封装在 `src/platform_thread.c` 和 `src/platform_thread.h` 中：Windows 使用 Win32 API，Linux 使用 pthread/semaphore。提升实验通过毫秒级计时统计不同并发计数策略的耗时，体现锁竞争和减少共享写入带来的性能优化效果。

### 4. 模拟文件系统

实现了一个简易内存模拟文件系统，不会修改真实磁盘文件。功能包括：

- 创建文件。
- 写入文件。
- 读取文件。
- 删除文件。
- 列出目录。
- 显示空闲块位图。

文件系统使用固定大小磁盘块和位图管理空闲空间，目录项保存文件名、大小、占用块数和块号列表。

## 目录结构

```text
操作系统实验/
├── Makefile
├── README.md
├── src/
│   ├── main.c
│   ├── scheduler.c / scheduler.h
│   ├── memory.c / memory.h
│   ├── sync_demo.c / sync_demo.h
│   ├── filesystem.c / filesystem.h
│   └── platform_thread.c / platform_thread.h
├── tests/
│   ├── scheduler_sample.txt
│   ├── memory_sample.txt
│   ├── concurrency_perf_sample.txt
│   ├── filesystem_sample.txt
│   └── README.md
```

## 如何编译

在 Windows PowerShell 中进入工程目录：

```powershell
cd C:\Users\Asus\Desktop\操作系统实验
mingw32-make
```

编译成功后会生成：

```text
os_course_design.exe
```

如果在 Linux 环境中编译：

```bash
make
./os_course_design
```

## 如何运行

Windows 下运行：

```powershell
.\os_course_design.exe
```

进入程序后会看到主菜单：

```text
1. 处理机调度
2. 内存管理
3. 进程同步与并发控制
4. 模拟文件系统
0. 退出
```

按数字选择对应模块即可。

## 手动使用说明

### 处理机调度

主菜单输入 `1`，然后输入进程数量和每个进程的信息：

```text
进程名 到达时间 运行时间 优先级
```

示例：

```text
P1 0 7 3
P2 2 4 1
P3 4 1 4
P4 5 4 2
```

随后选择算法：

```text
1. FCFS 先来先服务
2. SJF 短作业优先
3. RR 时间片轮转
4. Priority 优先级调度
5. 运行全部算法
0. 返回上级菜单
```

选择 `5` 可以一次运行全部调度算法。RR 会额外要求输入时间片，例如 `2`。

### 内存管理

主菜单输入 `2`。

选择 `1` 进入动态分区实验：

- 输入总内存大小，例如 `100`。
- 选择 `Allocate FF` 或 `Allocate BF`。
- 输入作业名和大小，例如 `A 20`。
- 选择 `Free` 时输入作业名，例如 `A`。

选择 `2` 进入页面置换实验：

- 输入页框数，例如 `3`。
- 输入页面访问串长度，例如 `12`。
- 输入访问串，例如 `7 0 1 2 0 3 0 4 2 3 0 3`。
- 选择 FIFO、LRU 或 Both。

### 进程同步

主菜单输入 `3`。

可选择：

```text
1. 生产者-消费者
2. 读者-写者
3. 哲学家进餐
4. 运行经典同步演示
5. 并发性能优化实验
0. 返回上级菜单
```

选择 `4` 可以一次运行三个经典同步演示。选择 `5` 会运行课程设计提升部分“并发性能优化实验”，输出单线程基线、多线程全局锁计数、多线程局部汇总三种策略的计数结果和耗时。由于是真实线程调度，每次输出顺序和耗时可能略有不同，但程序应能正常结束，不应死锁。

### 模拟文件系统

主菜单输入 `4`。

可选择：

```text
1. 创建文件
2. 写入文件
3. 读取文件
4. 删除文件
5. 列出目录
6. 显示位图
7. 格式化模拟文件系统
0. 返回上级菜单
```

推荐流程：

1. `Create` 创建文件，例如 `hello`。
2. `Write` 写入内容。
3. `List` 查看目录项和占用块。
4. `Bitmap` 查看空闲块位图。
5. `Read` 读取内容。
6. `Delete` 删除文件并释放块。

## 如何验证

工程提供了四个输入样例，可以用重定向快速验证。

如果在 Windows cmd 中运行，使用以下命令：

```bat
os_course_design.exe < tests\scheduler_sample.txt
os_course_design.exe < tests\memory_sample.txt
os_course_design.exe < tests\filesystem_sample.txt
os_course_design.exe < tests\concurrency_perf_sample.txt
```

如果在 Windows PowerShell 中运行，使用以下命令：

```powershell
Get-Content tests\scheduler_sample.txt | .\os_course_design.exe
Get-Content tests\memory_sample.txt | .\os_course_design.exe
Get-Content tests\filesystem_sample.txt | .\os_course_design.exe
Get-Content tests\concurrency_perf_sample.txt | .\os_course_design.exe
```

### 1. 验证处理机调度

```powershell
Get-Content tests\scheduler_sample.txt | .\os_course_design.exe
```

应能看到 FCFS、SJF、RR、Priority 四种算法的运行顺序和平均周转时间等指标。

### 2. 验证内存管理

```powershell
Get-Content tests\memory_sample.txt | .\os_course_design.exe
```

应能看到动态分区分配/回收过程，以及 FIFO、LRU 页面置换过程和缺页率。

### 3. 验证模拟文件系统

```powershell
Get-Content tests\filesystem_sample.txt | .\os_course_design.exe
```

应能看到文件创建、写入、列目录、显示位图、读取、删除等操作。

### 4. 验证进程同步

同步模块建议手动运行：

```powershell
.\os_course_design.exe
```

然后依次输入：

```text
3
4
0
0
```

应能看到生产者-消费者、读者-写者、哲学家进餐三个演示均正常结束。

### 5. 验证并发性能优化实验

```powershell
Get-Content tests\concurrency_perf_sample.txt | .\os_course_design.exe
```

应能看到单线程基线、多线程全局锁计数、多线程局部汇总三组结果，三组计数均应等于期望总计数。局部汇总策略通常会比全局锁计数耗时更低。

## 清理重新编译

```powershell
mingw32-make clean
mingw32-make
```
