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
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <signal.h>
#include <stdexcept>
#include <sys/stat.h>
#include <thread>
#include <unordered_set>

#include "add_testcase.h"
#include "generate_license.h"
#include "generate_test.h"
#include "read_annotations.h"

const char *tests_dir = "generated_tests/";  /* Directory to collect generated tests. */

/* Generate a test for the given utility. */
void
generatetest::GenerateTest(std::string utility,
			   std::string section,
			   std::string& license)
{
	std::list<utils::OptRelation *> identified_opt_list;  /* List of identified option relations. */
	std::vector<std::string> usage_messages;         /* Vector of usage messages. Used for validating
							  * their consistency across different runs.
							  */
	std::string command;                             /* (Utility-specific) command to be executed. */
	std::string testcase_list;                       /* List of testcases. */
	std::string testcase_buffer;                     /* Buffer for (temporarily) holding testcase data. */
	std::string test_file;                           /* atf-sh test name. */
	std::string util_with_section;                   /* Section number appended to utility. */
	std::ofstream test_fstream;                      /* Output stream for the atf-sh test. */
	std::pair<std::string, int> output;              /* Return value type for `Execute()`. */
	std::unordered_set<std::string> annotation_set;  /* Hashset of utility specific annotations. */
	/* Number of options for which a testcase has been generated. */
	int progress = 0;

	/* Read annotations and populate hash set "annotation_set". */
	annotations::read_annotations(utility, annotation_set);

	util_with_section = utility + '(' + section + ')';

	utils::OptDefinition opt_def;
	identified_opt_list = opt_def.CheckOpts(utility);
	test_file = tests_dir + utility + "_test.sh";

	/* Indicate the start of test generation for current utility. */
	std::cerr << std::setw(18) << util_with_section << " | "
		  << progress << "/" << opt_def.opt_list.size() << "\r";

	/* Add license in the generated test scripts. */
	test_fstream.open(test_file, std::ios::out);
	test_fstream << license;

	/*
	 * If a known option was encountered (i.e. `identified_opt_list` is
	 * populated), produce a testcase to check the validity of the
	 * result of that option. If no known option was encountered,
	 * produce testcases to verify the correct (generated) usage
	 * message when using the supported options incorrectly.
	 */

	/* Add testcases for known options. */
	if (!identified_opt_list.empty()) {
		for (const auto &i : identified_opt_list) {
			command = utility + " -" + i->value + " 2>&1 </dev/null";
			output = utils::Execute(command);
			if (boost::iequals(output.first.substr(0, 6), "usage:")) {
				/* A usage message was produced => our guessed usage is incorrect. */
				addtestcase::UnknownTestcase(i->value, util_with_section, output.first,
								   output.second, testcase_buffer);
			} else {
				addtestcase::KnownTestcase(i->value, util_with_section,
								 "", output.first, test_fstream);
			}
			testcase_list.append("\tatf_add_test_case " + i->value + "_flag\n");
		}
	}

	/* Add testcases for the options whose usage is not yet known. */
	if (!opt_def.opt_list.empty()) {
		/*
		 * For the purpose of adding a "$usage_output" variable,
		 * we choose the option which produces one.
		 * TODO(shivansh) Avoid double executions of an option, i.e. one while
		 * selecting usage message and another while generating testcase.
		 */

		if (opt_def.opt_list.size() == 1) {
			/* Utility supports a single option, check if it produces a usage message. */
			command = utility + " -" + opt_def.opt_list.front() + " 2>&1 </dev/null";
			output = utils::Execute(command);

			if (output.second && !output.first.empty())
				test_fstream << "usage_output=\'" + output.first + "\'\n\n";
		} else {
			/*
			 * Utility supports multiple options. In case the usage message
			 * is consistent for atleast "two" options, we reduce duplication
			 * by assigning a variable "usage_output" in the test script.
			 */
			for (const auto &i : opt_def.opt_list) {
				command = utility + " -" + i + " 2>&1 </dev/null";
				output = utils::Execute(command);

				if (output.second && usage_messages.size() < 3)
					usage_messages.push_back(output.first);
			}

			for (int j = 0; j < usage_messages.size(); j++) {
				if (!(usage_messages.at(j)).compare(usage_messages.at((j+1) % usage_messages.size())) &&
					!output.first.empty()) {
					test_fstream << "usage_output=\'"
						  + output.first.substr(0, 7 + utility.size())
						  + "\'\n\n";
					break;
				}
			}
		}

		/*
		 * Execute the utility with supported options
		 * and add (+ve)/(-ve) testcases accordingly.
		 */
		for (const auto &i : opt_def.opt_list) {
			/* If the option is annotated, ignore it. */
			if (annotation_set.find(i) != annotation_set.end())
				continue;

			command = utility + " -" + i + " 2>&1 </dev/null";
			output = utils::Execute(command);
			std::cerr << std::setw(18) << util_with_section << " | "
				  << ++progress << "/" << opt_def.opt_list.size() << "\r";

			if (output.second) {
				/* Non-zero exit status was encountered. */
				addtestcase::UnknownTestcase(i, util_with_section, output.first,
							     output.second, testcase_buffer);
			} else {
				/*
				 * EXIT_SUCCESS was encountered. Hence,
				 * the guessed usage was correct.
				 */
				addtestcase::KnownTestcase(i, util_with_section, "",
							   output.first, test_fstream);
				testcase_list.append(std::string("\tatf_add_test_case ")
						     + i + "_flag\n");
			}
		}

		std::cerr << std::endl;

		testcase_list.append("\tatf_add_test_case invalid_usage\n");
		test_fstream << std::string("atf_test_case invalid_usage\ninvalid_usage_head()\n")
			  + "{\n\tatf_set \"descr\" \"Verify that an invalid usage "
			  + "with a supported option \" \\\n\t\t\t\"produces a valid error message"
			  + "\"\n}\n\ninvalid_usage_body()\n{";

		test_fstream << testcase_buffer + "\n}\n\n";
	}

	/*
	 * Add a testcase under "no_arguments" for
	 * running the utility without any arguments.
	 */
	if (annotation_set.find("*") == annotation_set.end()) {
		command = utility + " 2>&1 </dev/null";
		output = utils::Execute(command);
		addtestcase::NoArgsTestcase(util_with_section, output, test_fstream);
		testcase_list.append("\tatf_add_test_case no_arguments\n");
	}

	test_fstream << "atf_init_test_cases()\n{\n" + testcase_list + "}\n";
	test_fstream.close();
}

