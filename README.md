# Smoke testing of base utilities

## Directory Structure
```
.
├── baseutils
│   ├── ........................:: Base utilities
│   └── ls
│       └── tests ..............:: Smoke tests
└── parse_options.py ...........:: Options parsing script (to be updated)
```
- - -

## Test Plan

### Test 1: Checking valid arguments
The file [functional_test.c](baseutils/ls/tests/functional_test.c) is a simple test file which checks whether the `ls` program is properly linked by running trivial commands. The arguments supported by ls are stored in `long_options[]`.
**Note 1:** This file is a WIP test which checks basic functionality of `ls` utility, namely the supported arguments.
**Note 2:** This file provides an example workflow.

The smoke tests can be run as follows from inside **baseutils/_utility_/tests** -
```
>> make
>> make test
```

It should be noted that finally the tests will be automated with appropriately passed options. The above commands will not have to be run for testing individual programs.

The test file uses `getopt()` and `getopt_long()` for testing the validity of the passed options. If a valid option is passed, the command `<utility> --<option(i)>` is executed.
In case the command fails to execute for a valid option, this will imply that the utility under test is not properly linked.

#### Populating `short_options[]` and `long_options[]`

`short_options[]` and `long_options[]` need to be initially populated with a few supported options for all the base utilities. This can be done by using either one of the following available approaches -
* Parse man pages for each utility to get the supported options.
* Pass an unsupported option to the utility. This might generate a usage message which can then be parsed. **Note:** The unsupported option will be chosen experimentally.

An automation script [parse_options.py](parse_options.py) will be written which will populate `short_options[]` and `long_options[]` by following the above mentioned approaches and will generate the relevant test files.
