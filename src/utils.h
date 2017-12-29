/*-
 * Copyright 2017 Shivansh Rai
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

#include <unordered_map>
#include <vector>

namespace utils {
	/*
	 * Option relation which maps option names to
	 * a unique identifier in their description.
	 */
	struct OptRelation {
		char type;            /* Option type: (s)short/(l)long. */
		std::string value;    /* Name of the option. */
		/* The keyword which should be looked up in usage
		 * message (if) produced when using this options.
		 */
		std::string keyword;
	};

	/*
	 * Read/Write file descriptors for a pipe.
	 */
	struct PipeDescriptor {
		int readfd;
		int writefd;
		pid_t pid;  /* PID of the forked shell process. */
	};

	/*
	 * Temporary directory inside which the utility-specific
	 * commands will be executed, and all the side effects
	 * (core dumps, executables etc.) that are created will
	 * be restricted in this directory.
	 */
	extern const char *tmpdir;

	std::string GenerateCommand(std::string, std::string);
	std::pair<std::string, int> Execute(std::string);
	PipeDescriptor* POpen(const char*);

	class OptDefinition {
	public:
		/* List of all the accepted options with unknown usage. */
		std::vector<std::string> opt_list;
		/* Map "option value" to "option definition". */
		std::unordered_map<std::string, OptRelation> opt_map;
		std::unordered_map<std::string, OptRelation>::iterator opt_map_iter;

		void InsertOpts();
		std::vector<OptRelation *> CheckOpts(std::string);
	};
}
