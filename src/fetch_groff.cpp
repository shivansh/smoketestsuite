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
std::unordered_map<std::string, std::string> groff::utilpath_map;

void
groff::Copy(std::string src, std::string dst)
{
	std::ifstream src_stream(src);
	std::ofstream dst_stream(dst);

	dst_stream << src_stream.rdbuf();
}

/*
 * Traverses the FreeBSD src tree looking for groff scripts for section
 * 1 and section 8 utilities and copies them to the directory "groff/".
 * TODO Use the paths of scripts instead of copying.
 */
int
groff::FetchGroffScripts()
{
	std::string utils_list = "scripts/utils_list";
	std::string groff_src = "groff/";
	std::string freebsd_src = "../../../";
	std::string util_dir;
	std::string pathname;
	std::ifstream utils_fstream;
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
	utils_fstream.open(utils_list);

	/* Remove the stale directory "groff/" and create an empty one. */
	boost::filesystem::remove_all(groff_src);
	boost::filesystem::create_directory(groff_src);

	while (getline(utils_fstream, util_dir)) {
		pathname = freebsd_src + util_dir + "/tests";
		/*
		 * Copy the groff script only if the utility does not
		 * already have tests, i.e. the "tests" directory is absent.
		 */
		if (!(stat(pathname.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))) {
			pathname = freebsd_src + util_dir + "/";
			utilpath_map[util_dir.substr
				(util_dir.find_last_of("/") + 1)] = pathname;
			if ((dir = opendir(pathname.c_str())) != NULL) {
				ent = readdir(dir);  /* Skip directory entry for "." */
				ent = readdir(dir);  /* Skip directory entry for ".." */
				while ((ent = readdir(dir)) != NULL) {
					if (std::regex_match(ent->d_name, section)) {
						boost::thread copy_thread(Copy,
								pathname + ent->d_name,
								groff_src + ent->d_name);
						copy_thread.join();
					}
				}
				closedir(dir);
			} else {
				logging::LogPerror("opendir()");
			}
		}
	}

	/* Remove non-executable utilities. */
	boost::filesystem::remove(groff_src + "elfcopy.1");
	utils_fstream.close();
	std::cout << "Successfully updated 'groff/'\n";

	return EXIT_SUCCESS;
}
