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

#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unordered_set>

#include "add_testcase.h"
#include "generate_license.h"
#include "generate_test.h"
#include "read_annotations.h"

#define TIMEOUT 2       /* threshold (seconds) for a function call to return. */

/*
 * Executes the passed argument "cmd" in a shell
 * and returns its output and the exit status.
 */
std::pair<std::string, int>
generatetest::Exec(const char *cmd)
{
	const int bufsize = 128;
	std::array<char, bufsize> buffer;
	std::string usage_output;
	FILE *pipe = popen(cmd, "r");

	if (!pipe)
		throw std::runtime_error("popen() failed!");

	try {
		while (!feof(pipe))
			if (std::fgets(buffer.data(), bufsize, pipe) != NULL)
				usage_output += buffer.data();
	}
	catch(...) {
		pclose(pipe);
		throw "Unable to execute the command: " + std::string(cmd);
	}

	return std::make_pair<std::string, int>
		((std::string)usage_output, WEXITSTATUS(pclose(pipe)));
}

/* Generate a test for the given utility. */
void
generatetest::GenerateTest(std::string utility,
			     std::string section,
			     std::string& license)
{
	std::list<Utils::OptRelation *> identified_opt_list;  /* List of identified option relations. */
	std::vector<std::string> usage_messages;         /* Vector of usage messages. Used for validating
							  * their consistency across different runs.
							  */
	std::string command;                             /* (Utility-specific) command to be executed. */
	std::string testcase_list;                       /* List of testcases. */
	std::string testcase_buffer;                     /* Buffer for (temporarily) holding testcase data. */
	std::string test_file;                           /* atf-sh test name. */
	std::string util_with_section;                   /* Section number appended to utility. */
	std::ofstream test_fstream;                      /* Output stream for the atf-sh test. */
	std::pair<std::string, int> output;              /* Return value type for `Exec()`. */
	std::unordered_set<std::string> annotation_set;  /* Hashset of utility specific annotations. */

	/* Read annotations and populate hash set "annotation_set". */
	annotations::read_annotations(utility, annotation_set);

	util_with_section = utility + '(' + section + ')';

	Utils::OptDefinition opt_def;
	identified_opt_list = opt_def.CheckOpts(utility);
	test_file = "generated_tests/" + utility + "_test.sh";

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
			output = generatetest::Exec(command.c_str());
			if (boost::iequals(output.first.substr(0, 6), "usage:")) {
				/* A usage message was produced => our guessed usage is incorrect. */
				addtestcase::UnknownTestcase(i->value, util_with_section, output.first,
								   output.second, testcase_buffer);
			} else {
				addtestcase::KnownTestcase(i->value, util_with_section,
								 NULL, output.first, test_fstream);
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
			output = generatetest::Exec(command.c_str());

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
				output = generatetest::Exec(command.c_str());

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
			output = generatetest::Exec(command.c_str());

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

		testcase_list.append("\tatf_add_test_case invalid_usage\n");
		test_fstream << std::string("atf_test_case invalid_usage\ninvalid_usage_head()\n")
			  + "{\n\tatf_set \"descr\" \"Verify that an invalid usage "
			  + "with a supported option produces a valid error message"
			  + "\"\n}\n\ninvalid_usage_body()\n{";

		test_fstream << testcase_buffer + "\n}\n\n";
	}

	/*
	 * Add a testcase under "no_arguments" for
	 * running the utility without any arguments.
	 */
	if (annotation_set.find("*") == annotation_set.end()) {
		command = utility + " 2>&1 </dev/null";
		output = generatetest::Exec(command.c_str());
		addtestcase::NoArgsTestcase(util_with_section, output, test_fstream);
		testcase_list.append("\tatf_add_test_case no_arguments\n");
	}

	test_fstream << "atf_init_test_cases()\n{\n" + testcase_list + "}\n";
	test_fstream.close();
}

int
main()
{
	std::ifstream groff_list;
	std::list<std::pair<std::string, std::string>> utility_list;
	std::string test_file;  /* atf-sh test name. */
	std::string util_name;  /* Utility name. */
	const char *tests_dir = "generated_tests/";
	struct stat sb;
	struct dirent *ent;
	DIR *groff_dir;
	char answer;            /* User input to determine overwriting of test files. */
	std::string license; 	/* Customized license generated during runtime. */

	/*
	 * For testing (or generating tests for only selected utilities),
	 * the utility_list can be populated above during declaration.
	 */
	if (utility_list.empty()) {
		if ((groff_dir = opendir("groff"))) {
			readdir(groff_dir); 	/* Skip directory entry for "." */
			readdir(groff_dir); 	/* Skip directory entry for ".." */
			while ((ent = readdir(groff_dir))) {
				util_name = ent->d_name;
				utility_list.push_back(std::make_pair<std::string, std::string>
						      (util_name.substr(0, util_name.length() - 2),
						       util_name.substr(util_name.length() - 1, 1)));
			}
			closedir(groff_dir);
		} else {
			fprintf(stderr, "Could not open the directory: ./groff\nRefer to the "
					"section \"Populating groff scripts\" in README!\n");
			return EXIT_FAILURE;
		}
	}

	/* Check if the directory 'generated_tests' exists. */
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
	license = generatelicense::GenerateLicense();

	for (const auto &util : utility_list) {
		test_file = tests_dir + util.first + "_test.sh";
		/* TODO(shivansh) Check before overwriting existing test scripts. */

		setlinebuf(stdout);
		std::cout << "Generating test for: " + util.first
			   + '('+ util.second + ')' << " ...";

		boost::thread api_caller(generatetest::GenerateTest, util.first, util.second, license);
		if (api_caller.try_join_for(boost::chrono::seconds(TIMEOUT))) {
			/* API call successfully returned within TIMEOUT (seconds). */
			std::cout << "Successful\n";
		} else {
			/* API call timed out. */
			std::cout << "Failed!\n";
			/* Remove the incomplete test file. */
			remove(("generated_tests/" + util.first + "_test.sh").c_str());
		}
	}

	return EXIT_SUCCESS;
}
