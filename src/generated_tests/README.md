# Generated tests
**NOTE:** The generated tests are maintained at [shivansh/smoketests](https://github.com/shivansh/smoketests). The directory [generated_tests](src/generated_tests) only contains a few demos.  
The following table summarizes the type of test-cases produced for 3 (randomly chosen) utilities.

|           **Test**            | **Positive test-cases** | **Negative test-cases** |
----------------------------|:-------------------:|:-------------------:|
[date_test.sh](src/generated_tests/date_test.sh)    | ~~5~~ 0 | ~~2~~ 6
[ln_test.sh](src/generated_tests/ln_test.sh)        | 0       | 11
[stdbuf_test.sh](src/generated_tests/stdbuf_test.sh)| 1       | ~~0~~ 3

Some key-points worth noting :
* [date_test.sh](src/generated_tests/date_test.sh) is time/timezone dependent, hence all the test-cases apart from `invalid_usage` will fail. The aim for using this utility is to demonstrate that the tool can generate positive test-cases along with negative ones. It also demonstrates the need to sanity check the tests.
  - After adding functionality to generate annotations, the 5 timezone dependent testcases were removed.
  - Adding functionality for long options increased the number of negative testcases by 4.

* [ln_test.sh](src/generated_tests/ln_test.sh) demonstrates that if the utility produces a common usage message for every invalid usage, then the tool can define a variable containing this message and avoid duplication while generating the test-cases. Also, all the negative tests are collated under a single testcase named `invalid_usage`.

* The groff script for `stdbuf(1)` is slightly different than others, hence
the tool couldn’t generate any negative test-case in [stdbuf_test.sh](src/generated_tests/stdbuf_test.sh) as it couldn't pick up the supported options. This scenario will be taken into account next. The aim for using this utility is to demonstrate that the tool will always produce atleast one positive/negative testcase.
  - Adding functionality for long options increased the number of negative testcases by 3.
