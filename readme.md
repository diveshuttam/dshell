# NETWORK PROGRAMMING ASSIGNMENT

Divesh Uttamchandani

# assumptions
input is valid and well spaced, according to the one given in the question
examples of valid and tested inputs
arbitrary inputs may raise errors
- ls
- ls -l
- ls | wc | wc | wc
- ls || wc, wc
- ls ||| wc, wc, wc
- wget facebook.com > l & 

# Design features
## part a
using execvp as no need to check/ maintain path variables.
forking a process for each of the process.

## part b
using dup to redirect. Though this redirect is not exactly same as the one in
traditional shells as i first create a file and then execute the command
followed by redirection so ls > l shows l also in the output. There was lack
of time so not corrected this. support both > and >> operations as well as <

## part c
for pipelineing n processes, using n-1 pipes, and creating n child processes
in the parent, parent connects the processes with pipes, using dup in the
children to override stdin and stdout.
I am executing all in parallel instead of choosing to wait for first to
finish. this is to ensure that I don't block in case the PIPE_BUF is full.
which will be the case in traditional waiting model
NOTE: though the commands run in parallel to to handle PIPE_BUF
restrictions, the order of exist is consistent due to IO dependencies.(i.e
read for further commnads will block untill prior command has given some data) so overall commands run in sequence

## part d
|| and |||
the first process (which is created as a child) here redirects to a pipe
the parent i.e my shell process reads from this pipe and writes the same data
to two/three other pipes which are read by other processes. Again using dup in
the child processes
NOTE: In these commands, nothing should be assumed about the order of
exectuion of the recipient commands they are running in concurrently

# my feature implemented
if a command ends with '&' then i won't wait for the commands to finish. i.e 
they run in the background
