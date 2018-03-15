//author: Divesh Uttamchandani
//Network Programming
//2016A7PS0045P

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<signal.h>
#include<fcntl.h>

//define path for commands here
//not needed using execvp instead
//#define PATH "/bin/"

#define LINESIZE 100
#define MAXNOOFCMD 10
#define MAXFILENAME 32
#define NOOFARGS 32
#define PIPE_BUF 4096

//the primary prompt string
#define PS1 "$ "

#define __DEBUG

#ifdef __DEBUG
#define DEBUG(st) (st)
#else
#define DEBUG(st) ;             //null statement
#endif

#define bool int
#define false 0
#define true 1

void
child_term (int sig, siginfo_t *siginfo, void *context)
{
  int status=0;
  waitpid((long)siginfo->si_pid, &status, 0);
  printf ("EXITED: PID %d, STATUS %d\n", (pid_t)(siginfo->si_pid),  (int)siginfo->si_status);
}

void
ignore (int sig)
{
  return;
}

void
swap (int **a, int **b)
{
  int *c;
  c = *a;
  *a = *b;
  *b = c;
  return;
}

int
remove_substring (char *str, char *startpos, char *endpos, char *string_end)
{
  char *i, *j;
  //DEBUG(printf("in remove substring %s\n", str));
  //DEBUG(printf("in remove substring startpos = %p, endpos = %p , string_end=%p %s\n",startpos, endpos, string_end, str));
  for (i = startpos - 1, j = endpos; j < string_end; i++, j++)
    {
      if (*j != '\0')
        *i = *j;
      else
        {
          *i = ' ';
        }
    }
  *i = '\0';
  return 0;
}

char *
trim (char *str)
{
  //trims the spaces arount str

  //remove from begining
  int i = 0;
  while (isspace (str[i]))
    {
      i++;
    }
  int new_start = i;

  //remove form the end
  int j, new_end;
  j = new_end = strlen (str) - 1;
  if (str[new_start] != '\0')
    {
      while (isspace (str[j]))
        {
          j--;
        }
      new_end = j;
    }
  str[new_end + 1] = '\0';
  return str + new_start;
}

int
parse (char *cmd, char *dest[], char delim[])
{
  int no_of_commands = 0;
  dest[0] = strtok (cmd, delim);
  int i = 1;
  while ((dest[i] = strtok (NULL, delim)) != NULL)
    {
      dest[i - 1] = trim (dest[i - 1]);
      //DEBUG(printf("\n%s",dest[i-1]));
      i++;
    };
  if (i >= 1 && dest[0] != NULL)
    {
      dest[i - 1] = trim (dest[i - 1]);
      //DEBUG(printf("\n%s",dest[i-1]));
    }
  no_of_commands = i;
  //DEBUG(printf("\n%d\n",no_of_commands));

  return no_of_commands;
}

