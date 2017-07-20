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

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include "run_tests.h"

void
generate_annotations()
{
  DIR *dirp;
  struct dirent *dp;
  string test_script;
  string util;

  dirp = opendir("generated_tests");
  if (dirp == NULL) {
    fprintf(stderr, "Warning: Cannot open directory generated_tests\n");
    return;
  }

  while ((dp = readdir(dirp)) != NULL) {
    test_script = dp->d_name;
    // Ignore hidden files.
    if (test_script[0] != '.') {
      // Extract utility name from test file.
      // TODO Maintain a list of utilities and their locations.
      util = test_script.substr(0, test_script.find("_test"));
    }
  }

  (void)closedir(dirp);
}

void
read_annotations(unordered_set<char>& annot)
{
  string line;
  // TODO do this for all the annotation files
  ifstream annot_fstream;
  annot_fstream.open("annotations/date_test.annot");

  while (getline(annot_fstream, line)) {
    annot.insert(line[0]);
  }

  annot_fstream.close();
}
