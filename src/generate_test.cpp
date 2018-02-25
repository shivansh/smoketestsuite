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

#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_set>

#include "add_testcase.h"
#include "fetch_groff.h"
#include "generate_license.h"
#include "generate_test.h"
#include "logging.h"
#include "read_annotations.h"

void
generatetest::IntHandler(int dummmy)
{
	std::cerr << "\nExiting...\n";
	/* Remove the temporary directory. */
	boost::filesystem::remove_all(utils::tmpdir);
	exit(EXIT_FAILURE);
}

/* [Batch mode] Generate a makefile for the test of given utility. */
void
generatetest::GenerateMakefile(std::string utility, std::string utildir)
{
	std::ofstream file;

	file.open(utildir + "/Makefile", std::ios::out);
	file << "# $FreeBSD$\n\nATF_TESTS_SH+=  "
		  + utility + "_test\n\n"
		  + ".include <bsd.test.mk>\n";
	file.close();
}

/* Generate a test for the given utility. */
void
generatetest::GenerateTest(std::string utility,
			   char section,
			   std::string& license,
			   const char *testsdir)
{
	std::vector<std::string> usage_messages;
	std::vector<utils::OptRelation *> identified_opts;
	std::string command;
	std::string testcase_list;
	std::string buffer;
	std::string testfile;
	std::string util_with_section;
	std::ofstream file;
	std::pair<std::string, int> output;
	std::unordered_set<std::string> annotation_set;
	/* Number of options for which a testcase has been generated. */
	int progress = 0;
	bool usage_output = false;  /* Tracks whether '$usage_output' variable is used. */

	/* Read annotations and populate hash set "annotation_set". */
	annotations::read_annotations(utility, annotation_set);
	util_with_section = utility + '(' + section + ')';
	utils::OptDefinition opt_def;
	identified_opts = opt_def.CheckOpts(utility);
	testfile = testsdir + utility + "_test.sh";

#ifndef DEBUG
	/* Indicate the start of test generation for current utility. */
	if (isatty(fileno(stderr))) {
		std::cerr << std::setw(18) << util_with_section << " | "
			  << progress << "/" << opt_def.opt_list.size() << "\r";
	}
#endif
	/* Add license in the generated test scripts. */
	file.open(testfile, std::ios::out);
	file << license;

	/*
	 * If a known option was encountered (i.e. `identified_opts` is
	 * populated), produce a testcase to check the validity of the
	 * result of that option. If no known option was encountered,
	 * produce testcases to verify the correct (generated) usage
	 * message when using the supported options incorrectly.
	 */
	for (const auto &i : identified_opts) {
		command = utils::GenerateCommand(utility, i->value);
		output = utils::Execute(command);
		if (boost::iequals(output.first.substr(0, 6), "usage:")) {
			/* Our guessed usage is incorrect as usage message is produced. */
			addtestcase::UnknownTestcase(i->value, util_with_section,
						     output, buffer, usage_output);
		} else {
			addtestcase::KnownTestcase(i->value, util_with_section,
						   "", output.first, file);
		}
		testcase_list.append("\tatf_add_test_case " + i->value + "_flag\n");
	}

	/* Add testcases for the options whose usage is not yet known.
	 * For the purpose of adding a "$usage_output" variable,
	 * we choose the option which produces one.
	 * TODO Avoid double executions of an option, i.e. one while
	 * selecting usage message and another while generating testcase.
	 */
	if (opt_def.opt_list.size() == 1) {
		/* Check if the single option produces a usage message. */
		command = utils::GenerateCommand(utility, opt_def.opt_list.front());
		output = utils::Execute(command);
		if (output.second && !output.first.empty()) {
			usage_output = true;
			file << "usage_output=\'" + output.first + "\'\n\n";
		}
	} else if (opt_def.opt_list.size() > 1) {
		/*
		 * Utility supports multiple options. In case the usage message
		 * is consistent for atleast "two" options, we reduce duplication
		 * by assigning a variable "usage_output" in the test script.
		 */
		for (const auto &i : opt_def.opt_list) {
			command = utils::GenerateCommand(utility, i);
			output = utils::Execute(command);
			if (output.second && usage_messages.size() < 3)
				usage_messages.push_back(output.first);
		}

		for (int j = 0; j < usage_messages.size(); j++) {
			if (!usage_messages[j].compare
					(usage_messages[(j+1) % usage_messages.size()])) {
				usage_output = true;
				file << "usage_output=\'"
					      + output.first.substr(0, 7 + utility.size())
					      + "\'\n\n";
				break;
			}
		}
	}

	/*
	 * Execute the utility with supported options, while
	 * adding positive and negative testcases accordingly.
	 */
	for (const auto &i : opt_def.opt_list) {
		/* Ignore the option if it is annotated. */
		if (annotation_set.find(i) != annotation_set.end())
			continue;

		command = utils::GenerateCommand(utility, i);
		output = utils::Execute(command);
#ifndef DEBUG
		if (isatty(fileno(stderr))) {
			std::cerr << std::setw(18) << util_with_section
				  << " | " << ++progress << "/"
				  << opt_def.opt_list.size() << "\r";
		}
#endif
		if (output.second) {
			addtestcase::UnknownTestcase(i, util_with_section, output,
						     buffer, usage_output);
		} else {
			/* Guessed usage is correct as EXIT_SUCCESS is encountered */
			addtestcase::KnownTestcase(i, util_with_section, "",
						   output.first, file);
			testcase_list.append(std::string("\tatf_add_test_case ")
					     + i + "_flag\n");
		}
	}
	std::cout << std::endl;  /* Takes care of the last '\r'. */

	if (!opt_def.opt_list.empty()) {
		testcase_list.append("\tatf_add_test_case invalid_usage\n");
		file << "atf_test_case invalid_usage\ninvalid_usage_head()\n"
			"{\n\tatf_set \"descr\" \"Verify that an invalid usage "
			"with a supported option \" \\\n\t\t\t\"produces a valid "
			"error message\"\n}\n\ninvalid_usage_body()\n{"
		      + buffer + "\n}\n\n";
	}

	/*
	 * Add a testcase under "no_arguments" for
	 * running the utility without any arguments.
	 */
	if (annotation_set.find("*") == annotation_set.end()) {
		command = utils::GenerateCommand(utility, "");
		output = utils::Execute(command);
		addtestcase::NoArgsTestcase(util_with_section, output,
					    file, usage_output);
		testcase_list.append("\tatf_add_test_case no_arguments\n");
	}

	file << "atf_init_test_cases()\n{\n" + testcase_list + "}\n";
	file.close();
}

