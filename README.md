# Smoke testing of base utilities

## Directory Structure
```
.
├── baseutils
│   ├── ........................:: Base utilities
│   └── ls
│       └── tests ..............:: Smoke tests
└── parse.py ...................:: Argument parsing script (to be updated)
```
- - -

## Test Plan

### Test 1: Checking valid arguments
* The file [functional_test.c](baseutils/ls/tests/functional_test.c) is an example test file which checks whether `ls` program supports `--version` and `--help` as parameters. <br>
  **Note 1:** This file is a WIP test which checks basic functionality of `ls` utility, namely the supported arguments.
  **Note 2:** This file provides an example workflow.

* Suppose we pass the string `ls --version` as an argument when executing [functional_test.c](baseutils/ls/tests/functional_test.c). The test file will check whether `ls` supports the passed option (in this case `--version`) by looking up available options in `long_options[]`. If it does, then the command will be executed.
  In case `ls` is not properly linked, `ls --version` will fail to execute and an error messaged will be generated.

#### Approaches for populating `long_options[]`
* We parse the man pages for getting the available arguments.
* We pass an invalid argument to the utility. This might produce a usage message which can be parsed.
