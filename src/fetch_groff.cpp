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

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

#include "fetch_groff.h"
#include "logging.h"

/* Map of utility name and its location in src tree. */
std::unordered_map<std::string, std::string> groff::groff_map;

/*
 * Traverses the FreeBSD src tree looking for groff scripts for section
 * 1 and section 8 utilities and stores their location in a hashmap.
 */
int
groff::FetchGroffScripts()
{
	std::string utils_list = "scripts/utils_list";
	std::string src = "../../../";  /* FreeBSD src. */
	std::string utildir;
	std::string utilname;
	std::string path;
	std::ifstream util_fstream;
	std::regex section ("(.*).(?:1|8)");
	struct stat sb;
	struct dirent *ent;
	DIR *dir;

	/* Check if the file "scripts/utils_list" exists. */
	if (stat(utils_list.c_str(), &sb) != 0) {
		std::cerr << "scripts/utils_list does not exists.\n"
			     "Run 'make fetch_utils' first.\n";
		return EXIT_FAILURE;
	}
	util_fstream.open(utils_list);

	while (getline(util_fstream, utildir)) {
		path = src + utildir + "/tests";
		/*
		 * Copy the groff script only if the utility does not
		 * already have tests, i.e. the "tests" directory is absent.
		 */
		if (stat(path.c_str(), &sb) || !S_ISDIR(sb.st_mode)) {
			path = src + utildir + "/";
			utilname = utildir.substr(utildir.find_last_of("/") + 1);
			if ((dir = opendir(path.c_str())) != NULL) {
				ent = readdir(dir);  /* Skip directory entry for "." */
				ent = readdir(dir);  /* Skip directory entry for ".." */
				while ((ent = readdir(dir)) != NULL) {
					if (std::regex_match(ent->d_name, section)) {
						groff_map[utilname] = path + ent->d_name;
					}
				}
				closedir(dir);
			} else {
				logging::LogPerror("opendir()");
			}
		}
	}
	util_fstream.close();

	return EXIT_SUCCESS;
}
