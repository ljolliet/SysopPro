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
    for(int i = 0; i< TEST_SIZE; i++)
        fw->tests[i] = NULL;
    fw->tests_length = 0;
    return fw; // GOOD ?
}

void testfw_free(struct testfw_t *fw)
{
 if(fw == NULL){

 }
 else{
  free(fw);
  fw = NULL; // TO CHECK
 }
}

int testfw_length(struct testfw_t *fw)
{

    if(fw == NULL)
        return -1; // smthg else ?
    else{
        printf("length : %d\n", fw->tests_length);
        return fw->tests_length;
    }
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{

    printf("k : %d\n", k); 
    if(k>=TEST_SIZE || k<0 || fw == NULL)    // to avoid a crash
        return NULL; // smthg else ?
    
    printf("get : %s.%s", fw->tests[k]->suite, fw->tests[k]->name); 
       
    return fw->tests[k];
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    // check pointers

    struct test_t * test = (struct test_t *) malloc(sizeof(struct test_t));
    test->suite = suite;
    test->name = name;
    test->func = func;

    fw->tests[fw->tests_length] = test; // TODO CHECK SIZE
    printf("register : %s.%s at %d\n" , suite, name, fw->tests_length);
    fw->tests_length++;

    return test; 
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
        // check pointers
//typedef int (*testfw_func_t)(int argc, char *argv[]);


    char * underscore = "_";
    char * funcname = (char *) malloc(strlen(name)+strlen(suite)+strlen(underscore));   // final function name with size of name+_+suite
    strcpy(funcname,suite); //funcname = suite
    strcat(funcname,underscore);    //funcname = suite_
    strcat(funcname, name); //funcame = suite_name
    void * handle = dlopen(fw->program, RTLD_LAZY); // open program exec
    struct testfw_func_t * func = (struct testfw_func_t *)dlsym(handle, funcname);
    dlclose(handle);  // SEGFAULT IS HERE  
    if(func == NULL)
        printf("func null\n");

    free(funcname); //free malloc funcname
    struct test_t *t = testfw_register_func(fw,suite,name,func); 
    return t;
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    char * name = NULL;
    size_t len = 0;
    ssize_t read;

            // check pointers
    
    char * cmd = (char *) malloc(100);   // CHECK SIZE
    strcpy(cmd,"nm "); // cmd = nm
    strcat(cmd, fw->program); // cmd = nm <program>
    strcat(cmd," | cut -d ' ' -f 3 | grep ^"); // ...
    strcat(cmd,suite);
    strcat(cmd,"_");
    strcat(cmd," | cut -d \"_\" -f 2"); // to get only the <name>




    FILE  * fp =  popen(cmd,"r");

     while (read = getline(&name, &len, fp) != -1) {
        testfw_register_symb(fw,suite,name);
    }
    int status = pclose(fp);
    free(cmd);

    return 0;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
        // check pointers

    return 0;
}