int
main(int argc, char **argv)
{
	std::ifstream groff_list;
	std::list<std::pair<std::string, std::string>> utility_list;
	std::string test_file;  /* atf-sh test name. */
	std::string util_name;  /* Utility name. */
	struct stat sb;
	struct dirent *ent;
	DIR *groff_dir_ptr;
	char answer;          	/* User input to determine overwriting of test files. */
	std::string license;  	/* Customized license generated during runtime. */
	/* Directory for collecting groff scripts for utilities with failed test generation. */
	const char *failed_groff_dir = "failed_groff/";
	const char *groff_dir = "groff/";  /* Directory of groff scripts. */

	/*
	 * For testing (or generating tests for only selected utilities),
	 * the utility_list can be populated above during declaration.
	 */
	if (utility_list.empty()) {
		if ((groff_dir_ptr = opendir(groff_dir))) {
			readdir(groff_dir_ptr); 	/* Skip directory entry for "." */
			readdir(groff_dir_ptr); 	/* Skip directory entry for ".." */
			while ((ent = readdir(groff_dir_ptr))) {
				util_name = ent->d_name;
				utility_list.push_back(std::make_pair<std::string, std::string>
						      (util_name.substr(0, util_name.length() - 2),
						       util_name.substr(util_name.length() - 1, 1)));
			}
			closedir(groff_dir_ptr);
		} else {
			fprintf(stderr, "Could not open the directory: ./groff\nRefer to the "
					"section \"Populating groff scripts\" in README!\n");
			return EXIT_FAILURE;
		}
	}

	/* Check if the directory "tests_dir" exists. */
	if (stat(tests_dir, &sb) || !S_ISDIR(sb.st_mode)) {
		boost::filesystem::path dir(tests_dir);
		if (boost::filesystem::create_directory(dir))
			std::cout << "Directory created: " << tests_dir << std::endl;
		else {
			std::cerr << "Unable to create directory: " << tests_dir << std::endl;
			return EXIT_FAILURE;
		}
	}

	/* Generate a license to be added in the generated scripts. */
	license = generatelicense::GenerateLicense(argc, argv);

	remove(failed_groff_dir);
	boost::filesystem::create_directory(failed_groff_dir);

	/* Generate a tabular-like format. */
	std::cout << std::endl;
	std::cout << std::setw(21) << "Utility | " << "Progress\n";
	std::cout << std::setw(32) << "----------+-----------\n";

	for (const auto &util : utility_list) {
		/* TODO(shivansh) Check before overwriting existing test scripts. */
		test_file = tests_dir + util.first + "_test.sh";
		fflush(stdout); 	/* Useful in debugging. */
		generatetest::GenerateTest(util.first, util.second, license);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return EXIT_SUCCESS;
}
