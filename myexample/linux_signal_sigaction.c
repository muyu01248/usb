 linux 信号signal和sigaction理解
2013-03-16 12:29 19008人阅读 评论(3) 收藏 举报
分类：
linux_系统编程（12）

版权声明：本文为博主原创文章，未经博主允许不得转载。

今天看到unp时发现之前对signal到理解实在浅显，今天拿来单独学习讨论下。


signal，此函数相对简单一些，给定一个信号，给出信号处理函数则可，当然，函数简单，其功能也相对简单许多，简单给出个函数例子如下：

[cpp] view plain copy

     1 #include <signal.h>  
     2 #include <stdio.h>  
     3 #include <unistd.h>  
     4   
     5 void ouch(int sig)  
     6 {  
     7     printf("I got signal %d\n", sig);  
     8     // (void) signal(SIGINT, SIG_DFL);  
     9     //(void) signal(SIGINT, ouch);  
    10   
    11 }  
    12   
    13   
    14   
    15 int main()  
    16 {  
    17     (void) signal(SIGINT, ouch);  
    18   
    19     while(1)  
    20     {  
    21         printf("hello world...\n");  
    22         sleep(1);  
    23     }  
    24 }  

当然，实际运用中，需要对不同到signal设定不同的到信号处理函数，SIG_IGN忽略/SIG_DFL默认，这俩宏也可以作为信号处理函数。同时SIGSTOP/SIGKILL这俩信号无法捕获和忽略。注意，经过实验发现，signal函数也会堵塞当前正在处理的signal，但是没有办法阻塞其它signal，比如正在处理SIG_INT，再来一个SIG_INT则会堵塞，但是来SIG_QUIT则会被其中断，如果SIG_QUIT有处理，则需要等待SIG_QUIT处理完了，SIG_INT才会接着刚才处理。


sigaction，这个相对麻烦一些，函数原型如下：

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)；

函数到关键就在于struct sigaction
[cpp] view plain copy

    stuct sigaction  
    {  
          void (*)(int) sa_handle;  
          sigset_t sa_mask;  
          int sa_flags;  
    }  


[cpp] view plain copy

    1 #include <signal.h>  
      2 #include <stdio.h>  
      3 #include <unistd.h>  
      4   
      5   
      6 void ouch(int sig)  
      7 {  
      8     printf("oh, got a signal %d\n", sig);  
      9   
     10     int i = 0;  
     11     for (i = 0; i < 5; i++)  
     12     {  
     13         printf("signal func %d\n", i);  
     14         sleep(1);  
     15     }  
     16 }  
     17   
     18   
     19 int main()  
     20 {  
     21     struct sigaction act;  
     22     act.sa_handler = ouch;  
     23     sigemptyset(&act.sa_mask);  
     24     sigaddset(&act.sa_mask, SIGQUIT);  
     25     // act.sa_flags = SA_RESETHAND;  
     26     // act.sa_flags = SA_NODEFER;  
     27     act.sa_flags = 0;  
     28   
     29     sigaction(SIGINT, &act, 0);  
     30   
     31   
     32     struct sigaction act_2;  
     33     act_2.sa_handler = ouch;  
     34     sigemptyset(&act_2.sa_mask);  
     35     act.sa_flags = 0;  
     36     sigaction(SIGQUIT, &act_2, 0);  
     37   
            while(1)  
            {  
                 sleep(1);  
            }  
     38     return;  
      
        }  


1. 阻塞，sigaction函数有阻塞的功能，比如SIGINT信号来了，进入信号处理函数，默认情况下，在信号处理函数未完成之前，如果又来了一个SIGINT信号，其将被阻塞，只有信号处理函数处理完毕，才会对后来的SIGINT再进行处理，同时后续无论来多少个SIGINT，仅处理一个SIGINT，sigaction会对后续SIGINT进行排队合并处理。

2. sa_mask，信号屏蔽集，可以通过函数sigemptyset/sigaddset等来清空和增加需要屏蔽的信号，上面代码中，对信号SIGINT处理时，如果来信号SIGQUIT，其将被屏蔽，但是如果在处理SIGQUIT，来了SIGINT，则首先处理SIGINT，然后接着处理SIGQUIT。

3. sa_flags如果取值为0，则表示默认行为。还可以取如下俩值，但是我没觉得这俩值有啥用。

SA_NODEFER，如果设置来该标志，则不进行当前处理信号到阻塞

SA_RESETHAND，如果设置来该标志，则处理完当前信号后，将信号处理函数设置为SIG_DFL行为


下面单独来讨论一下信号屏蔽，记住是屏蔽，不是消除，就是来了信号，如果当前是block，则先不传递给当前进程，但是一旦unblock，则信号会重新到达。

[cpp] view plain copy

    #include <signal.h>  
    #include <stdio.h>  
    #include <unistd.h>  
      
      
      
    static void sig_quit(int);  
      
    int main (void) {  
        sigset_t new, old, pend;  
          
        signal(SIGQUIT, sig_quit);  
      
        sigemptyset(&new);  
        sigaddset(&new, SIGQUIT);  
        sigprocmask(SIG_BLOCK, &new, &old);  
      
        sleep(5);  
      
        printf("SIGQUIT unblocked\n");  
        sigprocmask(SIG_SETMASK, &old, NULL);  
      
        sleep(50);  
        return 1;  
    }  
      
    static void sig_quit(int signo) {  
        printf("catch SIGQUIT\n");  
        signal(SIGQUIT, SIG_DFL);  
    }  


gcc -g -o mask mask.c 

./mask

========这个地方按多次ctrl+\

SIGQUIT unblocked

catch SIGQUIT
Quit (core dumped)

======================

注意观察运行结果，在sleep的时候，按多次ctrl+\，由于sleep之前block了SIG_QUIT，所以无法获得SIG_QUIT，但是一旦运行sigprocmask(SIG_SETMASK, &old, NULL);则unblock了SIG_QUIT，则之前发送的SIG_QUIT随之而来。

由于信号处理函数中设置了DFL，所以再发送SIG_QUIT，则直接coredump。