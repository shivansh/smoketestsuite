#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/wait.h>
#include "generate_test.hpp"

void
add_testcase(const char& option, string utility,
             ofstream& test_script, string output)
{
  // Add testcase name.
  test_script << "atf_add_test_case ";
  string testcase_name = string(1, option);
  testcase_name.append("_flag");
  test_script << testcase_name << "\n";

  if (output.empty()) {
    // Add testcase description.
    test_script << testcase_name
                << "_head()\n{\n\tatf_set \"descr\" "
                << "\"Verify the behavior for the option '-"
                << string(1, option)
                << "'\""
                << "\n}\n\n";

    // Add body of the testcase.
    test_script << testcase_name
                << "_body()\n{\n\tatf_check -s exit:0 "
                << utility
                << " -"
                << option
                << "\n}\n\n";
  } else {
    // Add testcase description.
    test_script << testcase_name
                << "_head()\n{\n\tatf_set \"descr\" "
                << "\"Verify if the option \'-"
                << string(1, option)
                << "\' produces a valid error message in case of an invalid usage\""
                << "\n}\n\n";

    // Add body of the testcase.
    test_script << testcase_name
                << "_body()\n{\n\tatf_fail -o inline:\""
                << output
                << "\"\n}\n\n";
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
    if (result.back() == '\n')
      result.erase(result.end() - 1);

    return result;
}

void
generate_test()
{
  tool::find_opts f_opts;
  string opt_string = f_opts.check_opts();
  string test_file = f_opts.utility + "_test.sh";

  // Check if the test file exists. In case it does, stop.
  struct stat buffer;
  if (stat (test_file.c_str(), &buffer) == 0) {
    cout << "Skipping: Test file already exists" << endl;
    return;
  }

  ofstream test_fstream;
  test_fstream.open(test_file);

  // TODO Add license to the test script.

  // If a known option was encountered, produce a testcase
  // to check the validity of the result of that option.
  // If no known option was not encountered, produce
  // testcases to verify the correct usage message when
  // using the valid options incorrectly.

  string output;
  if (!opt_string.empty())
    for (int i = 0; i < opt_string.length(); i++)
      add_testcase(opt_string.at(i), f_opts.utility, test_fstream, output);
  else {
    for (int i = 0; i < f_opts.opt_list.length(); i++) {
      string command = f_opts.utility + " -" + f_opts.opt_list.at(i)
                       + " 2>&1";
      output = exec(command.c_str());
      if (!output.empty())
        add_testcase(f_opts.opt_list.at(i), f_opts.utility, test_fstream, output);
    }
  }

  test_fstream.close();
}

int
main()
{
  generate_test();
  return 0;
}
