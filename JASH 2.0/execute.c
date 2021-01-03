#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<netdb.h>
#include<fcntl.h>
#include<math.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include "headers.h"


/***********************************
Shell: Built-in Function Prototypes
***********************************/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_quit(char **args);
int lsh_echo(char **args);
int lsh_pwd(char **args);
int lsh_history(char **args);
int lsh_pinfo(char **args);
int lsh_jobs(char **args);
int lsh_overkill(char **args);
int lsh_setenv(char **args);
int lsh_unsetenv(char **args);
int lsh_kjob(char **args);
int lsh_bg(char **args);
int lsh_fg(char **args);
int lsh_eexecute(char **args);

#define HISTORY_MAX_SIZE 20
extern unsigned int hCount;
extern char *history[HISTORY_MAX_SIZE];

int sigpid, max;

typedef struct {
    int id;
    int state;
    char name[100];
}process;

extern process parray[100];
extern int pcount;
int g_count;



/************************************
Built-in Commands of the Custom Shell
************************************/
char *builtin_str[] = {
    "cd", "quit", "echo", "pwd", "history" ,"pinfo" ,"jobs", "overkill", "setenv", "unsetenv", "kjob", "bg", "fg", "cronjob"
};

int (*builtin_func[]) (char **) = {
    &lsh_cd, &lsh_quit, &lsh_echo, &lsh_pwd, &lsh_history, &lsh_pinfo, &lsh_jobs, &lsh_overkill, &lsh_setenv, &lsh_unsetenv, &lsh_kjob, &lsh_eexecute, &lsh_eexecute, &lsh_eexecute
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


/************************************
Built-in Command: cd
************************************/
int lsh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}


/************************************
Built-in Command: quit
************************************/
int lsh_quit(char **args) {
    exit(0);
}


/************************************
Built-in Command: echo
************************************/
int lsh_echo(char **args) {
    int i = 0;

    while(args[++i] != NULL) {
        printf("%s ", args[i]);
    }

    printf("\n");
    return 1;
}


/************************************
Built-in Command: pwd
************************************/
int lsh_pwd(char **args) {
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    printf("%s\n", pwd);
}


/************************************
Built-in Command: ls
************************************/
int lsh_ls(char **args) {
    if(args[1] == "-l" && args[2] == "-a" || args[1] == "-a" && args[2] == "-l"|| args[1] == "-al" || args[1] == "-la") {
        DIR *mydir;
        char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        mydir = opendir(pwd);

        char buffer[1028];
        struct stat mystatus;
        struct dirent *myfile;

        while((myfile = readdir(mydir)) != NULL) {
            sprintf(buffer, "%s/%s", args[1], myfile->d_name);
            stat(buffer, &mystatus);
            printf("%ld %s\n", mystatus.st_size, myfile->d_name);

        }

        closedir(mydir);
    } else {
        DIR *mydir;
        mydir = opendir(args[1]);

        char buffer[1028];
        struct stat mystatus;
        struct dirent *myfile;

        while((myfile = readdir(mydir)) != NULL) {
            sprintf(buffer, "%s/%s", args[1], myfile->d_name);
            stat(buffer, &mystatus);
            printf("%ld %s\n", mystatus.st_size, myfile->d_name);

        }

        closedir(mydir); 
    }
    return 1;
}


/************************************
Built-in Command: history
************************************/
int lsh_history(char **args) {
    int i = 0, size, mCount;
    if(!args[1]) {
        size = 10;
    } else {
        size = (*args[1] - '0') - 1;
    }

    size = size < 9 ? size : 9;
 
    int count = 0, iCount = (hCount-size < 0 ? hCount-size : hCount);

    for(i = iCount-1; count <= size && i >= 0; i--) {
        printf("%d   %s\n", ++count, history[i]);
    }
    return 1;
}

