/*******************************************
Computer Science Engineering I: Assignment02
under Prof. P. Krishna Reddy, IIIT Hyderabad 
*******************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<netdb.h>
#include<sys/stat.h>
#include<sys/wait.h>

#define COLOR_NONE "\033[m"
#define COLOR_RED "\033[1;37;41m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_GREEN "\033[0;32;32m"
#define COLOR_GRAY "\033[1;30m"



/*****************************************
Function for the Implementation of history 
*****************************************/
#define HISTORY_MAX_SIZE 20
static char *history[HISTORY_MAX_SIZE];
static unsigned int hCount = 0;

void add_history(const char *command) {
    if(hCount < HISTORY_MAX_SIZE) {
        history[hCount++] = strdup(command);
    } else {
        free(history[0]);
        for(int i = 0; i < HISTORY_MAX_SIZE; i++) {
            history[i-1] = history[i];
        }
        history[HISTORY_MAX_SIZE-1] = strdup(command);
    }
}




/****************************************
 LOOP:Step 01 Read the command from stdin
****************************************/
#define LSH_RL_BUFSIZE 1024
int background = 0;

char *lsh_read_line(void) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "read:allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar();

        if(c == '&') {
            background = 1;
            continue;
        }

        if(c == EOF || c == '\n' || c == ';') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        ++position;

        if(bufsize <= position) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);

            if(!buffer) {
                fprintf(stderr, "read: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*
char *lsh_read_line(void) {
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}
*/



/**************************************
 LOOP: Step 02 Parse the command string
**************************************/
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if(!token) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if(position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if(!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}



/************************************
 LOOP:Step 03  Run the parsed command
************************************/
int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("lsh");
    } else {
        if(!background) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while(!WIFEXITED(status) && !WIFSIGNALED(status));
            printf("%s with PID %d exited successfully.\n", args[0], pid);
        } else {
            printf("%s with PID %d exited normally.\n", args[0], pid);
        }
    }

    background = 0;
    return 1;
}



/*****************************************
Function definitions for built-in commands
*****************************************/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_echo(char **args);
int lsh_pwd(char **args);
int lsh_history(char **args);
int lsh_pinfo(char **args);

char *builtin_str[] = {
    "cd", "help", "exit", "echo", "pwd", "history" ,"pinfo"
};

int (*builtin_func[]) (char **) = {
    &lsh_cd, &lsh_help, &lsh_exit, &lsh_echo, &lsh_pwd, &lsh_history, &lsh_pinfo
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

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

//Out of scope
int lsh_help(char **args) {
    int i;
    printf("Naren Akash R J's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for(i = 0; i <lsh_num_builtins(); i++) {
        printf("    %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args) {
    return 0;
}

int lsh_echo(char **args) {
    int i = 0;

    while(args[++i] != NULL) {
        printf("%s ", args[i]);
    }

    printf("\n");
    return 1;

}

int lsh_pwd(char **args) {
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    printf("%s\n", pwd);
}

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

int lsh_bgexecute(char **args) {
    pid_t pid = fork();

    if(pid == -1) {
        perror("Failure: Forking");
        exit(EXIT_FAILURE);
    } else if(pid == 0) {
        //Function not in use as of now.
    }
}

int lsh_execute(char **args) {
    int i;

    if(args[0] == NULL) {
        return 1;
    }

    for(i = 0; i <lsh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i]) (args);
        }
    }

    return lsh_launch(args);
}



/*****************************************
Function for String Search and Replacement 
*****************************************/
char *modify_absolute_path(char *str, char *original, char *replace) {
    static char buffer[1024];
    char *p;

    if(!(p = strstr(str, original))) {
        return str;
    }

    strncpy(buffer, str, p-str);
    buffer[p-str] = '\0';
    sprintf(buffer+(p-str), "%s%s", replace, p+strlen(original));
    return buffer;
}



/******************************************
Basic Loop of a Shell: Read, Parse, Execute
******************************************/
void lsh_loop() {
    char *line, hline[2048];
    char **args;
    int status;
    char home_directory[1024];

    getcwd(home_directory, sizeof(home_directory));

    do {
        char dir_path[1024], *rel_path, *username, hostname[1024];
        background = 0;
        struct hostent *h;
        username = getenv("USER");
        gethostname(hostname, sizeof(hostname));
        h = gethostbyname(hostname);
        getcwd(dir_path, sizeof(dir_path));
        rel_path = modify_absolute_path(dir_path,home_directory,"~");
        printf(COLOR_CYAN "<" COLOR_CYAN "%s" COLOR_CYAN "@" COLOR_CYAN "%s" COLOR_GRAY ":" COLOR_YELLOW "%s" COLOR_CYAN "> ",username,h->h_name, rel_path);
        line = lsh_read_line();   
        strcpy(hline,line);     
        args = lsh_split_line(line);
        add_history(hline);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while(status);
}



/************************************
Basic POSIX Shell in C: Main Function
************************************/
int main(int argc, char **argv) {
    lsh_loop();
    return EXIT_SUCCESS;
}



/*************************************************
 August 20, 2019, Tuesday: Naren Akash R J

 International Institute of Information Technolgy
 Hyderabad, Telengana, INDIA
 ************************************************/