int
main(int argc, char **argv)
{
	std::ifstream groff_list;
	struct stat sb;
	struct dirent *ent;
	char answer;
	std::string license;
	std::string utildir;  /* Path to utility in src tree. */
	std::string groffpath;
	const char *testsdir = "generated_tests/";
	/*
	 * Instead of generating tests for all the utilities, "batch mode"
	 * allows generation of tests for first "batch_limit" number of
	 * utilities selected from "scripts/utils_list".
	 */
	bool batch_mode = false;
	int batch_limit;  /* Number of tests to be generated in batch mode. */

	/* Handle interrupts. */
	signal(SIGINT, generatetest::IntHandler);

	if (groff::FetchGroffScripts() == EXIT_FAILURE)
		return EXIT_FAILURE;

	/*
	 * Create a temporary directory where all the side-effects
	 * introduced by utility-specific commands are restricted.
	 */
	boost::filesystem::create_directory(utils::tmpdir);

	std::cout << "\nInstead of generating tests for all the utilities, 'batch mode'\n"
		     "allows generation of tests for first N utilities selected from\n"
		     "'scripts/utils_list', and places them at their correct location\n"
		     "in the src tree, with corresponding makefiles created.\n"
		     "Run in 'batch mode' ? [y/N] ";
	std::cin.get(answer);

	switch(answer) {
	case 'y':
	case 'Y':
		batch_mode = true;
		std::cout << "Number of utilities to select for test generation: ";
		std::cin >> batch_limit;

		if (batch_limit <= 0) {
			std::cerr << "Invalid input. Exiting...\n";
			return EXIT_FAILURE;
		}
		break;
	case '\n':
	default:
		break;
	}

	/* Check if the directory "testsdir" exists. */
	if (stat(testsdir, &sb) || !S_ISDIR(sb.st_mode)) {
		boost::filesystem::path dir(testsdir);
		if (boost::filesystem::create_directory(dir))
			std::cout << "Directory created: " << testsdir << "\n";
		else {
			std::cerr << "Unable to create directory: " << testsdir << "\n";
			return EXIT_FAILURE;
		}
	}

	/* Generate a license to be added in the generated scripts. */
	license = generatelicense::GenerateLicense(argc, argv);

#ifndef DEBUG
	/* Generate a tabular-like format. */
	std::cout << std::endl;
	std::cout << std::setw(30) << "Utility | Progress\n";
	std::cout << std::setw(32) << "----------+-----------\n";
#endif

	if (batch_mode) {
		std::string command;
		std::unordered_map<std::string, std::string>::iterator it;

		/*
		 * Generate tests for first "batch_limit" number of
		 * utilities selected from "scripts/utils_list".
		 */
		it = groff::groff_map.begin();
		while (batch_limit-- && it != groff::groff_map.end()) {
			groffpath = groff::groff_map.at(it->first);
			utildir = groffpath.substr
				(0, groffpath.size() - 2 - it->first.size());
			utildir += "tests/";

			/* Populate "tests/" directory. */
			boost::filesystem::remove_all(utildir);
			boost::filesystem::create_directory(utildir);
			generatetest::GenerateMakefile(it->first, utildir);
			generatetest::GenerateTest(it->first, it->second.back(),
						   license, utildir.c_str());
			boost::filesystem::copy_file(utildir + it->first + "_test.sh",
				testsdir + it->first + "_test.sh",
				boost::filesystem::copy_option::overwrite_if_exists);
			std::advance(it, 1);
		}
	} else {
		for (const auto &it : groff::groff_map) {
			generatetest::GenerateTest(it.first, it.second.back(),
						   license, testsdir);
		}
	}

	/* Cleanup. */
	boost::filesystem::remove_all(utils::tmpdir);
	return EXIT_SUCCESS;
}
