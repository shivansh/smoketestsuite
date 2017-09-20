//
// Copyright 2017 Shivansh Rai
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
// $FreeBSD$

#include <array>
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

#define TIMEOUT 1       // threshold (seconds) for a function call to return.

std::string license;    // license file generated during runtime.

// Executes the passed argument "cmd" in a shell
// and returns its output and the exit status.
std::pair<std::string, int>
generate_test::exec(const char* cmd)
{
	const int bufsize = 128;
	std::array<char, bufsize> buffer;
	std::string usage_output;
	FILE* pipe = popen(cmd, "r");

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

// Generate a test for the given utility.
void
generate_test::generate_test(std::string utility,
			     std::string section)
{
	std::list<utils::opt_rel*> ident_opt_list;  // List of identified option relations.
	std::vector<std::string> usage_messages;    // Vector to store usage messages for comparison.
	std::string command;                        // Command to be executed in shell.
	std::string descr;                          // Testcase description.
	std::string testcase_list;                  // List of testcases.
	std::string testcase_buffer;                // Buffer for (temporarily) holding testcase data.
	std::string test_file;                      // atf-sh test name.
	std::string util_with_section;              // Section number appended to utility.
	std::ofstream test_ofs;                     // Output stream for the atf-sh test.
	std::pair<std::string, int> output;         // Return value type for `exec()`.
	std::unordered_set<std::string> annot;      // Hashset of utility specific annotations.
	int temp;

	// Read annotations and populate hash set "annot".
	annotations::read_annotations(utility, annot);

	util_with_section = utility + '(' + section + ')';

	utils::opt_def f_opts;
	ident_opt_list = f_opts.check_opts(utility);
	test_file = "generated_tests/" + utility + "_test.sh";

	// Add license in the generated test scripts.
	test_ofs.open(test_file, std::ios::out);
	test_ofs << license;

	// If a known option was encountered (i.e. `ident_opt_list` is
	// populated), produce a testcase to check the validity of the
	// result of that option. If no known option was encountered,
	// produce testcases to verify the correct (generated) usage
	// message when using the supported options incorrectly.

	// Add testcases for known options.
	if (!ident_opt_list.empty()) {
		for (const auto &i : ident_opt_list) {
			command = utility + " -" + i->value + " 2>&1 </dev/null";
			output = generate_test::exec(command.c_str());
			if (!output.first.compare(0, 6, "usage:")) {
				add_testcase::add_known_testcase(i->value, util_with_section,
								 descr, output.first, test_ofs);
			}
			else {
				// A usage message was produced, i.e. we
				// failed to guess the correct usage.
				add_testcase::add_unknown_testcase(i->value, util_with_section, output.first,
								   output.second, testcase_buffer);
			}
			testcase_list.append("\tatf_add_test_case " + i->value + "_flag\n");
		}
	}

	// Add testcases for the options whose usage is not known (yet).
	if (!f_opts.opt_list.empty()) {

		// For the purpose of adding a "$usage_output" variable,
		// we choose the option which produces one.
		// TODO Avoid double executions of an option, i.e. one while
		// selecting usage message and another while generating testcase.

		if (f_opts.opt_list.size() == 1) {
			// Utility supports a single option, check if it produces a usage message.
			command = utility + " -" + f_opts.opt_list.front() + " 2>&1 </dev/null";
			output = generate_test::exec(command.c_str());

			if (output.second)
				test_ofs << "usage_output=\'" + output.first + "\'\n\n";
		}

		else {
			// Utility supports multiple options. In case the usage message
			// is consistent for atleast "two" options, we reduce duplication
			// by assigning a variable "usage_output" in the test script.
			for (const auto &i : f_opts.opt_list) {
				command = utility + " -" + i + " 2>&1 </dev/null";
				output = generate_test::exec(command.c_str());

				if (output.second && usage_messages.size() < 3)
					usage_messages.push_back(output.first);
			}

			temp = usage_messages.size();
			for (int j = 0; j < temp; j++) {
				if (!(usage_messages.at(j)).compare(usage_messages.at((j+1) % temp))) {
					test_ofs << "usage_output=\'"
						  + output.first.substr(0, 7 + utility.size())
						  + "\'\n\n";
					break;
				}
			}
		}

		// Execute the utility with supported options
		// and add (+ve)/(-ve) testcases accordingly.
		for (const auto &i : f_opts.opt_list) {
			// If the option is annotated, skip it.
			if (annot.find(i) != annot.end())
				continue;

			command = utility + " -" + i + " 2>&1 </dev/null";
			output = generate_test::exec(command.c_str());

			if (output.second) {
				// Non-zero exit status was encountered.
				add_testcase::add_unknown_testcase(i, util_with_section, output.first,
								   output.second, testcase_buffer);
			}
			else {
				// EXIT_SUCCESS was encountered. Hence,
				// the guessed usage was correct.
				add_testcase::add_known_testcase(i, util_with_section, "",
								 output.first, test_ofs);
				testcase_list.append(std::string("\tatf_add_test_case ")
						     + i + "_flag\n");
			}
		}

		testcase_list.append("\tatf_add_test_case invalid_usage\n");
		test_ofs << std::string("atf_test_case invalid_usage\ninvalid_usage_head()\n")
			  + "{\n\tatf_set \"descr\" \"Verify that an invalid usage "
			  + "with a supported option produces a valid error message"
			  + "\"\n}\n\ninvalid_usage_body()\n{";

		test_ofs << testcase_buffer + "\n}\n\n";
	}

	// Add a testcase under "no_arguments" for
	// running the utility without any arguments.
	if (annot.find("*") == annot.end()) {
		command = utility + " 2>&1 </dev/null";
		output = generate_test::exec(command.c_str());
		add_testcase::add_noargs_testcase(util_with_section, output, test_ofs);
		testcase_list.append("\tatf_add_test_case no_arguments\n");
	}

	test_ofs << "atf_init_test_cases()\n{\n" + testcase_list + "}\n";
	test_ofs.close();
}

int
main()
{
	std::ifstream groff_list;
	std::list<std::pair<std::string, std::string>> utility_list;
	std::string test_file;  // atf-sh test name.
	std::string util_name;  // Utility name.
	const char *tests_dir = "generated_tests/";
	struct stat sb;
	struct dirent *ent;
	DIR *groff_dir;
	char answer;            // User input to determine overwriting of test files.
	int flag = 0;

	// For testing (or generating tests for only selected utilities),
	// the utility_list can be populated above during declaration.
	if (utility_list.empty()) {
		if ((groff_dir = opendir("groff"))) {
			readdir(groff_dir); 	// Skip directory entry for "."
			readdir(groff_dir); 	// Skip directory entry for ".."
			while ((ent = readdir(groff_dir))) {
				util_name = ent->d_name;
				utility_list.push_back(std::make_pair<std::string, std::string>
						      (util_name.substr(0, util_name.length() - 2),
						       util_name.substr(util_name.length() - 1, 1)));
			}
			closedir(groff_dir);
		}
		else {
			fprintf(stderr, "Could not open the directory: ./groff\nRefer to the "
					"section \"Populating groff scripts\" in README!\n");
			return EXIT_FAILURE;
		}
	}

	// Check if the directory 'generated_tests' exists.
	if (stat(tests_dir, &sb) || !S_ISDIR(sb.st_mode)) {
		boost::filesystem::path dir(tests_dir);
		if (boost::filesystem::create_directory(dir))
			std::cout << "Directory created: " << tests_dir << std::endl;
	}

	// Generate a license to be added in the generated scripts.
	license = add_license::generate_license();

	for (const auto &util : utility_list) {
		test_file = tests_dir + util.first + "_test.sh";

		// Check if the test file already exists. In
		// case it does, confirm before proceeding.
		if (!flag && !stat(test_file.c_str(), &sb)) {
			std::cout << "Test file(s) already exists. Overwrite? [y/n] ";
			std::cin >> answer;

			switch (answer) {
				case 'n':
				case 'N':
					fprintf(stderr, "Stopping execution\n");
					flag = 1;
					break;

				default:
					// TODO capture newline character
					flag = 1;
					break;
			}
		}

		// Enable line-buffering on stdout.
		setlinebuf(stdout);

		std::cout << "Generating test for: " + util.first
			     + '('+ util.second + ')' << " ...";

		boost::thread api_caller(generate_test::generate_test, util.first, util.second);
		if (api_caller.try_join_for(boost::chrono::seconds(TIMEOUT))) {
			// API call successfully returned within TIMEOUT (seconds).
			std::cout << "Successful\n";
		}
		else {
			// API call timed out.
			std::cout << "Failed!\n";
			// Remove the incomplete test file.
			remove(("generated_tests/" + util.first + "_test.sh").c_str());
		}
	}

	return EXIT_SUCCESS;
}
