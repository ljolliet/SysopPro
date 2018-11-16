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
    struct test_t tests[TEST_SIZE];
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
    for(int i =0;i<TEST_SIZE;i++){
        fw->tests[i].name = (char *)malloc(100);  // TODO FREEEEEEE
        fw->tests[i].suite = (char *)malloc(100);  // TODO FREEEEEEE

    }

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
        free(fw);  // free all the content of the structure
        fw = NULL; // TO CHECK
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
    else if (k < 0)
    {
        fprintf(stderr, "impossible to get a negative element\n");
        exit(EXIT_FAILURE);
    }
    else if (k >= TEST_SIZE)
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
        fprintf(stderr, "suite pointer is NULL, impossible to register a test without a test suite\n");
        exit(EXIT_FAILURE);
    }
    else if (name == NULL)
    {
        fprintf(stderr, "name pointer is NULL, impossible to register a test without a test name\n");
        exit(EXIT_FAILURE);
    }

    //CHECK FUNC ?
    
    strcpy(fw->tests[fw->tests_length].suite ,suite); // strcopy
    strcpy(fw->tests[fw->tests_length].name,name);
    fw->tests[fw->tests_length].func = func;


  
    if (fw->tests_length >= TEST_SIZE)
    {
        fprintf(stderr, "impossible to add an element out of the array\n");
        exit(EXIT_FAILURE);
    } 
    fw->tests_length++;
    return &fw->tests[fw->tests_length-1];
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
    char *underscore = "_";
    int size = strlen(name) + strlen(suite) + strlen(underscore);
    char *funcname = (char *)malloc(size); // final function name with size of name+_+suite
    strcpy(funcname, suite);                                                            //funcname = suite
    strcat(funcname, underscore);                                                       //funcname = suite_
    strcat(funcname, name);                                                            //funcame = suite_name
    void *handle = dlopen("./sample", RTLD_LAZY );                                      // open program exec
     testfw_func_t func = dlsym(handle, funcname);
        if(func == NULL)
            printf("func null");
   //  dlclose(handle); // SEGFAULT IS HERE

    free(funcname); //free malloc funcname
    return  testfw_register_func(fw, suite, name,func);
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
    strcat(cmd, "_");                    // grep <suite>_*
    strcat(cmd, " | cut -d \"_\" -f 2"); // to get only the <name> (without <suite>_)

    FILE *fp = popen(cmd, "r");

    while (getline(&name, &len, fp) != -1)
    {
        name[strlen(name)-1] = 0;
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
