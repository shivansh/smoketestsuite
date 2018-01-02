/*-
 * Copyright 2017-2018 Shivansh Rai
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "generate_license.h"
#include "utils.h"

std::string
generatelicense::GenerateLicense(int argc, char **argv)
{
	std::string license;
	std::string copyright_owner;

	if (argc > 1) {
		if (argc == 3 && strncmp(argv[1], "--name ", 7))
			copyright_owner = argv[2];
		else {
			std::cerr << "Usage: ./generate_tests --name <copyright_owner>\n";
			exit(EXIT_FAILURE);
		}
	} else
		copyright_owner = utils::Execute("id -P | cut -d : -f 8").first;

	license =
		"#\n"
		"# Copyright 2017-2018 " + copyright_owner + "\n"
		"# All rights reserved.\n"
		"#\n"
		"# Redistribution and use in source and binary forms, with or without\n"
		"# modification, are permitted provided that the following conditions\n"
		"# are met:\n"
		"# 1. Redistributions of source code must retain the above copyright\n"
		"#    notice, this list of conditions and the following disclaimer.\n"
		"# 2. Redistributions in binary form must reproduce the above copyright\n"
		"#    notice, this list of conditions and the following disclaimer in the\n"
		"#    documentation and/or other materials provided with the distribution.\n"
		"#\n"
		"# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND\n"
		"# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
		"# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
		"# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE\n"
		"# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
		"# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
		"# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
		"# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
		"# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
		"# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
		"# SUCH DAMAGE.\n"
		"#\n"
		"# $FreeBSD$\n"
		"#\n\n";

	return license;
}
