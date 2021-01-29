#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <termios.h>
#include <fcntl.h>
#define MAXSIZE 1000
#define MAXLIST 1000 
#define HISTORY_COUNT 1000
#define clear() printf("\033[H\033[J")
#define COLOR_NONE "\033[m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_LIGHT_CYAN "\033[1;36m"
#define COLOR_LIGHT_GREEN "\033[1;32m"
#define COLOR_LIGHT_RED "\033[1;31m"
int Built_ins(char** parsed);

int isBuiltInCommand(char**parsed);
pid_t pid;
char *hist[HISTORY_COUNT];
int children[1000]={0},childCount=0,status,i,current=0,hist_num=1;

void init_shell()
{
    clear();
    printf(COLOR_LIGHT_GREEN "\n********************************************************WELCOME TO MY SHELL****************************************************************\n");
}

int takeInput(char* str)
{
    char* buffer;
    buffer=readline(" ");
    if(strlen(buffer)!=0) 
    {
        hist[current]=strdup(buffer);
        current=(current + 1) % HISTORY_COUNT;
        strcpy(str, buffer);
        
        return 0;
    } 
    else 
    {
        return 1;
    }
}
 
void execArgs(char** parsed) 
{ 
    // Forking a child 
    pid_t pid = fork();  
  
    if (pid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) { 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command.."); 
        } 
        exit(0); 
    } else { 
        // waiting for child to terminate 
        wait(NULL);  
        return; 
    } 
} 
  
// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            // parent executing, waiting for two children 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 

int parsePipe(char* str, char** strpiped) 
{ 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0; // returns zero if no pipe is found. 
    else { 
        return 1;
        //printf("dff\n"); 
    } 
} 
void parseSpace(char* str, char** parsed)
{
    int i;
 
    for(i=0;i<MAXLIST;i++) 
    {
        parsed[i]=strsep(&str," ");
       
        if(parsed[i]==NULL)
        {
            break;
        }
        if(strlen(parsed[i])==0)
        {
            i--;
        }
    }
}

int processString(char* str, char** parsed, char** parsedpipe) 
{ 
  
    char* strpiped[2]; 
    int piped = 0; 
  
    piped = parsePipe(str, strpiped); 
  
    if (piped) { 
        parseSpace(strpiped[0], parsed); 
        parseSpace(strpiped[1], parsedpipe); 
  
    } else { 
  
        parseSpace(str, parsed); 
    } 
  
    if (isBuiltInCommand(parsed)) 
        return 0; 
    else
        return 1 + piped; 
} 

void Help()
{
    printf("\n*******WELCOME TO MY SHELL HELP*******"
        "\n\nList of Commands supported:"
        "\n---------------------------"
        "\n\nmessage-    Message for you!"
        "\n\njobs-       Lists the processes running in the background"
        "\n\ncd-         Changes current directory"
        "\n\nhistory-    Displays the history of commands used"
        "\n\nclrhistory- Clears the history of commands used"
        "\n\nclear-      Clears the terminal"
        "\n\nexit-       Exits from the shell"
        "\n\nkill-       Kills the background processes using job id"
        "\n\nkillAll-    Kills all the background processes using their job ids"
        "\n\nls or /bin/ls -  Lists the files in the current working directory (Both relative or absolute pathnames)"
        "\n\n./some_code & -  Run as background process using &"
        "\n\n./some_code < infile > outfile -  Input Output file redirection"
        "\n\ngcc some_code.c -o some_code  - Compilation and Execution of .c files\n");
 
    return;
}
 
void printDirectory()
{
    char cwd[1024],hostname[1024];
    char* username=getenv("USER");
    getcwd(cwd, sizeof(cwd));
    gethostname(hostname, 1024);
 
    printf( COLOR_LIGHT_RED"\n%s"COLOR_YELLOW "@" COLOR_YELLOW"%s" ":" COLOR_LIGHT_CYAN "%s",username,hostname,cwd);
    printf(COLOR_NONE ">>" COLOR_NONE " ");
}

int changeDirectory(char* args[])
{
        if(args[1]==NULL) 
        {
	   chdir(getenv("HOME")); 
	   return 1;
	}
	else
        { 
	   if (chdir(args[1])==-1) 
           {
	      printf("\n%s: No such directory\n",args[1]);
              return -1;
	   }
	}
	return 0;
}

