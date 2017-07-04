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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/wait.h>
#include "generate_test.h"

void
add_testcase(string option,
             string keyword,
             string utility,
             string output,
             ofstream& test_script)
{
  string testcase_name;

  // Add testcase name.
  test_script << "atf_test_case ";
  testcase_name = option;
  testcase_name.append("_flag");
  test_script << testcase_name + "\n";

  if (output.find("usage:") == string::npos) {
    // Absence of the string "usage:" in `output` denotes
    // that the `option` is a known one and EXIT_SUCCESS
    // will be encountered when it is executed.

    // Add testcase description.
    test_script << testcase_name
                 + "_head()\n{\n\tatf_set \"descr\" "
                 + "\"Verify the behavior for the option '-"
                 + option + "'\"" + "\n}\n\n";

    // Add body of the testcase.
    test_script << testcase_name
                 + "_body()\n{\n\tatf_check -s exit:0 -o match:\""
                 + keyword + "\" " + utility + " -" + option
                 + "\n}\n\n";
  } else {
    // Non-empty output denotes that stderr was not empty
    // and hence the command failed to execute. We add an
    // appropriate testcase to verify this behavior.

    // Add testcase description.
    test_script << testcase_name
                 + "_head()\n{\n\tatf_set \"descr\" "
                 + "\"Verify that the option \'-"
                 + option
                 + "\' produces a valid error message in case of an invalid usage\"\n}\n\n";

    // Add body of the testcase.
    test_script << testcase_name
                 + "_body()\n{\n\tatf_check -s exit:1 -e inline:\'"
                 + output + "\' " + utility + " -" + option + "\n}\n\n";
  }
}

string
exec(const char* cmd)
{
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }

    return result;
}

void
generate_test()
{
  list<utils::opt_rel*> ident_opt_list;
  string test_file;
  string output;
  string testcase_list;
  string command;
  struct stat buffer;
  ofstream test_fstream;
  ifstream license_fstream;

  utils::opt_def f_opts;
  ident_opt_list = f_opts.check_opts();
  test_file = f_opts.utility + "_test.sh";

  // Check if the test file exists. In case it does, stop execution.
  if (stat (test_file.c_str(), &buffer) == 0) {
    cout << "Skipping: Test file already exists" << endl;
    return;
  }

  test_fstream.open(test_file, ios::out);
  license_fstream.open("license", ios::in);

  test_fstream << license_fstream.rdbuf();
  license_fstream.close();

  // If a known option was encountered (i.e. `ident_opt_list` is
  // not empty), produce a testcase to check the validity of the
  // result of that option. If no known option was encountered,
  // produce testcases to verify the correct usage message
  // when using the valid options incorrectly.

  // Add testcases for known options.
  if (!ident_opt_list.empty()) {
    for (auto i = ident_opt_list.begin(); i != ident_opt_list.end(); i++) {
      add_testcase((*i)->value, (*i)->keyword,
                   f_opts.utility, output, test_fstream);
      testcase_list.append("atf_add_test_case " + (*i)->value + "_flag\n");
    }
  }

  // Add testcases for the options whose usage is unknown.
  for (int i = 0; i < f_opts.opt_list.length(); i++) {
    command = f_opts.utility + " -" + f_opts.opt_list.at(i)
                     + " 2>&1";
    output = exec(command.c_str());
    if (!output.empty()) {
      add_testcase(string(1, f_opts.opt_list.at(i)), "",
                   f_opts.utility, output, test_fstream);
      testcase_list.append("\tatf_add_test_case "
                           + string(1, f_opts.opt_list.at(i)) + "_flag\n");
    }
  }

  test_fstream << "atf_init_test_cases()\n{\n" + testcase_list + "}";
  test_fstream.close();
}

int
main()
{
  generate_test();
  return 0;
}
