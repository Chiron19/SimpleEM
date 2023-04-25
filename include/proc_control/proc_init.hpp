#ifndef SIMPLEEM_PROC_INIT
#define SIMPLEEM_PROC_INIT

/*
 * Creates procs_num new processes that want to run
 * the program specified in the 'config.h' file, but
 * just before they do, they raise(SIGSTOP). Their
 * process ids are saved in pids array
 */
void fork_stop_run(int procs_num, int* pids, 
				   const char* program_path, 
				   const char* program_config_path);

void child_init(int em_pid, const char* program_path, 
				const char* program_config_path);

#endif // SIMPLEEM_PROC_INIT