# Automation tool
An (in-progress) implementation of the automation described [here](https://shivrai.github.io/assets/tmp/GSoC17Automation.pdf).

## Instructions
Execute the following commands -
```
make
make run
```
The expected result (at the time of writing) should be generation of a atf-sh test file named `ln_test.sh`.

These are the options extracted from the man-page of ln(1). The aim for now is to use these options to produce tests for verifying whether they are actually meant for "help" and "version" operations respectively.
