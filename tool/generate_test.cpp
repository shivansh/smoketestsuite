#include <fstream>
#include <iostream>
#include "generate_test.hpp"

void
add_testcase(const char& option, std::ofstream& test_script) {
  test_script << "atf_add_test_case ";
  std::string testcase_name = std::string(1, option);
  testcase_name.append("_flag");
  test_script << testcase_name << "\n";

  test_script << testcase_name << "_head()\n{\n\tatf_set \"descr\" ";

  std::string testcase_descr = "\"Verify the behavior for option '-";
  testcase_descr.append(std::string(1, option));
  testcase_descr.append("'\"");

  test_script << testcase_descr << "\n";
  test_script << "}\n\n" ;

  // TODO: avoid hardcoding the utility name.
  test_script << testcase_name << "_body()\n{\n\tatf_check -s exit:0 ln -"
              << option << "\n}\n\n";
}

void
generate_test() {
  tool::insert_opts io;
  std::string opt_string = io.check_opts();

  std::ofstream ln_test;
  ln_test.open("ln_test.sh");

  for (int i = 0; i < opt_string.length(); i++)
    add_testcase(opt_string.at(i), ln_test);

  ln_test.close();
}

int main() {
  generate_test();
  return 0;
}
