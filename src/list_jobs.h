#ifndef LIST_JOBS_H
#define LIST_JOBS_H

#define MAXJOBS 10
typedef enum { ACTIVE, STOPPED } job_status;

typedef struct job{
	int job_id;
	int pid;
	char **cmd;
	job_status job_status;
} job; 

typedef struct job_list {
	int nb_jobs;
	job *jobs;
} job_list;

/* Find a job with a given pid. */
int pid_is_background(job jobs[MAXJOBS], int pid);

/* Find a job's pid with a given id. */
int get_pid_with_id(job jobs[MAXJOBS], int job_id);

/* Creates a new job_list and returns it */
void init_list(job jobs[MAXJOBS]);

/* Adds a new job to list of background jobs */
void add_job(job jobs[MAXJOBS], int job_id, int pid, char **cmd);

/* Remove a job from list of background jobs */
void remove_job(job jobs[MAXJOBS], int pid);

/* Updates job's status given its pid */
void change_job_status(job jobs[MAXJOBS], int pid, job_status status);

/* Prints the job's status (started or terminated). */
void print_job(job *job_node, int nb_jobs, int pid);

/* Prints all background jobs. */
void print_jobs(job bg_jobs[MAXJOBS]);

/* Creates a copy of the command with its arguments and return its pointer. */
char ** cpy_cmd(char **cmd);
#endif