int lsh_eexecute(char **args) {
    char string[1024];
    for(int i = 0; i < g_count; i++) {
        strcpy(string, args[i]);
        strcpy(string, " ");
    }
    system(string);
    return 0;
}
/************************************
Built-in Command: pinfo
************************************/
int lsh_pinfo(char **args) {
    if(args[1] != NULL) {
        char p_id[2048], pname[2048], status[2048], fp1[2048] = "/proc/", fp2[2048] ="/proc/",fp3[2048] ="/proc/", exec[2048], fp4[2048];
        int epath;
        FILE *f1, *f2;
        
        strcat(fp1,args[1]); strcat(fp2,args[1]);  strcat(fp3,args[1]); strcat(fp1,"/stat"); strcat(fp2,"/statm"); strcat(fp2,"/exe");
        f1 = fopen(fp1, "r");
        f2 = fopen(fp2, "r");

        if(!f1){
            printf("Invalid Process ID.\n");
            return 1;
        } else {
            fscanf(f1, "%s %s %s", p_id, pname, status);
        }

        if(f2) {
            fscanf(f2, "%d", &epath);
        }

        readlink(fp4, exec, sizeof(exec));
        strcat(exec, "\0");

        printf("Process ID: %s\nProcess Status: %s\nMemory: %d\nExecutable Path: %s\n",p_id, status, epath, exec);


    } else if(args[1] == NULL) {
        char p_id[2048], pname[2048], status[2048], fp[2048] = "/proc/self/exe", exec[2048];
        int epath;
        FILE *f1, *f2;
        pid_t pid = getpid();
        char path1[2048] = "/proc/self/stat", path2[2048] = "/proc/self/statm";
        f1 = fopen(path1, "r");
        f2 = fopen(path2, "r");

        if(f1) {
            fscanf(f1, "%s %s %s", p_id, pname, status);
        }
        if(f2) {
            fscanf(f2, "%d", &epath);
        }

        readlink(fp, exec, sizeof(exec));
        strcat(exec, "\0");

        printf("Process ID: %d\nProcess Status: %s\nMemory: %d\nExecutable Path: %s\n",pid, status, epath, exec);

    }

    return 1;
}


/************************************
Built-in Command: jobs
************************************/
int lsh_jobs(char **args) {

    for(int i = 0; i < pcount; i++) {
        printf("[%d] %s %d\n", i+1, parray[i+1].name, parray[i+1].id);
    }    
}


/************************************
Built-in Command: overkill
************************************/
int lsh_overkill(char **args) {
    int i = 1;
    while(i <= pcount) {
        kill(parray[i].id,SIGKILL);
        ++i;
    }
    return 0;
}


/************************************
Built-in Command: kjob
************************************/
int lsh_kjob(char **args) {
    if(g_count < 3) {
        printf("lsh: Arguments too few.\n");
    } else if(g_count > 3) {
        printf("Arguments too many.\n");
    } else {
        int j = 0, s = 0, i = 0, k = 0;

        while(i < strlen(args[1])) {
            j = j + (args[1][i] - '0')*(int)(pow(10, i));
            ++i;
        }
        while(k < strlen(args[2])) {
            s = s + (args[2][i] - '0')*(int)(pow(10, i));
            ++k;
        }

    }
}


/************************************
Built-in Command: unsetenv
************************************/
int lsh_unsetenv(char **args) {
    if(g_count != 1) {
        unsetenv(args[1]);
    } else {
        perror("lsh: Arguments too few.");
    }
}


/************************************
Built-in Command: setenv
************************************/
int lsh_setenv(char **args) {
    if(!g_count) {
        perror("lsh: Arguments too few.");
    } else if(g_count > 3) {
        perror("lsh: Arguments too many.");
    } else {
        if(g_count == 2) {
            setenv(args[1], " ", 2);
        } else {
            setenv(args[1], args[2], 2);
        }
    }
}


/************************************
Built-in Command: fg
************************************/
int lsh_fg(char **args)
{
	// int sz = 0;
	// for (int i=0; args[i] != NULL; ++i, ++sz);
    int sstatus = 0;
    int sz = g_count;

	if (!(sz == 2))
	{
		fprintf(stderr, "Error: invalid format\n");
		return;
	}
    
	int job_no = atoi(args[1]);
	
    if (!(job_no <= jobs_sz))
	{
		fprintf(stderr, "Error: job number does not exist");
		return;
	}

    int pid, status;

    sstatus = 1;
	pid = atoi(cur_jobs[job_no-1].p_pid);
	kill(pid, SIGCONT);
    sstatus = 2;
    // printf("%d\n",sstatus);
	waitpid(pid, &status, WUNTRACED);
	return;
}


