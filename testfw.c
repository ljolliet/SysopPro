#define _GNU_SOURCE
#define TEST_SIZE 100
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
    char *separator;
    struct test_t tests[TEST_SIZE];
    int tests_length;
};

/* ********** FRAMEWORK ********** */

struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
    struct testfw_t *fw = (struct testfw_t *)malloc(sizeof(struct testfw_t));
    fw->program = program;
    fw->timeout = timeout;
    fw->logfile = logfile;
    fw->cmd = cmd;
    fw->silent = silent;
    fw->verbose = verbose;
    fw->separator = "_";
    fw->tests_length = 0;
    return fw;
}

void testfw_free(struct testfw_t *fw)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to free\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        for (int i = 0; i < fw->tests_length; i++) // free content of fw->tests[]
        {
            free(fw->tests[i].name);
            free(fw->tests[i].suite);
        }
        free(fw); // free the framework structure
        fw = NULL;
    }
}

int testfw_length(struct testfw_t *fw)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to get length\n");
        exit(EXIT_FAILURE);
    }
    return fw->tests_length;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{

    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to get an element of it\n");
        exit(EXIT_FAILURE);
    }
    else if (k >= TEST_SIZE || k < 0)
    {
        fprintf(stderr, "impossible to get an element out of the array\n");
        exit(EXIT_FAILURE);
    }
    return &fw->tests[k];
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register a function in it\n");
        exit(EXIT_FAILURE);
    }
    else if (suite == NULL)
    {
        fprintf(stderr, "suite pointer is NULL, impossible to register a test without a suite\n");
        exit(EXIT_FAILURE);
    }
    else if (name == NULL)
    {
        fprintf(stderr, "name pointer is NULL, impossible to register a test without a name\n");
        exit(EXIT_FAILURE);
    }
    else if (func == NULL)
    {
        fprintf(stderr, "func pointer is NULL, impossible to register\n");
        exit(EXIT_FAILURE);
    }
    else if (fw->tests_length >= TEST_SIZE)
    {
        fprintf(stderr, "impossible to add an element out of the array\n");
        exit(EXIT_FAILURE);
    }

    fw->tests[fw->tests_length].name = (char *)malloc(100);
    fw->tests[fw->tests_length].suite = (char *)malloc(100);
    fw->tests[fw->tests_length].func = func;
    strcpy(fw->tests[fw->tests_length].name, name);
    strcpy(fw->tests[fw->tests_length].suite, suite);

    return &fw->tests[fw->tests_length++];
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register (symb) a function in it\n");
        exit(EXIT_FAILURE);
    }
    else if (suite == NULL)
    {
        fprintf(stderr, "suite pointer is NULL, impossible to register (symb) a test without a test suite\n");
        exit(EXIT_FAILURE);
    }
    else if (name == NULL)
    {
        fprintf(stderr, "name pointer is NULL, impossible to register (symb) a test without a test name\n");
        exit(EXIT_FAILURE);
    }
   
    int size = strlen(name) + strlen(suite) + strlen(fw->separator);
    char *funcname = (char *)malloc(size);        // final function name with size of name+_+suite
    strcpy(funcname, suite);                      //funcname = suite
    strcat(funcname, fw->separator);                  //funcname = suite_
    strcat(funcname, name);                       //funcame = suite_name
    void *handle = dlopen("./sample", RTLD_LAZY); // open program exec
    testfw_func_t func = dlsym(handle, funcname);
    //  dlclose(handle); // SEGFAULT  HERE
    if (func == NULL)
    {
        fprintf(stderr, "func pointer is NULL, impossible to register\n");
        exit(EXIT_FAILURE);
    }
    free(funcname); //free malloc funcname
    return testfw_register_func(fw, suite, name, func);
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register a suite in it\n");
        exit(EXIT_FAILURE);
    }
    else if (suite == NULL)
    {
        fprintf(stderr, "suite pointer is NULL, impossible to register a suite without a test suite\n");
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
    strcat(cmd, fw->separator);                    // grep <suite>_*
    strcat(cmd, " | cut -d \"_\" -f 2"); // to get only the <name> (without <suite>_)

    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "popen ended with an error\n");
        exit(EXIT_FAILURE);
    }

    while (getline(&name, &len, fp) != -1)
    {
        name[strlen(name) - 1] = 0; //remove \n at the end of the name
        testfw_register_symb(fw, suite, name);
        size++;
    }

    if (pclose(fp) == -1)
    {
        fprintf(stderr, "pclose ended with an error\n");
        exit(EXIT_FAILURE);
    }

    free(cmd);
    return size;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    // check pointers

    return 0;
}
