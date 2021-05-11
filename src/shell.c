 /*
 * Copyright (C) 2002, Simon Nieuviarts
 */

/*    ___       ___       ___       ___       ___   */
/*   /\  \     /\__\     /\  \     /\__\     /\__\  */
/*  /::\  \   /:/__/_   /::\  \   /:/  /    /:/  /  */
/* /\:\:\__\ /::\/\__\ /::\:\__\ /:/__/    /:/__/   */
/* \:\:\/__/ \/\::/  / \:\:\/  / \:\  \    \:\  \   */
/*  \::/  /    /:/  /   \:\/  /   \:\__\    \:\__\  */
/*   \/__/     \/__/     \/__/     \/__/     \/__/  */
/*                                                  */
/*                            TP4 Ieva and Nishith  */

/* PROBLEMS: */
/* 1 - Background precess can finish in the pause() after the foreground call. */
/* 2 - A job in our shell is considered to be a background process therefore bg doesn't make sense. */
/* 3 - Ctrl-Z kills the shell rather then stopping it. */

/* Function declaration. */
void exit_handler(int sig);
void child_handler();
void background_exec(int std_out, char **cmd, char *command);

/* Libraries*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#include "list_jobs.h"

/* Global variables for background jobs. */
job bg_jobs[MAXJOBS];
/* For generating background job id's. */
int nb_bg_commands = 0;


void exit_handler(int sig) {
	/* Ctrl-c and ctrl-z signal handler. */
	printf("\nshell> Caught an exit signal, will go now.\n");
	/* Stop all remaining processes in case of ctrl-z. */
	if(sig == SIGTSTP){
		int pid, status;
		pid = 0;
		while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0)
			kill(pid, SIGSTOP);
		exit(0);
	} else {
		exit(0);
	}
}

void child_handler(){
	/* SIGCHILD signal handler. */
	int pid_chld;
	int status;

	pid_chld = waitpid(-1, &status, WNOHANG|WUNTRACED);
	/* Handle end of a background process. */
	if(pid_is_background(bg_jobs, pid_chld)){
	    /* Check if waitpid has children and if it didn't return an error. */
	    if(pid_chld < 0 && errno != ECHILD){
			perror("Waitpid error ");
			return;
	    /* If child terminated normally. */
	    } else if(WIFEXITED(status) || WIFSTOPPED(status)){
			printf("Background process %d terminated.\n", pid_chld);
			remove_job(bg_jobs, pid_chld);
			fflush(stdout);
			printf("shell> ");
			fflush(stdout);
		} else {
			printf("Background process %d terminated abnormally.\n", pid_chld);
			fflush(stdout);
			printf("shell> ");
			fflush(stdout);
	    }
	/* Handle end of a foreground process. */
	} else {
	    if(!WIFEXITED(status)){
			fflush(stdout);
			printf("shell> ");
			fflush(stdout);
			printf("Foreground process %d terminated abnormally.\n", pid_chld);
	    }
	}
}

void background_exec(int std_out, char **cmd, char *command){
	/* Executes a background command. */
	int pid;
	char *error;
	/* Change output to standard output. */
	dup2(std_out, 1);
	/* Create child process for a background job. */
	pid = Fork();
	/* Child. */
	if (pid == 0){
		if (execvp(command, cmd) == -1) {
			error = command;
			perror(error);
			kill(getpid(), SIGKILL);
		}
	/* Parent. */
	} else {
		nb_bg_commands++;
		add_job(bg_jobs, nb_bg_commands, pid, cmd);
		/* Shows the start of the background processes. */
		print_job(NULL, nb_bg_commands, pid);
	}
}

