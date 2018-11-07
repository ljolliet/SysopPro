#define _GNU_SOURCE
#define TEST_SIZE 20
#include <stdbool.h>
#include <stdio.h>

#include "testfw.h"

/* ********** STRUCTURES ********** */

typedef struct testfw_t // NOT SUR OF THE WAY TO DECLARE IT
{
    char *program;
    int timeout;
    char *logfile;
    char *cmd;
    bool silent;
    bool verbose;
    struct test_t * tests[TEST_SIZE];
}testfw_t;

/* ********** FRAMEWORK ********** */

struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
    //TO DO ? : ALLOC/MALLOC/REALLOC I DON'T KNOW
    testfw_t t = { .program = program, .timeout = timeout, .logfile = logfile, .cmd = cmd, .silent = silent, .verbose = verbose };
    for(int i = 0; i< TEST_SIZE; i++)
        t.tests[i] = NULL;
    return &t;
}

void testfw_free(struct testfw_t *fw)
{
    //TODO : FREE THE STRUCT CONTENT AND THE STRUCTURE ITSELF ?
}

int testfw_length(struct testfw_t *fw)
{
    // SHOULD BE ENOUGH
    int length = 0;
    for(int i = 0; i<TEST_SIZE; i++)
       if(fw->tests[i] != NULL)
            length++;
    return length;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{
    // SHOULD BE ENOUGH
    if(k>=TEST_SIZE)    // to avoid a crash
        return NULL;
    return fw->tests[k];
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    return NULL;
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    return NULL;
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    return 0;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    return 0;
}
