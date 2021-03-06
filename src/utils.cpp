/*-
 * Copyright 2017-2018 Shivansh Rai
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "utils.h"
#include "fetch_groff.h"
#include "logging.h"

#define READ 0   /* Pipe descriptor: read end. */
#define WRITE 1	 /* Pipe descriptor: write end. */
/*
 * Buffer size (used for buffering output generated
 * after executing the utility-specific command).
 */
#define BUFSIZE 128
#define TIMEOUT 1  /* Threshold (seconds) for a function call to return. */

const char *utils::tmpdir = "tmpdir";

/*
 * Inserts a list of user-defined option definitions into a hashmap. These
 * specific option definitions are the ones which can be easily tested.
 */
void
utils::OptDefinition::InsertOpts()
{
	/* Option definitions. */
	OptRelation h_def;  /* '-h' */
	h_def.type    = 's';
	h_def.value   = "h";
	h_def.keyword = "help";

	OptRelation v_def;  /* '-v' */
	v_def.type    = 's';
	v_def.value   = "v";
	v_def.keyword = "version";

	/* "opt_map" contains all the options which can be easily tested. */
	opt_map.insert(std::make_pair<std::string, OptRelation>
		      ("h", (OptRelation)h_def));
	opt_map.insert(std::make_pair<std::string, OptRelation>
		      ("v", (OptRelation)v_def));
};

/*
 * Finds the supported options present in the hashmap generated by InsertOpts()
 * for the utility under test, and returns them in a form of list of option
 * relations.
 */
std::vector<utils::OptRelation *>
utils::OptDefinition::CheckOpts(std::string utility)
{
	std::string opt_id = ".It Fl";  /* Option identifier in man page. */
	std::string line;               /* An individual line in a man-page. */
	std::string opt_name;           /* Name of the option. */
	std::string opt_desc;           /* Option description extracted from man-page. */
	std::string opt_string;         /* Identified option names. */
	int opt_pos;                    /* Starting index of the (identified) option. */
	int space_index;                /* First occurrence of space in option definition. */
	std::vector<OptRelation *> identified_opts;
	std::vector<std::string> supported_sections = { "1", "8" };

	InsertOpts();  /* Generate the hashmap "opt_map". */
	std::ifstream infile(groff::groff_map[utility]);

	/*
	 * Search for all the options accepted by the utility and collect those
	 * present in "opt_map".
	 */
	while (std::getline(infile, line)) {
		if ((opt_pos = line.find(opt_id)) != std::string::npos) {
			/* Locate the position of option name. */
			opt_pos += opt_id.size() + 1;
			if (opt_pos > line.size()) {
				/*
				 * This condition will trigger when a utility
				 * supports an empty argument, e.g. tset(issue #9)
				 */
				continue;
			}

			/*
			 * Check for long options ; While here, also sanitize
			 * multi-word option definitions in a man page to properly
			 * extract short options from option definitions such as:
			 * 	.It Fl r Ar seconds (taken from date(1)).
			 */
			if ((space_index = line.find(" ", opt_pos + 1, 1))
					!= std::string::npos)
				opt_name = line.substr(opt_pos, space_index - opt_pos);
			else
				opt_name = line.substr(opt_pos);

			/*
			 * Check if the identified option matches the identifier.
			 * "opt_list.back()" is the previously checked option, the
			 * description of which is now stored in "opt_desc".
			 */
			if (!opt_list.empty() &&
			   (opt_map_iter = opt_map.find(opt_list.back()))
					!= opt_map.end() &&
			    opt_desc.find((opt_map_iter->second).keyword) != std::string::npos) {
				identified_opts.push_back(&(opt_map_iter->second));
				/* Remove options with a known usage. */
				opt_list.pop_back();
			}
			opt_list.push_back(opt_name);
			opt_desc.clear();
		} else {
			/*
			 * Collect the option description until next valid
			 * option definition is encountered.
			 */
			opt_desc.append(line);
		}
	}

	return identified_opts;
}

/* Generates command for execution. */
std::string
utils::GenerateCommand(std::string utility, std::string opt)
{
	std::string command = utility;

	if (!opt.empty())
		command += " -" + opt;
	command += " 2>&1 </dev/null";

	return command;
}

/*
 * When pclose() is called on the stream returned by popen(), it waits
 * indefinitely for the created shell process to terminate in cases where the
 * shell command waits for the user input via a blocking read (e.g. passwd(1)).
 * Hence, we define a custom function which alongside returning the read-write
 * file descriptors, also returns the pid of the newly created (child) shell
 * process. This pid can be later used for signalling the child.
 */