/************************************
Built-in Command: bg
************************************/
int lsh_bg(char **args)
{
	// int sz = 0;
	// for (int i=0; args[i] != NULL; ++i, ++sz);
    int sz = g_count;

	if (!(sz == 2))
	{
		fprintf(stderr, "Error: invalid format\n");
		return;
	}

	int job_no = atoi(args[1]);
	if (!(job_no < jobs_sz))
	{
		fprintf(stderr, "Error: job number does not exist\n");
		return;
	}
    
    int pid;
	pid = atoi(cur_jobs[job_no-1].p_pid);

	printf("name = %s\npid = %d\n", cur_jobs[job_no-1].p_name, pid);
	kill(pid, SIGCONT);
	return;
}




int lsh_lbranch(char **args) {
    int fd, iflag = 0, oc = 0, ac = 0, i = 0;
    char ifile[100], ofile[25][100], afile[25][100];

    while(args[i] != NULL) { 
        if(!strcmp(args[i],">")) {
            args[i] =  NULL;
            strcpy(ofile[oc++],args[i+1]);
        }else if(!strcmp(args[i],"<")) {
            iflag = 1;
            args[i] =  NULL;
            strcpy(ifile,args[i+1]);
        }else if(!strcmp(args[i],">>")) {
            args[i] =  NULL;
            strcpy(afile[ac++],args[i+1]);
        }
        ++i;
    }

        if(iflag) {
            fd = open(ifile, O_RDONLY, 0);
            if(fd < 0) {
                perror("lsh Output File");
            }
            if(dup2(fd, 0) < 0) {
                perror("lsh Output Dup");
            }
            close(fd);
        }

        for(int j = 0; j < oc; j++) {
            fd = open(ofile[j], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if(fd < 0) {
                perror("lsh Output File");
            }
            if(dup2(fd, STDOUT_FILENO) < 0) {
                perror("lsh Output Dup");
            }
            close(fd);
        }

        for(int j = 0; j < ac; j++) {
            fd = open(afile[j], O_WRONLY | O_APPEND | O_CREAT, 0644);
            if(fd < 0) {
                perror("lsh Output File");
            }
            if(dup2(fd, STDOUT_FILENO) < 0) {
                perror("lsh Output Dup");
            }
            close(fd);
        }
}


/*************************************
Shell: Non-Built-in Commands Execution
*************************************/
int lsh_launch(char **args, int background, int count) {

    if(!count) {
        return 0;
    }

    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        if(args[0][0] != '\0') {
            lsh_lbranch(args);
        }
        if(execvp(args[0], args) == -1) {
            perror("lsh Error");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("lsh Error");
    } else {
        if(!background) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while(!WIFEXITED(status) && !WIFSIGNALED(status));
            printf("%s with PID %d exited successfully.\n", args[0], pid);
        } else {
            ++pcount;
            parray[pcount].id = (int)pid;
            strcpy(parray[pcount].name,args[0]);
            printf("%s with PID %d exited normally.\n", args[0], pid);
        }
    }

    return 1;
}

/*
int sigpid;
void  SIGINT_handler(int signal_num)
{	
	int k=0;

	if(sigpid){
		if(kill(sigpid,SIGINT)) 
		{			
			return;
		}
		k =1;
	}
	if(!k){
		signal(signal_num, SIG_IGN); 
		printf("\n");
		print_prompt();
		//printf("form signal\n");
		fflush(stdout); 
		signal(signal_num, SIGINT_handler);
	}
}*/



/************************************
Shell: Built-in Commands Execution
************************************/
int lsh_execute(char **args, int background, int count) {
    int i;
    g_count = count;

    if(count == 0) {
        return 0;
    }

    if(strcmp(*args, "quit") == 0) {
        exit(0);
    }

    if(!background) {
        for(i = 0; i < lsh_num_builtins(); i++) {
            if(strcmp(args[0], builtin_str[i]) == 0) {
                return (*builtin_func[i]) (args);
            }
        }
    }

    return lsh_launch(args, background, count);
}