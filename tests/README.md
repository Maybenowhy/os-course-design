# 测试样例

在工程根目录编译后，可以用重定向方式快速演示。

Windows cmd：

```bat
os_course_design.exe < tests\scheduler_sample.txt
os_course_design.exe < tests\memory_sample.txt
os_course_design.exe < tests\concurrency_perf_sample.txt
os_course_design.exe < tests\filesystem_sample.txt
```

Windows PowerShell：

```powershell
Get-Content tests\scheduler_sample.txt | .\os_course_design.exe
Get-Content tests\memory_sample.txt | .\os_course_design.exe
Get-Content tests\concurrency_perf_sample.txt | .\os_course_design.exe
Get-Content tests\filesystem_sample.txt | .\os_course_design.exe
```

Linux/macOS shell：

```bash
./os_course_design < tests/scheduler_sample.txt
./os_course_design < tests/memory_sample.txt
./os_course_design < tests/concurrency_perf_sample.txt
./os_course_design < tests/filesystem_sample.txt
```

其中 `concurrency_perf_sample.txt` 会运行课程设计提升部分“并发性能优化实验”，对比单线程基线、多线程全局锁计数、多线程局部汇总三种策略的耗时。

同步模块包含线程调度，输出顺序和性能耗时会因系统调度略有差异，建议在菜单中选择 `3 -> 4` 手动运行三个经典同步演示，选择 `3 -> 5` 运行并发性能优化实验。
