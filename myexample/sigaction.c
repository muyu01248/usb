 Linux系统sigaction函数的使用示例
标签： Linux信号sigaction
2015-09-01 16:18 3537人阅读 评论(0) 收藏 举报
分类：
4.Linux高级编程（27）

版权声明：本文为博主原创文章，未经博主允许不得转载。

一、函数原型

int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);

    1
    2
    3

signum参数指出要捕获的信号类型，act参数指定新的信号处理方式，oldact参数输出先前信号的处理方式（如果不为NULL的话）。

二、 struct sigaction结构体介绍

struct sigaction {
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
}

    1
    2
    3
    4
    5
    6
    7

    sa_handler此参数和signal()的参数handler相同，代表新的信号处理函数
    sa_mask 用来设置在处理该信号时暂时将sa_mask 指定的信号集搁置
    sa_flags 用来设置信号处理的其他相关操作，下列的数值可用。
        SA_RESETHAND：当调用信号处理函数时，将信号的处理函数重置为缺省值SIG_DFL
        SA_RESTART：如果信号中断了进程的某个系统调用，则系统自动启动该系统调用
        SA_NODEFER ：一般情况下， 当信号处理函数运行时，内核将阻塞该给定信号。但是如果设置了 SA_NODEFER标记， 那么在该信号处理函数运行时，内核将不会阻塞该信号

三、示例代码

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int main()
{
    struct sigaction newact,oldact;

    /* 设置信号忽略 */
    newact.sa_handler = SIG_IGN; //这个地方也可以是函数
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    int count = 0;
    pid_t pid = 0;

    sigaction(SIGINT,&newact,&oldact);//原来的备份到oldact里面

    pid = fork();
    if(pid == 0)
    {
        while(1)
        {
            printf("I'm child gaga.......\n");
            sleep(1);
        }
        return 0;
    }

    while(1)
    {
        if(count++ > 3)
        {
            sigaction(SIGINT,&oldact,NULL);  //备份回来
            printf("pid = %d\n",pid);
            kill(pid,SIGKILL); //父进程发信号，来杀死子进程
        }

        printf("I am father .......... hahaha\n");
        sleep(1);
    }

    return 0;
}