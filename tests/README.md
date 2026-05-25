# 测试样例

在工程根目录编译后，可以用重定向方式快速演示：

```bash
./os_course_design.exe < tests/scheduler_sample.txt
./os_course_design.exe < tests/memory_sample.txt
./os_course_design.exe < tests/filesystem_sample.txt
```

同步模块包含线程调度，输出顺序会因系统调度略有差异，建议在菜单中选择 `3 -> 4` 手动运行全部同步演示。