int main(){
/* The main shell program that interprets and executes the command line. */

	/* Initializes the list of background jobs. */
	init_list(bg_jobs);

	/* Start ctrl-c and ctrl-z signal listener. */
    signal(SIGINT, exit_handler);
	signal(SIGTSTP, exit_handler);

	/* Start SIGCHILD signal listener. */
	signal(SIGCHLD, child_handler);
	
	/* Will run until the program exits.*/
	while (1) {
		struct cmdline *l;
		int i, pid, std_in, std_out, fd_in, fd_out, nb_commands;
		char *command, *error;
		int tube[2];

		fflush(stdout);
		printf("shell> ");
		fflush(stdout);
		l = readcmd();

		/* If input stream closed, normal termination. */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, reads another command. */
			printf("error: %s\n", l->err);
			continue;
		}

		/* Saves initial standard input and output. */
		std_in = dup(0);
		std_out = dup(1);

		/* Initializes current input and output. */
		fd_in = dup(std_in);
		fd_out = dup(std_out);

		/* Replaces standart input with a given file. */
		if (l->in != NULL) {
			if ((fd_in = open(l->in, O_RDONLY)) < 0) {
        			// perror("File opening failed for input redirection.");
					error = l->in;
					perror(error);
			}
		}

		/* Finds total number of commands. */
		int z;
		for (z=0; l->seq[z]!=0; z++);
		nb_commands = z;

		/* Executes the command line. */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			command = cmd[0];

			/* Quit command. */
			if (strcmp(command,"quit") == 0) {
				printf("Bye! See you soon.\n");
				exit(0);
			}

			/* Jobs command. */
			if (strcmp(command,"jobs") == 0) {
				/* Displays all active background jobs. */
				print_jobs(bg_jobs);
				break;
			}

			/* Stop a background job or bring it to foreground. */
			if (strcmp(command,"stop") == 0 || strcmp(command,"fg") == 0) {
				int bg_pid;
				/* Finds a background job's pid. */
				if (cmd[1][0] == '%') {
    					memmove(cmd[1], cmd[1]+1, strlen(cmd[1]));
					bg_pid = get_pid_with_id(bg_jobs, atoi(cmd[1]));
				} else {
					bg_pid = atoi(cmd[1]);
				}
				if (bg_pid == 0 || !pid_is_background(bg_jobs, bg_pid)){
					printf("Error: %s is not a valid id number nor a background pid.\n", cmd[1]);
				/* Kills the background job and updates its status. */
				} else if (strcmp(command,"stop") == 0){
					kill(bg_pid, SIGSTOP);
				/* Brings the background job to foreground. */
				} else {
					/* Remove from background jobs. */
					remove_job(bg_jobs, bg_pid);
					printf("Background process %d braught to foreground.\n", bg_pid);
					/* Waits for the foreground process to finish. */
					pause();
				}
				break;
			}

			/* Makes standart input point to current input. */
			dup2(fd_in, 0);
			close(fd_in);

			/* Handles foreground command pipes. */
			if(!is_bg_job(cmd, l->bg)){
				/* Handles pipes for all except the last command in the command line. */
				if (i != nb_commands-1) {
					/* Creates a pipe. */
					if (pipe(tube) == -1) {
						error = "Pipe failure";
						perror(error);
						exit(1);
					}
					/* Current input becomes tube's output. */
					fd_in = tube[0];
					/* Current output becomes tube's input. */
					fd_out = tube[1];

				/* Last command in command line. */
				} else {
					/* Current output becomes standart output. */
					fd_out = dup(std_out);
					/* Redirects current output to a file. */
					if (l->out != NULL) {
						if ((fd_out = creat(l->out, 0644)) < 0) {
							error = l->out;
							perror(error);
						}
					}
				}
			}


			/* Makes standart output point to current output. */
			dup2(fd_out, 1);
			close(fd_out);

			/* Background command. */
			if(is_bg_job(cmd, l->bg)) {
				/* Handles a background job. */
				background_exec(std_out, cmd, command);

			/* Foreground command. */
			} else {
				pid = Fork();
				if (pid == -1) {
					error = "Unsuccessful fork for ";
					strcat(error, command);
					perror(error);

				/* Child. */
				} else if (pid == 0) {
					/* Executes a foreground command. */
					if (execvp(command, cmd) == -1) {
						perror(command);
						/* Kills the child process if couldn't execute. */
						kill(getpid(), SIGKILL);
					}

				/* Parent. */
				} else {
					/* Waits for the foreground process to finish. */
					pause();
				}
			}
		}
		dup2(std_in, 0);
		close(std_in);
		dup2(std_out, 1);
		close(std_out);
	}
}
