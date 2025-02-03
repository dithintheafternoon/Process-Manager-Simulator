This is a process manager simulator that will schedule CPU bound processes in a round robin manager and allocate memory differently depending on arguments passed via command line. 
The compiled file is called allocate
In the command line, write:
allocate -f <filename> -m (infinite | first-fit | paged | virtual) -q (1 | 2 | 3)
e.g. ./allocate -f processes.txt -m infinite -q 3

<filename> contains processes to be schedule. It should be in the form of
0 P4 30 16
29 P2 40 64
99 P1 20 32
Where process P4 arrives at time 0, requires 30 seconds of CPU time to finish, and requires 16 KB of memory
