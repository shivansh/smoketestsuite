# Smoke testing of base utilities

## Directory Structure
```
.
├── baseutils
│   ├── ........................:: Base utilities
│   └── sample .................:: Example base utility
│       └── ....................:: Sample functional tests
└── parse.py ...................:: Argument parsing script (to be updated)
```
- - -

## Test Plan

### Test 1: Checking valid arguments
* The file [functional_test.c](baseutils/sample/functional_test.c) is an example test file which checks whether a program supports `--version` and `--help` as parameters. <br>
  **Note 1:** These are just sample arguments. <br>
  **Note 2:** This file is a sample test which checks basic functionality of a utility, namely the supported arguments.

* Suppose we have some utility and we pass the string `<utility> --version` as an argument when executing [functional_test.c](baseutils/sample/functional_test.c). The test file will check whether the utility supports the passed argument (in this case `--version`) by looking up available options in `long_options[]`.
  Since we don't know yet whether the utility supports `--version`, `long_options[]` needs to be correctly populated with the list of supported arguments.

#### Approaches for populating `long_options[]`
* We parse the man pages for getting the available arguments.
* We pass an invalid argument to the utility. This might produce a usage message which can be parsed.
