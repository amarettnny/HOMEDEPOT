CONTRIBUTIONS

    Tianji finished Task 1, 2, and 4
    Siyang finished Task 3

REPORT
1. Time output of the program:
Test run with threshold 2097152

real	0m0.399s
user	0m0.386s
sys	0m0.011s
Test run with threshold 1048576

real	0m0.215s
user	0m0.369s
sys	0m0.028s
Test run with threshold 524288

real	0m0.172s
user	0m0.423s
sys	0m0.047s
Test run with threshold 262144

real	0m0.123s
user	0m0.439s
sys	0m0.043s
Test run with threshold 131072

real	0m0.138s
user	0m0.457s
sys	0m0.066s
Test run with threshold 65536

real	0m0.134s
user	0m0.483s
sys	0m0.079s
Test run with threshold 32768

real	0m0.120s
user	0m0.499s
sys	0m0.106s
Test run with threshold 16384

real	0m0.135s
user	0m0.546s
sys	0m0.160s

2.
First, when the threshold is large, the time is large. Since only a few
child processes are created, and each child process is comparatively big, 
each child process will take a comparatively large amount of time to sort 
this large amount of data, resulting in a larger runtime.

When we decrease the threshold, we can see that the real time is getting 
lower. This is because there are more child processes created and parallel,
and each child process is smaller and takes a shorter runtime to sort.

However, a diminishing return will be countered when we continue to 
decrease the threshold. Even though there are more child processes and each 
process is taking less time to execute, there will be an increasing number 
of "forks" of the processes and OS needs to take time for system calls(like
fork, waitpid, etc.) and parallel different processes to different CPU cores. 
The increasing time spent on these factors cancels out the time saved by 
parallelism. 
