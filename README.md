# Lab syscall: System calls
## Task 1: System call tracing
- 目标：添加一个新的系统调用track用于追踪，该系统调用接受一个int类型的参数mask，mask的位说明了需要追踪的系统调用号

<details>
<summary> show code </summary>

首先在表示进程的数据结构中添加mask成员变量，mask的位用于标识需要追踪的系统调用。

然后在userinit()函数中将第一个进程的mask值设置为0。并且在fork()函数中，将父进程的mask值传递给子进程。

```C
// specify which system calls to track
uint64 sys_trace(void) {
    if (argint(0, &myproc()->mask) < 0) 
       return -1;

    return 0;
}
```

```C
void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
    // print if trace
    if (p->mask & (1 << num))
        printf("%d: syscall %s -> %d\n", p->pid, syscall2name[num], p->trapframe->a0);
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
```

</details>

## Task 2: Sysinfo
- 目标：实现系统调用sysinfo收集当前系统的信息，写入到结构体struct sysinfo中

<details>
<summary> show code </summary>

计算空闲空间字节数
```C
// return the number of bytes of free memory
uint64 getFreeMemoryCount() {
    int count = 0;
    struct run *r = kmem.freelist;
    while (r) {
        ++count;
        r = r->next;
    }

    return count * PGSIZE;
}
```

计算进程数
```C
// return the number of processes whose state is not UNUSED
uint64 getUsedProcessesCount() {
    int count = 0;
    struct proc *p;
    for (p = proc; p != &proc[NPROC]; ++p)
        if (p->state != UNUSED)
            ++count;

    return count;
}
```

实现sysinfo系统调用
```C
// collect information about the running system
uint64 sys_sysinfo(void) {
    struct sysinfo si;
    si.freemem = getFreeMemoryCount();
    si.nproc = getUsedProcessesCount();

    uint64 addr;
    if (argaddr(0, &addr) < 0)
        return -1;

    struct proc *p = myproc();
    if (copyout(p->pagetable, addr, (char*)&si, sizeof(struct sysinfo)) < 0)
        return -1;

    return 0;
}
```

</details>