# DSHELL
```
.__    __. .     .___ ._     ._
|  \  /    |   | |    | |    | |
|  |  |__  |---| |--  | |__. | |__.
|__/     | |   | |___ |____| |____|
      .__/
```

shell just supports parsing and execution of commands and invoke coressponding executable.   
It does not have any programmable features (variables etc.).  
The main shell features supported are redirection and piping.  
apart form this two additional types of piping - to pipe single process to multiple(2/3) processes - are supported
shell prints the process id and status of the exited processes.

NETWORK PROGRAMMING ASSIGNMENT
Author: Divesh Uttamchandani

# main concepts used
Systems programming
  - parent child process (forking etc.)
  - inter process communication 
    - pipes
    - signals
  - unix model for file i/o
    - file descriptors (for i/o redirection as well as pipes)
  
# assumptions
input is valid and well spaced, according to the one given in the question  
examples of valid and tested inputs  
arbitrary inputs may raise errors like mixing piping and io redirections arbitraily

in interpreting commands pipes have least pre
- `ls`
- `ls -l`
- `ls | wc | wc | wc`
- `ls || wc, wc`
- `ls ||| wc, wc, wc`
- `wget facebook.com > l &` 

# Design features
## part a
using execvp as no need to check/ maintain path variables.  
forking a process for each of the part within commands i.e different pocess
for ls, wc ,wc etc.  

## part b
using dup to redirect. Though this redirect is not exactly same as the one in 
traditional shells as i first create a file and then execute the command
followed by redirection so `ls > l` shows l also in the output. There was lack
of time so not corrected this. support both > and >> operations as well as `<`

## part c
for pipelineing n processes, using `n-1` pipes, and creating `n` child processes
in the parent, parent connects the processes with pipes, using dup in the
children to override stdin and stdout.  
I am executing all in parallel instead of choosing to wait for first to
finish. this is to ensure that I don't block in case the `PIPE_BUF` is full.
which will be the case in traditional waiting model
NOTE: though the commands run in parallel to to handle `PIPE_BUF`
restrictions, the order of exist is consistent due to IO dependencies.(i.e
read for further commnads will block untill prior command has given some data)
so overall commands run in sequence

## part d
`|| and |||`
the first process (which is created as a child) here redirects to a pipe
the parent i.e my shell process reads from this pipe and writes the same data
to two/three other pipes which are read by other processes. Again using dup in
the child processes
NOTE: In these commands, nothing should be assumed about the order of
exectuion of the recipient commands they are running in concurrently

# my feature implemented
if a command ends with `&` then i won't wait for the commands to finish. i.e 
they run in the background
