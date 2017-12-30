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

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
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
	std::ofstream makefile_fstream;

	makefile_fstream.open(utildir + "/Makefile", std::ios::out);
	makefile_fstream << "# $FreeBSD$\n\nATF_TESTS_SH+=  "
			  + utility + "_test\n\n"
			  + ".include <bsd.test.mk>\n";
	makefile_fstream.close();
}

/* Generate a test for the given utility. */
void
generatetest::GenerateTest(std::string utility,
			   char section,
			   std::string& license,
			   const char *tests_dir)
{
	std::vector<std::string> usage_messages;
	std::vector<utils::OptRelation *> identified_opts;
	std::string command;            /* (Utility-specific) command to be executed. */
	std::string testcase_list;      /* List of testcases. */
	std::string buffer;             /* Buffer for (temporarily) holding testcase data. */
	std::string testfile;           /* atf-sh test name. */
	std::string util_with_section;  /* Section number appended to utility. */
	std::ofstream test_fstream;     /* Output stream for the atf-sh test. */
	std::pair<std::string, int> output;
	std::unordered_set<std::string> annotation_set;  /* Utility specific annotations. */
	/* Number of options for which a testcase has been generated. */
	int progress = 0;
	bool usage_output = false;  /* Tracks whether '$usage_output' variable is used. */

	/* Read annotations and populate hash set "annotation_set". */
	annotations::read_annotations(utility, annotation_set);
	util_with_section = utility + '(' + section + ')';
	utils::OptDefinition opt_def;
	identified_opts = opt_def.CheckOpts(utility);
	testfile = tests_dir + utility + "_test.sh";

#ifndef DEBUG
	/* Indicate the start of test generation for current utility. */
	if (isatty(fileno(stderr))) {
		std::cerr << std::setw(18) << util_with_section << " | "
			  << progress << "/" << opt_def.opt_list.size() << "\r";
	}
#endif
	/* Add license in the generated test scripts. */
	test_fstream.open(testfile, std::ios::out);
	test_fstream << license;

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
						   "", output.first, test_fstream);
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
			test_fstream << "usage_output=\'" + output.first + "\'\n\n";
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
				test_fstream << "usage_output=\'"
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
						   output.first, test_fstream);
			testcase_list.append(std::string("\tatf_add_test_case ")
					     + i + "_flag\n");
		}
	}
	std::cout << std::endl;  /* Takes care of the last '\r'. */

	if (!opt_def.opt_list.empty()) {
		testcase_list.append("\tatf_add_test_case invalid_usage\n");
		test_fstream << "atf_test_case invalid_usage\ninvalid_usage_head()\n"
			     << "{\n\tatf_set \"descr\" \"Verify that an invalid usage "
			     << "with a supported option \" \\\n\t\t\t\"produces a valid "
			     << "error message\"\n}\n\ninvalid_usage_body()\n{";

		test_fstream << buffer + "\n}\n\n";
	}

	/*
	 * Add a testcase under "no_arguments" for
	 * running the utility without any arguments.
	 */
	if (annotation_set.find("*") == annotation_set.end()) {
		command = utils::GenerateCommand(utility, "");
		output = utils::Execute(command);
		addtestcase::NoArgsTestcase(util_with_section, output,
					    test_fstream, usage_output);
		testcase_list.append("\tatf_add_test_case no_arguments\n");
	}

	test_fstream << "atf_init_test_cases()\n{\n" + testcase_list + "}\n";
	test_fstream.close();
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
	const char *tests_dir = "generated_tests/";
	/*
	 * Instead of generating tests for all the utilities, "batch mode"
	 * allows generation of tests for first "batch_limit" number of
	 * utilities selected from "scripts/utils_list".
	 */
	bool batch_mode = false;
	int batch_limit;  /* Number of tests to be generated in batch mode. */

	/* Handle interrupts. */
	signal(SIGINT, generatetest::IntHandler);

	if (groff::FetchGroffScripts() == -1)
		return EXIT_FAILURE;

	/*
	 * Create a temporary directory where all the side-effects
	 * introduced by utility-specific commands are restricted.
	 */
	boost::filesystem::create_directory(utils::tmpdir);

	std::cout << "\nInstead of generating tests for all the utilities, 'batch mode'\n"
		     "allows generation of tests for first few utilities selected from\n"
		     "'scripts/utils_list', and places them at their correct location\n"
		     "in the src tree, with corresponding makefiles created.\n"
		     "NOTE: You will be prompted for the superuser password when\n"
		     "creating test directory under '/usr/tests/' and when installing\n"
		     "the tests via `sudo make install`.\n"
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

	/* Check if the directory "tests_dir" exists. */
	if (stat(tests_dir, &sb) || !S_ISDIR(sb.st_mode)) {
		boost::filesystem::path dir(tests_dir);
		if (boost::filesystem::create_directory(dir))
			std::cout << "Directory created: " << tests_dir << "\n";
		else {
			std::cerr << "Unable to create directory: " << tests_dir << "\n";
			return EXIT_FAILURE;
		}
	}

	/* Generate a license to be added in the generated scripts. */
	license = generatelicense::GenerateLicense(argc, argv);

