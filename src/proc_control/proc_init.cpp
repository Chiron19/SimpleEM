#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include "proc_control/proc_init.hpp"

void fork_stop_run(int procs_num, int* pids, 
				   const char* program_path, 
				   const char* program_argv[]) {
    int status;

    for (int i = 0; i < procs_num; ++i) {
		status = fork();

		if (status == -1) {
			printf("[TINYEM][ERROR] Forking error, exiting\n");
			exit(1);
		}
		else if (status == 0) {
			/* This is a child process */
			
            if ((status = raise(SIGSTOP)) != 0) {
                printf("[TINYEM] Process %d couldnt SIGSTOP itself\n", getpid());
                return;
            }
            if ((status = execve(program_path, (char * const*)program_argv, NULL)) == -1) {
                printf("[TINYEM] Process %d couldnt execute program\n", getpid());
                return;
            }

            printf("[TINYEM] Process %d finished all the work\n", getpid());
			return;
		}
		else {
			/* This is a parent process, new_id is set to child's pid */
			pids[i] = status;
		}
	}
}

void kill_all(int procs_num, int* pids) {
	for (int i = 0; i < procs_num; ++i) {
		kill(pids[i], SIGCONT);
		kill(pids[i], SIGINT);
	}
}