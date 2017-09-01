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
#include "generate_test.h"
#include "add_testcase.h"
#include "read_annotations.h"

#define TIMEOUT 1       // threshold (seconds) for a function call to return.

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
  std::ifstream license_ifs;                  // Input stream for license.
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
  license_ifs.open("license", std::ios::in);

  test_ofs << license_ifs.rdbuf();
  license_ifs.close();

  // If a known option was encountered (i.e. `ident_opt_list` is
  // populated), produce a testcase to check the validity of the
  // result of that option. If no known option was encountered,
  // produce testcases to verify the correct (generated) usage
  // message when using the supported options incorrectly.

  // Add testcases for known options.
  if (!ident_opt_list.empty()) {
    for (const auto &i : ident_opt_list) {
      command = utility + " -" + i->value + " 2>&1";
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
      command = utility + " -" + f_opts.opt_list.front() + " 2>&1";
      output = generate_test::exec(command.c_str());

      if (output.second)
        test_ofs << "usage_output=\'" + output.first + "\'\n\n";
    }

    else {
      // Utility supports multiple options. In case the usage message
      // is consistent for atleast "two" options, we reduce duplication
      // by assigning a variable "usage_output" in the test script.
      for (const auto &i : f_opts.opt_list) {
        command = utility + " -" + i + " 2>&1";
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

      command = utility + " -" + i + " 2>&1";
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
    command = utility + " 2>&1";
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
  struct stat buffer;
  struct dirent *ent;
  DIR *dir;
  char answer;            // User input to determine overwriting of test files.
  int flag = 0;

  // For testing (or generating tests for only selected utilities),
  // the utility_list can be populated above during declaration.
  if (utility_list.empty()) {
    if ((dir = opendir("groff"))) {
      while ((ent = readdir(dir))) {
        util_name = ent->d_name;
        utility_list.push_back(std::make_pair<std::string, std::string>
                              (util_name.substr(0, util_name.length() - 2),
                               util_name.substr(util_name.length() - 1, 1)));
      }
      closedir(dir);
    }
    else {
      fprintf(stderr, "Could not open directory: groff/");
      return EXIT_FAILURE;
    }
  }

  for (const auto &util : utility_list) {
    test_file = "generated_tests/" + util.first + "_test.sh";

    // Check if the test file already exists. In
    // case it does, confirm before proceeding.
    if (!flag && !stat(test_file.c_str(), &buffer)) {
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

    std::cout << "Generating test for: " + util.first
               + '('+ util.second + ')' << " ...";

    boost::thread api_caller(generate_test::generate_test, util.first, util.second);
    if (api_caller.timed_join(boost::posix_time::seconds(TIMEOUT))) {
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