void redirectfileIO(char * args[], char* inputFile, char* outputFile, int option)
{
	int err=-1,fileDescriptor; 
	
	if((pid=fork())==-1)
        {
	    printf("\nFailed forking child\n");
	    return;
	}
	if(pid==0)
        {
		if (option==0)
                {
		   fileDescriptor=open(outputFile,O_CREAT | O_TRUNC | O_WRONLY,0600); 
		   dup2(fileDescriptor,STDOUT_FILENO); 
		   close(fileDescriptor);
		}
                else if (option==1)
                {
                  fileDescriptor=open(inputFile,O_RDONLY,0600);  
		  dup2(fileDescriptor,STDIN_FILENO);
		  close(fileDescriptor);
		  fileDescriptor=open(outputFile,O_CREAT | O_TRUNC | O_WRONLY,0600);
		  dup2(fileDescriptor,STDOUT_FILENO);
		  close(fileDescriptor);		 
		}
		if (execvp(args[0],args)==err)
                {
		    printf("\nerror\n");
		    kill(getpid(),SIGTERM);
		}		 
	}
	waitpid(pid,NULL,0);
}

int history(char *hist[], int current)
{
        int i=current;
        hist_num=1;
        do {
                if (hist[i]) 
                {
                    printf("%d  %s\n", hist_num, hist[i]);
                    hist_num++;
                }
                i = (i + 1)%HISTORY_COUNT;

        } while (i != current);
  
        return 0;
}

int clearHistory(char *hist[])
{
        int i;
        for (i = 0; i < HISTORY_COUNT; i++) 
        {
           free(hist[i]);
           hist[i] = NULL;
        }
        return 0;
}

void jobs()
{
  int j;
  char*stat;
  for(j = 0;j<childCount;j++)
  {
    if(kill(children[j],0) == 0)
    {
       stat="Job Running";
    }
    else
    {
      stat="Job Done";
    }
    printf("\n[%d]    %s     %d\n", j+1,stat,children[j]);
  }
return;
}

void Execute_command(char **args, int background)
{	 
	 int err=-1;
         if((pid=fork())==-1)
         {
            printf("\nFailed forking child\n");
            return;
	 }
	 if(pid==0)
         {	
	   signal(SIGINT,SIG_IGN);
	   if(execvp(args[0],args)<0)
           {
	      printf("\nCommand does not exist\n");
	      kill(getpid(),SIGTERM);
	   }
	 }
	 if(background==0)
         {
            waitpid(pid,NULL,0);
	 }
         else
         {
	    printf("\nProcess created in the background with PID: %d\n",pid);
            children[childCount]=pid;
            childCount++;
            printf("\n[%d] %d\n\n",childCount,pid);
	 }	 
}

int check_command(char * args[])
{
        int aux,background=0,i=0,j=0;
	char *args_aux[256];
        memset(args_aux,'\0',256);
	
	while ( args[j]!=NULL)
        {
	  if((strcmp(args[j],">")==0) || (strcmp(args[j],"<")==0) || (strcmp(args[j],"&")==0) || (strcmp(args[j],"!")==0) || (strcmp(args[j],"!-")==0))
          {
	    break;
	  }
	  args_aux[j] = args[j];
	  j++;
	}
      while (args[i] != NULL && background == 0)
      {
	  if (strcmp(args[i],"&") == 0)
          {
	    background = 1;
          }
	  else if (strcmp(args[i],"<") == 0)
          {
	    aux = i+1;
	      if (args[aux] == NULL || args[aux+1] == NULL || args[aux+2] == NULL )
              {
		 printf("Invalid Command Syntax\n");
		 return -1;
	      }
              else
              {
		  if (strcmp(args[aux+1],">") != 0)
                  {
		     printf("Syntax: Expected '>' and found %s\n",args[aux+1]);
		     return -2;
		  }
		}
		redirectfileIO(args_aux,args[i+1],args[i+3],1);
	        return 1;
	    }
		else if (strcmp(args[i],">") == 0)
                 {
		   if (args[i+1] == NULL)
                   {
			printf("Invalid Command Syntax\n");
			 return -1;
		   }
		   redirectfileIO(args_aux,NULL,args[i+1],0);
		   return 1;
		 }
                 else if(strcmp(args[i],"!")==0)
                 {
                   aux = i+1;
                   if (args[aux]==NULL)
                   {
		     printf("Invalid Command Syntax\n");
		     return -1;
	           }
                   else
                   {
                      int k;
                      k=atoi(args[1]);
                      if(k<=0)
                      {
                         printf("\nInvalid syntax\n");
                         return -1;
                      }
                      if(k>=HISTORY_COUNT)
                      {
                         printf("\nExceeds maximum history limit\n");
                         return -1;
                      }
                      if(hist[k-1]==NULL)
                      {
                         printf("\nExceeds current history created\n");
                         return -1;
                      }
                      printf("\n%s\n",hist[k-1]);
                      return 1;
                   }
                 }
                  else if(strcmp(args[i],"!-")==0)
                 {
                   aux = i+1;
                   if (args[aux]==NULL)
                   {
		     printf("Invalid Command Syntax\n");
		     return -1;
	           }
                   else
                   {
                      int k;
                      k=atoi(args[1]);
                      if(k<=0)
                      {
                         printf("\nInvalid syntax\n");
                         return -1;
                      }
                      if(k>=HISTORY_COUNT)
                      {
                         printf("\nExceeds maximum history limit\n");
                         return -1;
                      }
                      if(hist[hist_num-k-1]==NULL)
                      {
                         printf("\nExceeds current history created\n");
                         return -1;
                      }
                      printf("\n%s\n",hist[hist_num-k-1]);
                      return 1;
                   }
                 }
                  if(args[i+1]!=NULL)
                  {
                   if(strcmp(args[i+1],"%")==0)
                  {
                   aux=i;
                   if(args[aux+1]=="%" || args[aux]=="kill" || args[aux+2]!=NULL)
                   {
                      int job_id=-1;
                      job_id=atoi(args[aux+2]);
                      if(job_id<=0)
                      {
                         printf("\nInvalid job id\n");
                         return -1;
                      }
                      pid=children[job_id-1];
                      
                      if (pid<=0) 
                      {
                        printf("\nInvalid job id\n");
                        return -1;
                      }
                      kill(pid,SIGKILL);
                      waitpid(children[job_id-1],NULL,WNOHANG);
                      printf("\njobid: %d with  pid: %d is killed\n",job_id,pid);
                      return 1;
                    }
                    else
                    {
                      printf("\nInvalid Syntax!\n");
                      return -1;
                    }
                  }	
                 }
		i++;
	  }
		args_aux[i]=NULL;
		Execute_command(args_aux,background);

                return 1;
}