//return *[0] as input fd
//and *[1] as output fd
int *
get_remove_redirection (char *cmd)
{
  //DEBUG(printf("inside get remove redirection cmd = %s\n", cmd));
  int *redirect = (int *) malloc (sizeof (int) * 2);
  redirect[0] = 0;              //stdin
  redirect[1] = 1;              //stdout
  int i, j;

  char *string_end;
  for (string_end = cmd; *string_end != '\0'; string_end++);

  char *filenamei = NULL;
  char *filenameo = NULL;

  //check for input redirection
  int flag_i_redirect = false;

  char *savptr, *savptr1;
  if (strtok_r (cmd, "<", &savptr) != NULL)
    {
      //DEBUG(printf("cmd %s\n",cmd));
      savptr1 = savptr;
      filenamei = strtok_r (NULL, " ", &savptr1);
      //DEBUG(printf("cmd %s\n",cmd));
      if (filenamei != NULL)
        {
          flag_i_redirect = true;
          //DEBUG(printf("inside file i redirect to file %s\n", filenamei));
          remove_substring (cmd, savptr, savptr1, string_end);
        }
      //DEBUG(printf("the stirng changed to %s \n", cmd));
    }

  //check for input redirection
  int flag_o_redirect = false;
  int flag_a_redirect = false;

  for (string_end = cmd; *string_end != '\0'; string_end++);

  if (strtok_r (cmd, ">", &savptr) != NULL)
    {
      savptr1 = savptr;
      filenameo = strtok_r (NULL, " ", &savptr1);
      if (filenameo != NULL && filenameo[0] == '>')
        {
          //DEBUG(printf("inside >> filename = "));
          //DEBUG(printf("%s\n",filenameo));
          flag_a_redirect = true;
          if (filenameo[1] == '\0')
            filenameo = strtok_r (NULL, " ", &savptr1);
          else
            filenameo += 1;
        }

      if (filenameo != NULL)
        {
          flag_o_redirect = true;
          remove_substring (cmd, savptr, savptr1, string_end);
          //DEBUG(printf("inside file o redirect to file %s\n", filenameo));
        }
      //DEBUG(printf("the stirng changed to %s \n", cmd));
    }

  int output_fd;
  int input_fd;
  if (flag_i_redirect)
    {
      input_fd = open (filenamei, O_RDWR, S_IRWXU);
      redirect[0] = input_fd;
      if (input_fd < 0)
        {
          perror ("OPEN 1");
        }
    }
  if (flag_o_redirect)
    {
      if (flag_a_redirect)      //append
        {
          //lseek to end
          output_fd =
            open (filenameo, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
          if (output_fd < 0)
            {
              perror ("OPEN 2");
            }
        }
      else
        {
          output_fd =
            open (filenameo, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
          if (output_fd < 0)
            {
              perror ("OPEN 3");
            }
        }
      redirect[1] = output_fd;
    }
  if (redirect[0] != 0)
    {
      close (0);
      dup (input_fd);
    }
  if (redirect[1] != 1)
    {
      close (1);
      dup (output_fd);
    }
  return redirect;
}

void
execute_cmd_with_redirect (char *cmd)
{
  char cmd1[LINESIZE];
  int outfd;
  //DEBUG(printf("inside execute each cmd = %s\n",cmd));
  int *redirection = get_remove_redirection (cmd);
  strcpy (cmd1, cmd);
  char *parsedargs[NOOFARGS];
  char finalcmd[LINESIZE];
  int no = parse (cmd1, parsedargs, " ");
  parsedargs[no] = (char *) NULL;
  //DEBUG(fprintf(stderr, "\nfinal command execution for %s begins here:\n", parsedargs[0]));
  execvp (parsedargs[0], parsedargs);
}

int
execute_line_with_3_pipe (char *cmd123)
{
  //printf("3 commands\n");
  char cmd[LINESIZE];
  strcpy (cmd, cmd123);
  char *parsedcmds[100];
  int i;
  char *temp;
  char *maincmd, *cmd1, *cmd2, *cmd3;
  maincmd = strtok_r (cmd, "|", &temp);
  //printf("%c, %c\n", *temp, *(temp+1));
  if (!((*temp == '|' && *(temp + 1) == '|')))
    {                           //printf("returning false");
      return false;
    }
  temp += 2;
  //execute and connect pipes
  cmd1 = strtok_r (NULL, ", ", &temp);
  cmd2 = strtok_r (NULL, ", ", &temp);
  cmd3 = strtok_r (NULL, ", ", &temp);
  //fprintf(stderr, "%s, %s ,%s\n", cmd1, cmd2, cmd3);
  int stdinfd = 0;
  int stdoutfd = 1;
  int pipe1[2];
  pipe (pipe1);

  int pid1, pid2, pid3;
  switch (pid1 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      close (stdinfd);
      dup (pipe1[0]);
      close (pipe1[1]);
      // fprintf(stderr,"in process %d executing %s\n", getpid(), cmd1);
      execute_cmd_with_redirect (cmd1);
      break;
    default:
      break;
    }

  int pipe2[2];
  pipe (pipe2);

  //second process
  switch (pid2 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      close (pipe1[0]);
      close (pipe1[1]);
      close (stdinfd);
      dup (pipe2[0]);
      close (pipe2[1]);
      // fprintf(stderr,"in process %d executing %s\n", getpid(), cmd2);
      execute_cmd_with_redirect (cmd2);
      break;
    default:
      break;
    }

  int pipe3[2];
  pipe (pipe3);
  switch (pid3 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      close (pipe1[0]);
      close (pipe1[1]);
      close (pipe2[0]);
      close (pipe2[1]);
      close (stdinfd);
      dup (pipe3[0]);
      close (pipe3[1]);
      // fprintf(stderr,"in process %d executing %s\n", getpid(), cmd2);
      execute_cmd_with_redirect (cmd3);
      break;
    default:
      break;
    }

  pid_t pid0;
  int pipe0[2];
  pipe (pipe0);
  switch (pid0 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      //close other pipes
      close (pipe1[1]);
      close (pipe1[0]);
      close (pipe2[1]);
      close (pipe2[0]);
      close (pipe3[0]);
      close (pipe3[0]);
      close (stdoutfd);
      dup (pipe0[1]);
      close (pipe0[0]);
      //fprintf(stderr,"in process %d executing %s\n", getpid(), maincmd);
      execute_cmd_with_redirect (maincmd);
      //fprintf(stderr,"in process kjflsjfsak%d executing %s\n", getpid(), maincmd);
      break;
    default:
      break;
    }
  //signal(SIGCHLD,child_term);
  int size;
  close (pipe0[1]);
  char buf[PIPE_BUF];
  while ((size = read (pipe0[0], buf, PIPE_BUF)) > 0)
    {
      write (pipe1[1], buf, size);
      write (pipe2[1], buf, size);
      write (pipe3[1], buf, size);
    }
  close (pipe0[0]);
  close (pipe1[0]);
  close (pipe1[1]);
  close (pipe2[0]);
  close (pipe2[1]);
  close (pipe3[1]);
  close (pipe3[1]);
  int status;
  if (cmd123[strlen (cmd123) - 1] != '&')
    {
      waitpid (pid0, &status, 0);
      waitpid (pid1, &status, 0);
      waitpid (pid2, &status, 0);
      waitpid (pid3, &status, 0);
    }
  //fprintf(stderr,"exiting\n");
  return true;
}

int
execute_line_with_2_pipe (char *cmd123)
{
  //printf("2 commands\n");
  char cmd[LINESIZE];
  strcpy (cmd, cmd123);
  char *parsedcmds[100];
  int i;
  char *temp;
  char *maincmd, *cmd1, *cmd2;
  maincmd = strtok_r (cmd, "|", &temp);
  // printf("%c\n", *temp);
  if (!((*temp == '|')))
    {
      return false;
    }
  temp += 1;
  //execute and connect pipes
  cmd1 = strtok_r (NULL, ", ", &temp);
  cmd2 = strtok_r (NULL, ", ", &temp);
  int stdinfd = 0;
  int stdoutfd = 1;
  int pipe1[2];
  pipe (pipe1);

  int pid1, pid2;
  switch (pid1 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      close (stdinfd);
      dup (pipe1[0]);
      close (pipe1[1]);
      // fprintf(stderr,"in process %d executing %s\n", getpid(), cmd1);
      execute_cmd_with_redirect (cmd1);
      break;
    default:
      break;
    }

  int pipe2[2];
  pipe (pipe2);

  //second process
  switch (pid2 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      close (pipe1[0]);
      close (pipe1[1]);
      close (stdinfd);
      dup (pipe2[0]);
      close (pipe2[1]);
      // fprintf(stderr,"in process %d executing %s\n", getpid(), cmd2);
      execute_cmd_with_redirect (cmd2);
      break;
    default:
      break;
    }
  pid_t pid0;
  int pipe0[2];
  pipe (pipe0);
  switch (pid0 = fork ())
    {
    case -1:
      perror ("fork:");
      break;
    case 0:
      //in child
      //close other pipes
      close (pipe1[1]);
      close (pipe1[0]);
      close (pipe2[1]);
      close (pipe2[0]);
      close (stdoutfd);
      dup (pipe0[1]);
      close (pipe0[0]);
      //fprintf(stderr,"in process %d executing %s\n", getpid(), maincmd);
      execute_cmd_with_redirect (maincmd);
      //fprintf(stderr,"in process kjflsjfsak%d executing %s\n", getpid(), maincmd);
      break;
    default:
      break;
    }
  //signal(SIGCHLD,child_term);
  int size;
  close (pipe0[1]);
  char buf[PIPE_BUF];
  while ((size = read (pipe0[0], buf, PIPE_BUF)) > 0)
    {
      write (pipe1[1], buf, size);
      write (pipe2[1], buf, size);
    }
  close (pipe0[0]);
  close (pipe1[0]);
  close (pipe1[1]);
  close (pipe2[0]);
  close (pipe2[1]);
  int status;
  if (cmd123[strlen (cmd123) - 1] != '&')
    {
      waitpid (pid0, &status, 0);
      waitpid (pid1, &status, 0);
      waitpid (pid2, &status, 0);
    }
  //fprintf(stderr,"exiting\n");
  return true;
}

void
execute_line_with_pipe (char *cmd123)
{
  char cmd[LINESIZE];
  strcpy (cmd, cmd123);
  char *parsedcmds[100];
  int i;
  int no_of_commands = parse (cmd, parsedcmds, "|");
  //execute and connect pipes
  int stdinfd = 0;
  int stdoutfd = 1;
  int readpipe[2];
  readpipe[0] = stdinfd;
  readpipe[1] = stdoutfd;
  int writepipe[2];

  //read from read end of readpipe
  //write to write end of writepipe
  pid_t pid, pid1;
  int status, status1;
  int process_count = 0;
  bool flag_background = false;
  for (i = 0; i < no_of_commands; i++)
    {
      //Ignore SIGINT in parent
      bool firstcommand = (i == 0);
      bool lastcommand = (i == no_of_commands - 1);

      //no descriptor open except stdin and stdout
      if (lastcommand)
        {
          writepipe[1] = stdoutfd;      //stdout
          writepipe[0] = stdinfd;       //stdin
          int temp = strlen (parsedcmds[i]);
          if (parsedcmds[i][temp - 1] == '&')
            {
              flag_background = true;
              //printf ("executing to background %s\n", parsedcmds[i]);
            }
          //if(lastletter is &)
          //flag_background=1
        }
      else
        {
          if (pipe (writepipe) == -1)
            {
              perror ("PIPE:");
            }
        }
      //DEBUG(printf("writing to  fd %d\n",writepipe[1]));

      switch (pid = fork ())
        {
          //get a new write pipe for each command
        case 0:                //in child
          //exit on  SIGINT in child
          signal (SIGINT, exit);
          if (!firstcommand && !lastcommand)
            {
              close (readpipe[1]);      // i won't write into readpipe
              close (writepipe[0]);     // i won't read form writepipe
              close (0);
              dup (readpipe[0]);
              close (1);
              dup (writepipe[1]);
            }
          else if (firstcommand && lastcommand)
            {
              //do nothing
              //correct
            }
          else if (firstcommand && !lastcommand)
            {
              close (stdoutfd);
              close (writepipe[0]);
              close (1);
              dup (writepipe[1]);
            }
          else if (lastcommand && !firstcommand)
            {
              close (stdinfd);
              close (readpipe[1]);
              close (0);
              dup (readpipe[0]);
            }
          if (flag_background == true)
            {
              parsedcmds[i][strlen (parsedcmds[i]) - 1] = '\0';
            }
          // DEBUG(fprintf(stderr, "inside child process pid :%d outfd %d, infd %d\n", getpid(), writepipe[1], readpipe[0]));
          execute_cmd_with_redirect (parsedcmds[i]);
        case -1:               //error
          perror ("fork: ");
          exit (1);
          break;
        default:
          //parent is not using any pipes
          if (readpipe[1] != stdinfd && readpipe[1] != stdoutfd)
            close (readpipe[1]);
          if (readpipe[0] != stdinfd && readpipe[0] != stdoutfd)
            close (readpipe[0]);
          //if(writepipe[1]!=stdinfd && writepipe[1]!=stdoutfd)
          //close(writepipe[1]);
          //if(writepipe[0]!=stdinfd && writepipe[0]!=stdoutfd)
          // close(writepipe[0]);
          readpipe[0] = writepipe[0];
          readpipe[1] = writepipe[1];
        }
      //printf ("pid for %s is %d\n", parsedcmds[i], pid);
    }
  //wait for all
  if (readpipe[1] != stdinfd && readpipe[1] != stdoutfd)
    close (readpipe[1]);
  if (readpipe[0] != stdinfd && readpipe[0] != stdoutfd)
    close (readpipe[0]);
  if (writepipe[1] != stdinfd && writepipe[1] != stdoutfd)
    close (writepipe[1]);
  if (writepipe[0] != stdinfd && writepipe[0] != stdoutfd)
    close (writepipe[0]);
  if (!flag_background)
    {                           //if not to be run in background
      waitpid (pid, &status, 0);        //wait for the last command to finish
    }
  return;
}

int
main ()
{
  FILE *intro=fopen("introascii.txt", "r");
  char buf[100];
  while(fgets(buf, 100, intro)!=NULL)
    printf("%s", buf);

  char *cmd;
  cmd = (char *) malloc (sizeof (char) * LINESIZE);
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_sigaction = & child_term;
  act.sa_flags = SA_SIGINFO;
  signal (SIGINT, ignore);
  sigaction(SIGCHLD, &act, NULL);
  do
    {
      if (cmd[0] != '\0')
        if (!execute_line_with_3_pipe (cmd))
          if (!execute_line_with_2_pipe (cmd))
              execute_line_with_pipe (cmd);
      printf (PS1);
    }while (gets(cmd));
  printf ("\nBye!\n");
}
