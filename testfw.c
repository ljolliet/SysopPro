#define _GNU_SOURCE
#define TEST_SIZE 50
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

#include "testfw.h"

/* ********** STRUCTURES ********** */

struct testfw_t
{
    char *program;
    int timeout;
    char *logfile;
    char *cmd;
    bool silent;
    bool verbose;
    struct test_t *tests[TEST_SIZE];
    int tests_length;
};

/* ********** FRAMEWORK ********** */

struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
    // precisions in .h :
    //logfile : else NULL
    //cmd : else NULL
    //timeout: else 0
    struct testfw_t *fw = (struct testfw_t *)malloc(sizeof(struct testfw_t));
    fw->program = program;
    fw->timeout = timeout;
    fw->logfile = logfile;
    fw->cmd = cmd;
    fw->silent = silent;
    fw->verbose = verbose;
    for (int i = 0; i < TEST_SIZE; i++)
        fw->tests[i] = NULL;
    fw->tests_length = 0;
    return fw;
}

void testfw_free(struct testfw_t *fw)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to free");
        exit(EXIT_FAILURE);
    }
    else
    {
        free(fw);  // free all the content of the structure
        fw = NULL; // TO CHECK
    }
}

int testfw_length(struct testfw_t *fw)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to get length");
        exit(EXIT_FAILURE);
    }
    return fw->tests_length;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{

    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to get an element of it");
        exit(EXIT_FAILURE);
    }
    else if (k < 0)
    {
        fprintf(stderr, "impossible to get a negative element");
        exit(EXIT_FAILURE);
    }
    else if (k >= TEST_SIZE)
    {
        fprintf(stderr, "impossible to get an element out of the array");
        exit(EXIT_FAILURE);
    }
    return fw->tests[k];
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register a function in it");
        exit(EXIT_FAILURE);
    }
    else if (suite == NULL)
    {
        fprintf(stderr, "suite pointer is NULL, impossible to register a test without a test suite");
        exit(EXIT_FAILURE);
    }
    else if (name == NULL)
    {
        fprintf(stderr, "name pointer is NULL, impossible to register a test without a test name");
        exit(EXIT_FAILURE);
    }

    //CHECK FUNC ?

    struct test_t *test = (struct test_t *)malloc(sizeof(struct test_t)); // free in the testfw_free
    test->suite = suite;
    test->name = name;
    test->func = func;

    if (fw->tests_length >= TEST_SIZE)
    {
        fprintf(stderr, "impossible to add an element out of the array");
        exit(EXIT_FAILURE);
    }
    fw->tests[fw->tests_length] = test;
    fw->tests_length++;

    return test;
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register (symb) a function in it");
        exit(EXIT_FAILURE);
    }
    else if (suite == NULL)
    {
        fprintf(stderr, "suite pointer is NULL, impossible to register (symb) a test without a test suite");
        exit(EXIT_FAILURE);
    }
    else if (name == NULL)
    {
        fprintf(stderr, "name pointer is NULL, impossible to register (symb) a test without a test name");
        exit(EXIT_FAILURE);
    }
    char *underscore = "_";
    char *funcname = (char *)malloc(strlen(name) + strlen(suite) + strlen(underscore)); // final function name with size of name+_+suite
    strcpy(funcname, suite);                                                            //funcname = suite
    strcat(funcname, underscore);                                                       //funcname = suite_
    strcat(funcname, name);                                                             //funcame = suite_name
    void *handle = dlopen(fw->program, RTLD_LAZY);                                      // open program exec
    struct testfw_func_t * func = (struct testfw_func_t *)dlsym(handle, funcname);
    // dlclose(handle); // SEGFAULT IS HERE
    if(func == NULL)
        printf("func null\n");

    free(funcname); //free malloc funcname
    struct test_t *t = testfw_register_func(fw, suite, name,func); // func is a pointer ?
    return t;
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
     if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register a suite in it");
        exit(EXIT_FAILURE);
    }
    else if (suite == NULL)
    {
        fprintf(stderr, "suite pointer is NULL, impossible to register a suite without a test suite");
        exit(EXIT_FAILURE);
    }
    char *name = NULL;
    int size = 0;
    size_t len = 0;

    char *cmd = (char *)malloc(100);            // CHECK SIZE
    strcpy(cmd, "nm ");                         // cmd = nm
    strcat(cmd, fw->program);                   // cmd = nm <program>
    strcat(cmd, " | cut -d ' ' -f 3 | grep ^"); // ...
    strcat(cmd, suite);
    strcat(cmd, "_");                    // grep <suite>_*
    strcat(cmd, " | cut -d \"_\" -f 2"); // to get only the <name> (without <suite>_)

    FILE *fp = popen(cmd, "r");

    while (getline(&name, &len, fp) != -1)
    {
        printf("%s\n", name);
        testfw_register_symb(fw, suite, name);
        size++;
    }
    pclose(fp); //check value
    free(cmd);

    return size;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    // check pointers

    return 0;
}