utils::PipeDescriptor*
utils::POpen(const char *command)
{
	int pdes[2];
	char *argv[4];
	pid_t child_pid;
	PipeDescriptor *pipe_descr = (PipeDescriptor *)malloc(sizeof(PipeDescriptor));

	/*
	 * Create a pipe with ~
	 *   - pdes[READ]: read end
	 *   - pdes[WRITE]: write end
	 */
	if (pipe2(pdes, O_CLOEXEC) < 0)
		return NULL;
	pipe_descr->readfd = pdes[READ];
	pipe_descr->writefd = pdes[WRITE];

	/* Type-cast to avoid compiler warnings [-Wwrite-strings]. */
	argv[0] = (char *)"sh";
	argv[1] = (char *)"-c";
	argv[2] = (char *)command;
	argv[3] = NULL;

	switch (child_pid = vfork()) {
	case -1: 		/* Error. */
		free(pipe_descr);
		close(pdes[READ]);
		close(pdes[WRITE]);
		return NULL;
	case 0: 		/* Child. */
		if (pdes[WRITE] != STDOUT_FILENO)
			dup2(pdes[WRITE], STDOUT_FILENO);
		else
			fcntl(pdes[WRITE], F_SETFD, 0);
		if (pdes[READ] != STDIN_FILENO)
			dup2(pdes[READ], STDIN_FILENO);
		else
			fcntl(pdes[READ], F_SETFD, 0);
		/*
		 * For current usecase, it might so happen that the child gets
		 * stuck on a blocking read (e.g. passwd(1)) waiting for user
		 * input. In that case the child will be killed via a signal.
		 * To avoid any effect on the parent's execution, we place the
		 * child in a separate process group with pgid set as
		 * "child_pid".
		 */
		setpgid(child_pid, child_pid);
		execve("/bin/sh", argv, NULL);
		exit(127);
	}

	pipe_descr->pid = child_pid;
	return pipe_descr;
}

/*
 * Executes the command passed as argument in a shell and returns its output
 * and exit status.
 */
std::pair<std::string, int>
utils::Execute(std::string command)
{
	int result;
	int exitstatus;
	std::array<char, BUFSIZE> buffer;
	std::string usage_output;
	struct timeval tv;
	fd_set readfds;
	FILE *pipe;
	PipeDescriptor *pipe_descr;
	pid_t pid;
	int pstat;

	/* Execute "command" inside "tmpdir". */
	chdir(tmpdir);
	pipe_descr = utils::POpen(command.c_str());
	chdir("..");
	if (pipe_descr == NULL) {
		logging::LogPerror("utils::POpen()");
		exit(EXIT_FAILURE);
	}

	/* Close the unrequired file-descriptor. */
	close(pipe_descr->writefd);
	pipe = fdopen(pipe_descr->readfd, "r");
	free(pipe_descr);
	if (pipe == NULL) {
		close(pipe_descr->readfd);
		logging::LogPerror("fdopen()");
		exit(EXIT_FAILURE);
	}

	/* Set a timeout for shell process to complete its execution. */
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(fileno(pipe), &readfds);
	result = select(fileno(pipe) + 1, &readfds, NULL, NULL, &tv);

	if (result > 0) {
		while (!feof(pipe) && !ferror(pipe)) {
			if (fgets(buffer.data(), BUFSIZE, pipe) != NULL)
				usage_output += buffer.data();
		}
	} else if (result == -1) {
		logging::LogPerror("select()");
		if (kill(pipe_descr->pid, SIGTERM) < 0)
			logging::LogPerror("kill()");
	} else if (result == 0) {
		/*
		 * We gave a relaxed value of TIMEOUT seconds for the shell
		 * process to complete it's execution. If at this point it is
		 * still alive, it (most probably) is stuck on a blocking read
		 * waiting for the user input. Since a few of the utilities
		 * performing such blocking reads don't respond to SIGINT (e.g.
		 * pax(1)), we terminate the shell process via SIGTERM.
		 */
		if (kill(pipe_descr->pid, SIGTERM) < 0)
			logging::LogPerror("kill()");
	}

	/* Retrieve exit status of the shell process. */
	do {
		pid = wait4(pipe_descr->pid, &pstat, 0, (struct rusage *)0);
	} while (pid == -1 && errno == EINTR);

	fclose(pipe);
	exitstatus = (pid == -1) ? -1 : WEXITSTATUS(pstat);
	DEBUGP("Command: %s, exit status: %d\n", command.c_str(), exitstatus);

	return std::make_pair<std::string, int>
		((std::string)usage_output, (int)exitstatus);
}