void killAll()
{
  int i;
  for(i=0;i<childCount;i++)
  {
     pid=children[i];
     if((kill(pid,0))==0)
     {
       kill(pid,SIGKILL);
       waitpid(pid,NULL,WNOHANG);
       printf("\njobid: %d with  pid: %d is killed\n",i+1,pid);
     }
  }
}

int isBuiltInCommand(char**parsed)
{
   int NoOfOwnCmds=8, i, switchOwnArg=0;
    char* ListOfOwnCmds[NoOfOwnCmds];
 
    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "message";
    ListOfOwnCmds[4] = "history";
    ListOfOwnCmds[5] = "clrhistory";
    ListOfOwnCmds[6] = "jobs"; 
    ListOfOwnCmds[7] = "killAll"; 
	 ListOfOwnCmds[8] = "ls|wc"; 
    for (i = 0; i < NoOfOwnCmds; i++) 
    {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) 
        {
          return 1;
        }
    }
    return 0;
}

int Built_ins(char** parsed)
{
    int NoOfOwnCmds=8,i,switchOwnArg=0,count=0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char *username;
    char cwd[1024];
    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "message";
    ListOfOwnCmds[4] = "history";
    ListOfOwnCmds[5] = "clrhistory";
    ListOfOwnCmds[6] = "jobs"; 
    ListOfOwnCmds[7] = "killAll"; 
    ListOfOwnCmds[8] = "ls|wc"; 
    for (i = 0; i < NoOfOwnCmds; i++) 
    {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) 
        {
            switchOwnArg = i + 1;
            break;
        }
    }
 
    switch (switchOwnArg) 
    {
    case 1:
        for(i=0;i<childCount;i++)
        { 
          if(kill(children[i],0)==0)
          {
            count++;
          }
        }
        if(count>0)
        {
          printf("\nProcesses are running in the background!\nYou can exit only after killing the background processes.\nYou can kill them using killAll command or kill them individually using job id.\n");
          return 1;
        }
        else
        {
        printf(COLOR_LIGHT_GREEN "\n\n************************************************************SHELL CLOSED******************************************************************\n\n");
        exit(0);
        }
    case 2:
        changeDirectory(parsed);
        return 1;
    case 3:
        Help();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nYou can use help to know more!\n",username);
        return 1;
    case 5:
          history(hist, current);
        return 1;
    case 6:
          clearHistory(hist);
        return 1;
    case 7:
          jobs();
        return 1;
    case 8:
          killAll();
        return 1;
	case 9:
          system("pipe.c");
        return 1;
    default:
        break;
    }
  return 0;
}
 
int main()
{
    char inputString[MAXSIZE],*parsedArgs[MAXLIST];
	 char* parsedArgsPiped[MAXLIST]; 
    int execFlag = 0; 
    init_shell();
    while (1) 
    {
        printDirectory();
        if (takeInput(inputString))
        {
            continue;
        }
        //parseSpace(inputString, parsedArgs);
		execFlag = processString(inputString, 
        parsedArgs, parsedArgsPiped); 
        // execflag returns zero if there is no command 
        // or it is a builtin command, 
        // 1 if it is a simple command 
        // 2 if it is including a pipe. 
  
        // execute 
        if (execFlag == 1) 
            execArgs(parsedArgs); 
  
        if (execFlag == 2) 
            execArgsPiped(parsedArgs, parsedArgsPiped); 
 
        if (isBuiltInCommand(parsedArgs))
        {
          Built_ins(parsedArgs);
        }
        else
        {
          check_command(parsedArgs);
        }    
 
    }
    return 0;
}
