#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <string>

#include "proc_control/proc_init.hpp"

void fork_stop_run(int procs_num, int* pids, 
				   const char* program_path, 
				   const char* program_config_path) {
    int status;

    for (int i = 0; i < procs_num; ++i) {
		status = fork();

		if (status == -1) {
			printf("[TINYEM][ERROR] Forking error, exiting\n");
			exit(1);
		}
		else if (status == 0) { /* This is a child process */
			child_init(i, program_path, program_config_path);
		}
		else { /* This is a parent process, new_id is set to child's pid */
			pids[i] = status;
		}
	}
}

void child_init(int em_pid, const char* program_path, 
				const char* program_config_path) {
	int status;
	const char *program_argv[] = {
		"dummy", 
		std::to_string(em_pid).c_str(),
		program_config_path,
		NULL
	};

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