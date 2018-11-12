#define _GNU_SOURCE
#define TEST_SIZE 50
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

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
 //   fw->tests = (struct test_t *)malloc(sizeof(struct test_t));
    for(int i = 0; i< TEST_SIZE; i++)
        fw->tests[i] = NULL;
    fw->tests_length = 0;
    return fw; // GOOD ?
}

void testfw_free(struct testfw_t *fw)
{
 // free(fw->tests);
  free(fw);
  fw = NULL; // TO CHECK
}

int testfw_length(struct testfw_t *fw)
{
    return fw->tests_length;
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
    printf("register func\n");

    struct test_t test = { .func = func, .suite = suite, .name = name};
    fw->tests[fw->tests_length] = fw; // TODO CHECK SIZE
    fw->tests_length++;
    return &test; 
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    printf("register symb\n");

    testfw_func_t * func = NULL;    
    char * underscore = "_";
    char * funcname = (char *) malloc(strlen(name)+strlen(suite)+strlen(underscore));   // final name with size of name+_+suite
    strcpy(funcname,suite); //funcname = suite
    strcat(funcname,underscore);    //funcname = suite_
    strcat(funcname, name); //funcame = suite_name
    void * handle = dlopen(fw->program, RTLD_LAZY); // open program exec
    func = dlsym(handle, funcname);    //find function in exec
    struct test_t test = { .func = func, .suite = suite, .name = name};
    fw->tests[fw->tests_length] = fw; // TODO CHECK SIZE
    fw->tests_length++;
    dlclose(handle);    // close exec
    free(funcname); //free malloc funcname
    
    return &test; 
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    printf("register suite\n");

    return 0;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    return 0;
}
