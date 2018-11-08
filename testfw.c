#define _GNU_SOURCE
#define TEST_SIZE 20
#include <stdbool.h>
#include <stdio.h>

#include "testfw.h"

/* ********** STRUCTURES ********** */

struct testfw_t // NOT SUR OF THE WAY TO DECLARE IT
{
    char *program;
    int timeout;
    char *logfile;
    char *cmd;
    bool silent;
    bool verbose;
    struct test_t *tests[TEST_SIZE];
    int tests_lengh;
};

/* ********** FRAMEWORK ********** */

struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
    // precisions in .h :
    //logfile : else NULL
    //cmd : else NULL
    //timeout: else 0
    //TO DO ? : ALLOC/MALLOC/REALLOC I DON'T KNOW
    struct testfw_t *fw = (testfw_t *)malloc(sizeof(testfw_t));
    fw->program = program;
    fw->timeout = timeout;
    fw->logfile = logfile;
    fw->cmd = cmd;
    fw->silent = silent;
    fw->verbose = verbose;
    fw->tests = malloc(sizeof(test_t));
    for(int i = 0; i< TEST_SIZE; i++)
        fw->tests[i] = NULL;
    fw->tests_lenght = 0;
    return fw; // GOOD ?
}

void testfw_free(struct testfw_t *fw)
{
  free(fw->tests);
  free(fw);
  fw = NULL;
    //TODO : FREE THE STRUCT CONTENT AND THE STRUCTURE ITSELF ? the free function frees the strcuture and its content
}

int testfw_length(struct testfw_t *fw)
{
    return fw->tests_lengh;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{
    // SHOULD BE ENOUGH
    if(k>=TEST_SIZE || k<0)    // to avoid a crash
        return NULL;
    return fw->tests[k];
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    struct test_t test = { .func = func, .suite = suite, .name = name};
    fw->tests[fw->tests_lengh] = fw;
    fw->tests_lengh++;
    return &test; // GOOD ?
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
