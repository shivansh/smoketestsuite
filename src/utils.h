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

#include <list>
#include <unordered_map>

namespace utils {
	/*
	 * Option relation which maps option names to
	 * a unique identifier in their description.
	 */
	typedef struct OptRelation {
		char type;            /* Option type: (s)short/(l)long. */
		std::string value;    /* Name of the option. */
		std::string keyword;  /* The keyword which should be looked up in usage
				       * message (if) produced when using this option.
				       */
	} opt_relation;

	std::pair<std::string, int> Execute(const char*);

	class OptDefinition {
		public:
			std::list<std::string> opt_list;    	/* list of all the accepted options with unknown usage. */
			std::unordered_map<std::string, opt_relation> opt_map;   /* Map "option value" to "option definition". */
			std::unordered_map<std::string, opt_relation>::iterator opt_map_iter;

			void InsertOpts();
			std::list<opt_relation *> CheckOpts(std::string);
	};
}