#ifndef DEBUG
	/* Generate a tabular-like format. */
	std::cout << std::endl;
	std::cout << std::setw(21) << "Utility | " << "Progress\n";
	std::cout << std::setw(32) << "----------+-----------\n";
#endif

	if (batch_mode) {
		/* Number of hops required to reach root directory. */
		std::string hops = "../../../../";
		std::string command;
		std::string testdir;
		int retval;
		int maxdepth = 32;
		std::unordered_map<std::string, std::string>::iterator it;
		/* Remember pwd so that we can return back. */
		boost::filesystem::path tooldir = boost::filesystem::current_path();

		/*
		 * Discover number of hops required to reach root directory.
		 * To avoid an infinite loop in case directory "usr/" doesn't
		 * exist in the filesystem, we assume that the maximum depth
		 * from root directory at which pwd is located is "maxdepth".
		 */
		chdir(hops.c_str());  /* Move outside FreeBSD src. */
		while (maxdepth--) {
			if (chdir("..") == -1) {
				perror("chdir");
				return EXIT_FAILURE;
			}
			hops += "../";
			if (stat("usr", &sb) == 0 && S_ISDIR(sb.st_mode))
				break;
		}

		/*
		 * Generate tests for first "batch_limit" number of
		 * utilities selected from "scripts/utils_list".
		 */
		it = groff::groff_map.begin();
		while (batch_limit-- && it != groff::groff_map.end()) {
			/* Move back to the tool's directory. */
			boost::filesystem::current_path(tooldir);

			groffpath = groff::groff_map.at(it->first);
			utildir = groffpath.substr
				(0, groffpath.size() - 2 - it->first.size());
			testdir = hops + "usr/tests/" + utildir.substr(9);
			utildir += "tests/";

			DEBUGP("testdir: %s, utildir: %s\n", testdir.c_str(), utildir.c_str());

			/* Populate "tests/" directory. */
			boost::filesystem::remove_all(utildir);
			boost::filesystem::create_directory(utildir);
			generatetest::GenerateMakefile(it->first, utildir);
			generatetest::GenerateTest(it->first, it->second.back(),
						   license, utildir.c_str());
			std::advance(it, 1);

			/* Execute the generated test and note success/failure. */
			if (stat(testdir.c_str(), &sb) || !S_ISDIR(sb.st_mode)) {
				command = "sudo mkdir -p " + testdir;
				if ((retval = system(command.c_str())) == -1) {
					perror("system");
					return EXIT_FAILURE;
				} else if (retval) {
					boost::filesystem::current_path(tooldir);
					boost::filesystem::remove_all(utildir);
					continue;
				}
			}

			/* Install the test. */
			chdir(utildir.c_str());
			if ((retval = system("sudo make install")) == -1) {
				perror("system");
				return EXIT_FAILURE;
			} else if (retval) {
				boost::filesystem::current_path(tooldir);
				boost::filesystem::remove_all(utildir);
				continue;
			}
			boost::filesystem::current_path(tooldir);

			/* Run the test. */
			chdir(testdir.c_str());
			if ((retval = system("kyua test")) == -1) {
				perror("system");
				return EXIT_FAILURE;
			} else if (retval) {
				boost::filesystem::current_path(tooldir);
				boost::filesystem::remove_all(utildir);
				continue;
			}
		}
	} else {
		for (const auto &it : groff::groff_map) {
			generatetest::GenerateTest(it.first, it.second.back(),
						   license, tests_dir);
		}
	}

	/* Cleanup. */
	boost::filesystem::remove_all(utils::tmpdir);
	return EXIT_SUCCESS;
}
