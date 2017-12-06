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

#include <fstream>
#include <iostream>
#include <string>

#include "read_annotations.h"

/*
 * Read the annotation files and skip
 * generation of the respective tests.
 */
void
annotations::read_annotations(std::string utility,
			      std::unordered_set<std::string>& annotation_set)
{
	std::string line;
	std::ifstream annotation_fstream;
	annotation_fstream.open("annotation_set/" + utility + "_test.annot");

	while (getline(annotation_fstream, line)) {
		/* Add a unique identifier for no_arguments testcase */
		if (!line.compare(0, 12, "no_arguments"))
			annotation_set.insert("*");

		/*
		 * Add flag value for supported argument testcases
		 * Doing so we ignore the "invalid_usage" testcase
		 * as it is guaranteed to always succeed.
		 */
		else if (!line.compare(2, 4, "flag"))
			annotation_set.insert(line.substr(0, 1));
	}

	annotation_fstream.close();
}
