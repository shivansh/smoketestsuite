# Smoke testing of base utilities (FreeBSD)

Test generation tool made as a part of [Google Summer of Code '17 with FreeBSD](https://summerofcode.withgoogle.com/projects/#6426676740227072).  
**Refer the [FreeBSD wiki](https://wiki.freebsd.org/SummerOfCode2017/SmokeTestingOfBaseUtilities) for an overview and updates.**

## Directory Structure
```
.
├── deprecated_tests
│   └── ........................:: Tests pertaining to initial test plan
└── src ........................:: Automation tool pertaining to new test plan
    ├── annotations
    │   └── ....................:: Annotation files (generated/user-defined)
    ├── generated_tests
    │   └── ....................:: Generated atf-sh test scripts
    ├── scripts
    │   └── ....................:: Handy scripts
    ├── generate_test.cpp ......:: Test generator
    ├── read_annotations.cpp ...:: Annotation parser
    └── utils.cpp ..............:: Index generator
```

## Automation tool
An (in-progress) implementation of the automation tool briefly described [here](https://lists.freebsd.org/pipermail/soc-status/2017-July/001079.html).  
The following diagram summarizes how different components fit with the testcase-generator -  

![Automation-Tool](http://i.imgur.com/JhKM7h1.png)

- - -

## Dependencies
* Boost C++ libraries : The tool was tested to work with the port `boost-all-1.64.0`.

## Instructions

Clone the repository via -
```
git clone git@github.com:shivansh/smoketestsuite.git ~/smoketestsuite
```
The location `~/smoketestsuite` is important! If using a different location, the scripts under [src/scripts](src/scripts) need to be updated accordingly (for the time being).

### Populating groff scripts
* The directory [src/groff](src/groff) should be populated with the relevant groff scripts before proceeding for test generation. These scripts are available in the FreeBSD source tree. For filtering the utilities section wise, [fetch_groff.sh](src/scripts/fetch_groff.sh) sets the variable `section` to a default value of **1**. This value can be changed at will. However, it should be noted that currently the tool is tested to successfully generate **section 1 utilities** and might probably fail for other section numbers.

* The variable `src` in [fetch_groff.sh](src/scripts/fetch_groff.sh) should be updated to the local location of the FreeBSD source. The default value is `~/freebsd`.

* For populating `src/groff`, execute from [src](src) -
  ```
  make fetch_groff
  ```

### Generating tests
Execute the following commands inside [src](src) -
```
make clean
make && make run
```

A few demo tests are located in [src/generated_tests](src/generated_tests).
