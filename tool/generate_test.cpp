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
#include <boost/thread/thread.hpp>
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

std::pair<std::string, int>
generate_test::exec(const char* cmd)
{
  std::array<char, 128> buffer;
  std::string usage_output;
  FILE* pipe = popen(cmd, "r");

  if (!pipe) throw std::runtime_error("popen() failed!");
  try {
    while (!feof(pipe)) {
        if (std::fgets(buffer.data(), 128, pipe) != NULL)
            usage_output += buffer.data();
    }
  }
  catch(...) {
    pclose(pipe);
    throw "Unable to execute the command: " + std::string(cmd);
  }

  return std::make_pair<std::string, int>
         ((std::string)usage_output, WEXITSTATUS(pclose(pipe)));
}

void
generate_test::generate_test(std::string utility)
{
  std::list<utils::opt_rel*> ident_opt_list;  // List of identified option relations.
  std::string test_file;             // atf-sh test name.
  std::string testcase_list;         // List of testcases.
  std::string command;               // Command to be executed in shell.
  std::string descr;                 // Testcase description.
  std::string testcase_buffer;       // Buffer for (temporarily) holding testcase data.
  std::ofstream test_ofs;            // Output stream for the atf-sh test.
  std::ifstream license_ifs;         // Input stream for license.
  std::pair<std::string, int> output;     // Return value type for `exec()`.
  std::unordered_set<char> annot;

  // Read annotations and populate hash set "annot".
  annotations::read_annotations(utility, annot);

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
        add_testcase::add_known_testcase(i->value, utility, descr,
                                         output.first, test_ofs);
      }
      else {
        // A usage message was produced, i.e. we
        // failed to guess the correct usage.
        add_testcase::add_unknown_testcase(i->value, utility, output.first, testcase_buffer);
      }
      testcase_list.append("\tatf_add_test_case " + i->value + "_flag\n");
    }
  }

  // Add testcases for the options whose usage is not known (yet).
  if (!f_opts.opt_list.empty()) {

    // For the purpose of adding a "$usage_output" variable,
    // we choose the option which produces one.
    // TODO Avoid multiple executions of an option
    for (const auto &i : f_opts.opt_list) {
      command = utility + " -" + i + " 2>&1";
      output = generate_test::exec(command.c_str());
      if (!output.first.compare(0, 5, "usage") ||
          !output.first.compare(0, 5, "Usage") &&
          output.second) {
        test_ofs << "usage_output=\'" + output.first + "\'\n\n";
        break;
      }
    }

    // Execute the utility with supported options
    // and add (+ve)/(-ve) tests accordingly.
    for (const auto &i : f_opts.opt_list) {
      // If the option is annotated, skip it.
      if (annot.find(i) != annot.end())
        continue;

      command = utility + " -" + i + " 2>&1";
      output = generate_test::exec(command.c_str());

      if (output.second) {
        // Non-zero exit status was encountered.
        add_testcase::add_unknown_testcase(std::string(1, i), utility,
                                           output.first, testcase_buffer);
      }
      else {
        // EXIT_SUCCESS was encountered. Hence,
        // the guessed usage was correct.
        add_testcase::add_known_testcase(std::string(1, i), utility,
                                         "", output.first, test_ofs);
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
  if (annot.find('*') == annot.end()) {
    command = utility + " 2>&1";
    output = generate_test::exec(command.c_str());
    add_testcase::add_noargs_testcase(utility, output, test_ofs);
    testcase_list.append("\tatf_add_test_case no_arguments\n");
  }

  test_ofs << "atf_init_test_cases()\n{\n" + testcase_list + "}\n";
  test_ofs.close();
}

int
main()
{
  std::ifstream groff_list;
  std::list<std::string> utility_list;
  std::string test_file;  // atf-sh test name.
  struct stat buffer;
  std::string util_name;
  DIR *dir;
  struct dirent *ent;
  char answer;            // User input to determine overwriting of test files.
  int flag = 0;

  if ((dir = opendir("groff")) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      util_name = ent->d_name;
      utility_list.push_back(util_name.substr(0, util_name.length()-2));
    }
    closedir(dir);
  }
  else {
    fprintf(stderr, "Could not open directory: groff/");
    return EXIT_FAILURE;
  }

  for (const auto &util : utility_list) {
    test_file = "generated_tests/" + util + "_test.sh";

    // Check if the test file exists.
    // In case the test file exists, confirm before proceeding.
    if (!stat(test_file.c_str(), &buffer) && !flag) {
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
      }
    }

    std::cout << "Generating test for: " + util << std::endl;

    boost::thread api_caller(generate_test::generate_test, util);
    if (api_caller.timed_join(boost::posix_time::seconds(10))) {
      // API call returned within 10 seconds
    }
    else {
      // API call timed out
      continue;
    }
  }

  return 0;
}
