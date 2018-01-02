#
# Copyright 2017-2018 Shivansh Rai

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

atf_test_case j_flag
j_flag_head()
{
	atf_set "descr" "Verify the usage of option 'j'"
}

j_flag_body()
{
	atf_check -s exit:0 -o inline:"Fri Dec 29 11:46:38 IST 2017
" date -j
}

atf_test_case n_flag
n_flag_head()
{
	atf_set "descr" "Verify the usage of option 'n'"
}

n_flag_body()
{
	atf_check -s exit:0 -o inline:"Fri Dec 29 11:46:38 IST 2017
" date -n
}

atf_test_case R_flag
R_flag_head()
{
	atf_set "descr" "Verify the usage of option 'R'"
}

R_flag_body()
{
	atf_check -s exit:0 -o inline:"Fri, 29 Dec 2017 11:46:38 +0530
" date -R
}

atf_test_case u_flag
u_flag_head()
{
	atf_set "descr" "Verify the usage of option 'u'"
}

u_flag_body()
{
	atf_check -s exit:0 -o inline:"Fri Dec 29 06:16:38 UTC 2017
" date -u
}

atf_test_case invalid_usage
invalid_usage_head()
{
	atf_set "descr" "Verify that an invalid usage with a supported option " \
			"produces a valid error message"
}

invalid_usage_body()
{
	atf_check -s not-exit:0 -e inline:"date: option requires an argument -- d
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[[[cc]yy]mm]dd]HH]MM[.ss]] [+format]
" date -d
	atf_check -s not-exit:0 -e inline:"date: option requires an argument -- f
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[[[cc]yy]mm]dd]HH]MM[.ss]] [+format]
" date -f
	atf_check -s not-exit:0 -e inline:"date: option requires an argument -- r
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[[[cc]yy]mm]dd]HH]MM[.ss]] [+format]
" date -r
	atf_check -s not-exit:0 -e inline:"date: option requires an argument -- r
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[[[cc]yy]mm]dd]HH]MM[.ss]] [+format]
" date -r
	atf_check -s not-exit:0 -e inline:"date: option requires an argument -- t
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[[[cc]yy]mm]dd]HH]MM[.ss]] [+format]
" date -t
	atf_check -s not-exit:0 -e inline:"date: option requires an argument -- v
usage: date [-jnRu] [-d dst] [-r seconds] [-t west] [-v[+|-]val[ymwdHMS]] ... 
            [-f fmt date | [[[[[cc]yy]mm]dd]HH]MM[.ss]] [+format]
" date -v
}

atf_test_case no_arguments
no_arguments_head()
{
	atf_set "descr" "Verify that date(1) executes successfully and produces a valid " \
			"output when invoked without any arguments"
}

no_arguments_body()
{
	atf_check -s exit:0 -o inline:"Fri Dec 29 11:46:38 IST 2017
" date
}

atf_init_test_cases()
{
	atf_add_test_case j_flag
	atf_add_test_case n_flag
	atf_add_test_case R_flag
	atf_add_test_case u_flag
	atf_add_test_case invalid_usage
	atf_add_test_case no_arguments
}
