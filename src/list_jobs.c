#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_jobs.h"
#include "readcmd.h"

void init_list(job jobs[MAXJOBS]){
    /* Initializes the list of background jobs with null id's. */
    for(int i = 0; i < MAXJOBS; i++){
        jobs[i].job_id = 0;
    }
}

void add_job(job jobs[MAXJOBS], int job_id, int pid, char **cmd){
    /* Adds a job and its attributes to the list of background jobs. */
    for(int i = 0; i < MAXJOBS; i++){
        if(jobs[i].job_id == 0){
            jobs[i].job_id = job_id;
            jobs[i].pid = pid;
            jobs[i].cmd = cpy_cmd(cmd);
            jobs[i].job_status = ACTIVE;
            return;
        }
    }
    printf("Background jobs limit reached.");
    return;
}

void remove_job(job jobs[MAXJOBS], int pid){
    /* Removes a job from list of background jobs by freeing cmd and setting the id to 0. */
    for(int i = 0; i < MAXJOBS; i++){
        if(jobs[i].pid == pid){
            for(int k = 0; jobs[i].cmd[k] != 0; k++){
                free(jobs[i].cmd[k]);
            }
            free(jobs[i].cmd);
            jobs[i].job_id = 0;
            jobs[i].pid = 0;
            return;
        }
    }
}

int pid_is_background(job jobs[MAXJOBS], int pid){
    /* Find a job with a given pid. */
    for(int i = 0; i < MAXJOBS; i++){
        if(jobs[i].pid == pid){
            return 1;
        }
    }
    return 0;
}

int get_pid_with_id(job jobs[MAXJOBS], int job_id){
    /* Find a job's pid with a given id. */
    for(int i = 0; i < MAXJOBS; i++){
        if(jobs[i].job_id == job_id){
            return jobs[i].pid;
        }
    }
    return 0;
}

void change_job_status(job jobs[MAXJOBS], int pid, job_status status){
    /* Change status of a job with a given pid. */
    for(int i = 0; i < MAXJOBS; i++){
        if(jobs[i].pid == pid){
            jobs[i].job_status = status;
            return;
        }
    }
}

void print_job(job *job_node, int nb_jobs, int pid){
    /* Prints the job's status (started or terminated). */
    /* Job has terminated. */
    if(job_node != NULL){
        printf("[%d]+  Done                    ", nb_jobs);
        for(int i = 0; job_node->cmd[i] != 0; i++)
            printf("%s", job_node->cmd[i]);
        printf("\n");
    /* Job has started. */
    } else {
        fflush(stdout);
        printf("shell> ");
        fflush(stdout);
        printf("[%d] %d\n", nb_jobs, pid);
    }
}

void print_jobs(job bg_jobs[MAXJOBS]){
    /* Prints all background jobs. */
    printf("Background jobs: \n");
    for(int i = 0; i < MAXJOBS; i++) {
        if(bg_jobs[i].job_id != 0) {// && bg_jobs[i].job_status == ACTIVE){
            printf("[%d] %d : ", bg_jobs[i].job_id, bg_jobs[i].pid);
            for(int k = 0; bg_jobs[i].cmd[k] != 0; k++)
                printf("%s ", bg_jobs[i].cmd[k]);
            if (bg_jobs[i].job_status == STOPPED)
                printf("stopped");
            printf("\n");
        }
    }
}

char ** cpy_cmd(char **cmd){
    /* Creates a copy of the command with its arguments and return its pointer. */
    int k;
    char **dest = malloc(sizeof(char **));
    /* Copy every word in the command and its arguments. */
    for(k = 0; cmd[k] != 0; k++){
        char *tmp = malloc(sizeof(cmd[k]));
        strcpy(tmp, cmd[k]);
        dest = realloc(dest, (k+2) * sizeof(char **));
        dest[k] = tmp;
    }
    dest[k] = 0;
    return dest;
}
