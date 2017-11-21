#!/usr/bin/env bash

# Auto generate single AllTests file for CuTest.
# Searches through all *.c files in the current directory.
# Prints to stdout.
# Author: Asim Jalis
# Date: 01/08/2003
#
# Modified by Fletcher T. Penney for proper error codes
# Modified by Fletcher T. Penney to handle spaces in path names

if test $# -eq 0 ; then FILES=("*.c") ; else FILES=("$@") ; fi

echo '

/* This is auto-generated code. Edit at your own peril. */
#include <stdio.h>
#include <stdlib.h>

#include "CuTest.h"

'

cat "${FILES[@]}" | grep '^void Test' | 
    sed -e 's/(.*$//' \
        -e 's/$/(CuTest*);/' \
        -e 's/^/extern /'

echo \
'

void RunAllTests(void) 
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    int failCount = 0;

'
cat "${FILES[@]}" | grep '^void Test' | 
    sed -e 's/^void //' \
        -e 's/(.*$//' \
        -e 's/^/    SUITE_ADD_TEST(suite, /' \
        -e 's/$/);/'

echo \
'
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\\n", output->buffer);
    CuStringDelete(output);

    failCount = suite->failCount;
    CuSuiteDelete(suite);

    if (failCount != 0)
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}

int main(void)
{
    RunAllTests();
}
'
