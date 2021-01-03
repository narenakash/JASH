#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<netdb.h>
#include<errno.h>
#include<signal.h>
#include<termios.h>
#include<glob.h>
#include<sys/utsname.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include "headers.h"

#define COLOR_NONE "\033[m"
#define COLOR_RED "\033[1;37;41m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_GREEN "\033[0;32;32m"
#define COLOR_GRAY "\033[1;30m"

#define HISTORY_MAX_SIZE 20

char *history[HISTORY_MAX_SIZE];
unsigned int hCount = 0;

typedef struct {
    int id;
    int state;
    char name[100];
}process;

process parray[100];
int pcount;

typedef struct {
		char* p_pid;
		char p_name[1024];
		char p_stime[128];
		char p_status[128];
}ongoing_processes;

struct ongoing_processes cur_jobs[2048];
int jobs_sz;

void sigintHandler (int sig_num)
{
	signal(SIGINT, sigintHandler);
	fflush(stdout);
}

void sigtstpHandler(int sig_num) 
{ 
	signal(SIGTSTP, sigtstpHandler);
}

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



void lsh_loop() {
    char *line, hline[2048];
    char **commands, **args;
    int status;
    char home_directory[1024];

    getcwd(home_directory, sizeof(home_directory));

    do {
        
        char dir_path[1024], *rel_path, *username, hostname[1024];
        
        struct hostent *h;
        
        username = getenv("USER");
        gethostname(hostname, sizeof(hostname));
        h = gethostbyname(hostname);
        getcwd(dir_path, sizeof(dir_path));

        process temp[101];
        int t_count = 0;

        for(int i = 1; i <= pcount; i++) {
            int a = kill(parray[i].id,0);
            if(!(a == -1 && errno == ESRCH)) {
                temp[++t_count] = parray[i];
            } 

        }

        pcount = t_count;
        int i = 1;
        while(i <= pcount) {
            parray[i] = temp[i];
            ++i;
        }
        
        rel_path = modify_absolute_path(dir_path,home_directory,"~");
        printf(COLOR_CYAN "<" COLOR_CYAN "%s" COLOR_CYAN "@" COLOR_CYAN "%s" COLOR_GRAY ":" COLOR_YELLOW "%s" COLOR_CYAN "> ",username,h->h_name, rel_path);
        
        line = lsh_read_line();   
        strcpy(hline,line);     
        commands = lsh_split_line(line,';');
        add_history(hline);

        i = 0;

        while(commands[i] != NULL) {
            char **pcommands = lsh_split_line(commands[i], '|');
            int pflag = 0, fd = 0, parr[2];

            if(pcommands[1] != NULL) {
                pflag = 1;
            }

            int type = 0;

            if(pflag) {

                int j = 0;

                while(pcommands[j] != NULL) {
                    char *aargs[100];
                    int count = lsh_parse_line(pcommands[j],aargs, &type);
                    pipe(parr); 
                    pid_t np = fork();

                    if(!np) {
                        dup2(fd, 0);
                        if(pcommands[j+1] != NULL) {
                            dup2(parr[1], 1);
                        }
                        close(parr[1]);
                        lsh_execute(aargs, type, count);
                        exit(2);

                    } else {
                        wait(NULL);
                        close(parr[1]);
                        fd = parr[0];
                        i++;
                    }

                    j++; 
                }
            } else {
                char *aargs[100];
                int count = lsh_parse_line(commands[i],aargs, &type);
                lsh_execute(aargs, type, count); 
            }

            i++;
        }

        free(line);
        free(args);
    } while(1);
}



int main(int argc, char **argv) {

    signal(SIGTSTP, sigtstpHandler);
    signal(SIGINT, sigintHandler);
	

    lsh_loop();
    return EXIT_SUCCESS;
}
