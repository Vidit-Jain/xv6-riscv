# xv6-riscv

# Specification 1: syscall tracing
1. Created `strace.c` in `user/`, added `$U/_strace` to UPROGS in Makefile
2. Added a variable `tracemask` in `struct proc` to keep track of what system calls to print by checking the bits in it.
3. Created a `sys_trace()` function in `kernel/sysproc.c`, which assigns the passed value to the tracemask variable of the currently running process.
4. Modified the `fork()` function to pass the tracemask value of the parent to the child created.
5. Added the `sys_trace` function to the array of function pointers `syscalls` in `kernel/syscall.c`.
6. Added macro `SYS_trace` macro in `kernel/syscall.h`, added entry for `trace` in `user/usys.pl`, and definition of the syscall in `user/user.h`.
6. Created two more arrays `syscallnames` and `syscallargc` to store the name of each syscall and to store the number of arguments it takes respectively.
7. In the function `syscall()`, we store the first argument before executing the syscall as it's modifies `a0` in the trapframe.
8. After execution we check if the syscall executed is to be tracked. If it is, we print all the necessary info, the pid using `p->pid`, the syscall name using the `syscallnames` array, and the arguments in integer form by checking the arg count for the syscall using `syscallargc` and the values using the `argint()` function. The return value is stored in `a0` after execution of the syscall.

# Specification 2: Scheduling
1. Added support for passing macros when running `make`. If `make qemu SCHEDULER=MLFQ` is executed in the shell, then all the files will be compiled with a `-D MLFQ` flag, hence defining the macro specified in all files. (NOTE: Run make clean before changing the scheduler as the make file doesn't detect that it needs to recompile all files when a macro is changed)
2. In `proc.c`, an extern variable `schedulingpolicy` is defined. 
    a. Round Robin (DEFSCHED) - 0
    b. First Come First Serve (FCFS) - 1
    c. Priority Based Scheduling (PBS) - 2
    d. Multilevel feedback queue (MLFQ) - 3
3. The value of `schedulingpolicy` is assigned based on the macro defined. If no macro, or an incorrect macro is passed, the default round robin scheduler will be picked. Else, the corresponding scheduler is picked.
4. To support various scheduling policies, the structure of the `scheduler()` function was changed. In the infinite loop, the value of `schedulingpolicy` decides which policy to be used. Each scheduling policy is implemented as a separate function, hence initially the default scheduler was made into a function that is called when `schedulingpolicy = 1`.
5. A function `runprocess()` was created to reduce the code that needs to be rewritten, as we have to run a process in all 4 scheduling policy functions.
## First Come First Served (FCFS)
1. To compare which process was created first, we store a value `createtime` in `struct proc`. In `allocproc()`, the variable is set to the current value of `ticks`.
2. A function `fcfssched()` that is called by `scheduler()` is defined. It goes through all the `RUNNABLE` processes in the process array, and selects the process that has the lowest `createtime` value (created the earliest), and then runs the process.
3. As FCFS is non-preemptive, we go to `usertrap()` and `kerneltrap()` in the `kernel/trap.c` file, and change the lines which check if `which_dev == 2` to yield, as this line checks if it's a clock interrupt, and if yes, it must yield. Instead, we only yield or check if we need to yield if the scheduling policies are default or MLFQ, else we make no attempt at yielding on a timer interrupt. 

## Priority Based Scheduler (PBS)
1. Created a variable `staticpriority` in `struct proc`. Is given a default value 60 in the `allocproc()` function, but can be changed by `setpriority()`.
2. Created variables `runningticks` and `sleepingticks` in `struct proc` to keep track of the total ticks ran and the total ticks for which it was sleeping since it was last scheduled.
3. Created a function `updatetime` that updates the values of `runningticks` and `sleepingticks`
4. Created a variable `schedulecount` in `struct proc` to keep track of how many times a process was scheduled, and is updated every time it's scheduled. Given a default value 0 in `allocproc()`
5. Implemented a function `dynamicpriority()` that takes `struct proc*` and returns the dynamic priority of that process. Uses a default niceness value 5 if total ticks is 0, else it calculates the niceness according to the formula given, and calculates the dynamic priority as well and returns it.
6. Created a function `pbssched()` that schedules processes according to priority.
7. Similar to FCFS, it goes through every process in the process array to find the best process to schedule according to the policy.
8. Due to slight complexity of comparing processes (due to multiple tiebreakers), a function `pbsswap()` was implemented to compare the processes first on dynamic priority(lower is better), if equal then compare the number of times scheduled(lower is better), if again equal then compare the time it was created (lower is better). If we are supposed to select this process over the current best, pbsswap will return 1, else 0.
9. Using the value returned by pbsswap, we iterate through the entire array and pick the best process that is `RUNNABLE`, and run the process.
10. The scheduler is non-preemptive, so we've disabled the timer interrupt yielding the exact same way as FCFS.
### setpriority syscall
1. A new syscall is created called `sys_set_priority`
2. Created `setpriority.c` in `user/`, added `$U/_strace` to UPROGS in Makefile
3. Created a function `changepriority()` in `kernel/proc.c` that changes the static priority of a given process to the value mentioned. Resets the niceness to 5 by setting the `runningticks` and `sleepingticks` to 0. If the dynamic priority is lower in value (higher in priority), then we yield the process to reschedule.
4. Created a `sys_set_priority()` function in `kernel/sysproc.c`, that calls the `changepriority()` function to change the static priority of the process given.
5. Added the `sys_set_priority` function to the array of function pointers `syscalls` in `kernel/syscall.c`.
6. Added macro `SYS_set_priority` macro in `kernel/syscall.h`, added entry for `set_priority` in `user/usys.pl`, and definition of the syscall in `user/user.h`.

## Multilevel Feedback Queue (MLFQ)
1. Defined macros `QCOUNT` and `QSIZE` to represent the number of priority queues and the size of each queue.
2. Created an enum `queued`, and a variable `queuestate` in `struct proc` to store whether the process is in a queue in MLFQ or not.
3. Created variables `queuelevel`, `queueruntime`, `queueentertime` in `struct proc` to keep track of the process, and initialize all to 0 in `allocproc()`.
4. Created a struct `Queue` which implements a circular queue of size `QSIZE` each, and stores a `front` and `back` variables to store the front and back of the queue.
5. Created a file `queue.c` to implement queue-related functions such as `push()`, `pop()`, `remove()`, `empty()`. `push()` pushes a process onto the specified queue and changes it's state to `QUEUED`, `pop()` returns the next process in queue and pops it from the queue as well as changing the state to `NOTQUEUED`. `remove()` removes the process specified from the queue specified, and makes it `NOTQUEUED`. `empty()` returns whether the specified queue is empty or not.
6. Created an array of queues of size `QCOUNT` to store the queued processes in MLFQ.
7. Create a function `queuetableinit()` to initialize all structs of the `queuetable` when starting the xv6.
8. Implemented `mlfqshed()` to schedule processes according to MLFQ policy.
9. The function goes through all the processes in the process array and inserts them into the respective queues if they are `RUNNABLE`, but `NOTQUEUED`.
10. Goes through the `queuetable` to find the process with the lowes priority value, and the first one among them. It picks this process as the next process to run. 
11. In `kernel/trap.c`, we only want to preempt the running process if the running time has exceeded `p->queuelevel`. If we need to preempt it, we decrease the priority (by increasing queue level value)
12. To prevent aging, a function `ageprocesses` is implemented to check the queueentertime of every process every time mlfqsched is called, to check if any process has been waiting for more than `AGE` ticks (which has been set to 64). if so, the priority of that process is increased (decreasing queue level).

