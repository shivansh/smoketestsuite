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

#include <fstream>
#include <iostream>
#include "add_testcase.h"

// Adds a test-case for an option with known usage.
void
add_testcase::add_known_testcase(std::string option,
				 std::string util_with_section,
				 std::string descr,
				 std::string output,
				 std::ofstream& test_script)
{
	std::string testcase_name;
	std::string utility = util_with_section.substr(0,
			      util_with_section.length() - 3);

	// Add testcase name.
	test_script << "atf_test_case ";
	if (!option.empty()) {
		testcase_name = option;
		testcase_name.append("_flag");
	}
	else
		testcase_name = "no_arguments";
	test_script << testcase_name + "\n";

	// Add testcase description.
	test_script << testcase_name
		     + "_head()\n{\n\tatf_set \"descr\" ";
	if (!descr.empty())
		test_script << descr;
	else
		test_script << "\"Verify the usage of option \'" + option + "\'\"";
	test_script << "\n}\n\n";

	// Add body of the testcase.
	test_script << testcase_name + "_body()\n{"
		     + "\n\tatf_check -s exit:0 -o ";

	// Match the usage output if generated.
	if (!output.empty())
		test_script << "inline:\"" + output + "\" ";
	else
		test_script << "empty ";
	test_script << utility;

	if (!option.empty())
		test_script << " -" + option;
	test_script << "\n}\n\n";
}

// Adds a test-case for an option with unknown usage.
void
add_testcase::add_unknown_testcase(std::string option,
				   std::string util_with_section,
				   std::string output,
				   int exitstatus,
				   std::string& testcase_buffer)
{
	std::string utility = util_with_section.substr(0,
			      util_with_section.length() - 3);

	testcase_buffer.append("\n\tatf_check -s not-exit:0 -e ");

	// Check if a usage message was produced.
	if (!output.compare(0, 6, "usage:"))
		testcase_buffer.append("match:\"$usage_output\" ");
	else if (!output.empty())
		testcase_buffer.append("inline:\"" + output + "\" ");
	else
		testcase_buffer.append("empty ");

	testcase_buffer.append(utility);

	if (!option.empty())
		testcase_buffer.append(" -" + option);
}

// Adds a test-case for usage without any arguments.
void
add_testcase::add_noargs_testcase(std::string util_with_section,
				  std::pair<std::string, int> output,
				  std::ofstream& test_script)
{
	std::string descr;
	std::string utility = util_with_section.substr(0,
			      util_with_section.length() - 3);

	if (output.second) {
		// An error was encountered.
		test_script << std::string("atf_test_case no_arguments\n")
			     + "no_arguments_head()\n{\n\tatf_set \"descr\" ";
		if (!output.first.empty()) {
			// We expect a usage message to be generated in this case.
			if (!output.first.compare(0, 6, "usage:")) {
				descr = "\"Verify that " + util_with_section
				      + " fails and generates a valid usage"
				      + " message when no arguments are supplied\"";

				test_script << descr + "\n}\n\nno_arguments_body()\n{"
					       + "\n\tatf_check -s not-exit:0 -e match:\"$usage_output\" "
					       + utility;
			}
			else {
				descr = "\"Verify that " + util_with_section
				      + " fails and generates a valid output"
				      + " when no arguments are supplied\"";

				test_script << descr + "\n}\n\nno_arguments_body()\n{"
					     + "\n\tatf_check -s not-exit:0 -e inline:\""
					     + output.first + "\" "
					     + utility;
			}
		}

		else {
			descr = "\"Verify that " + util_with_section
				+ "fails silently when no arguments are supplied\"" ;
			test_script << descr + "\n}\n\nno_arguments_body()\n{"
				     + "\n\tatf_check -s not-exit:0 -e empty "
				     + utility;
		}

		test_script << "\n}\n\n";
	}

	else {
		// The command ran successfully, hence we guessed
		// a correct usage for the utility under test.
		if (!output.first.empty())
			descr = "\"Verify that " + util_with_section
			      + " executes successfully and produces a valid"
			      + " output when invoked without any arguments\"";
		else
			descr = "\"Verify that " + util_with_section
			      + " executes successfully and silently"
			      + " when invoked without any arguments\"";

		add_testcase::add_known_testcase("", util_with_section, descr,
						 output.first, test_script);
	}
}
