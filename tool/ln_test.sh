#
# Copyright 2017 Shivansh Rai
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $FreeBSD$
#

atf_test_case F_flag
F_flag_head()
{
	atf_set "descr" "Verify that the option '-F' produces a valid error message in case of an invalid usage"
}

F_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -F
}

atf_test_case L_flag
L_flag_head()
{
	atf_set "descr" "Verify that the option '-L' produces a valid error message in case of an invalid usage"
}

L_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -L
}

atf_test_case P_flag
P_flag_head()
{
	atf_set "descr" "Verify that the option '-P' produces a valid error message in case of an invalid usage"
}

P_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -P
}

atf_test_case f_flag
f_flag_head()
{
	atf_set "descr" "Verify that the option '-f' produces a valid error message in case of an invalid usage"
}

f_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -f
}

atf_test_case h_flag
h_flag_head()
{
	atf_set "descr" "Verify that the option '-h' produces a valid error message in case of an invalid usage"
}

h_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -h
}

atf_test_case i_flag
i_flag_head()
{
	atf_set "descr" "Verify that the option '-i' produces a valid error message in case of an invalid usage"
}

i_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -i
}

atf_test_case n_flag
n_flag_head()
{
	atf_set "descr" "Verify that the option '-n' produces a valid error message in case of an invalid usage"
}

n_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -n
}

atf_test_case s_flag
s_flag_head()
{
	atf_set "descr" "Verify that the option '-s' produces a valid error message in case of an invalid usage"
}

s_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -s
}

atf_test_case v_flag
v_flag_head()
{
	atf_set "descr" "Verify that the option '-v' produces a valid error message in case of an invalid usage"
}

v_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -v
}

atf_test_case w_flag
w_flag_head()
{
	atf_set "descr" "Verify that the option '-w' produces a valid error message in case of an invalid usage"
}

w_flag_body()
{
	atf_check -s exit:1 -e inline:'usage: ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file [target_file]
       ln [-s [-F] | -L | -P] [-f | -i] [-hnv] source_file ... target_dir
       link source_file target_file
' ln -w
}

atf_init_test_cases()
{
	atf_add_test_case F_flag
	atf_add_test_case L_flag
	atf_add_test_case P_flag
	atf_add_test_case f_flag
	atf_add_test_case h_flag
	atf_add_test_case i_flag
	atf_add_test_case n_flag
	atf_add_test_case s_flag
	atf_add_test_case v_flag
	atf_add_test_case w_flag
}
