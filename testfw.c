#define _GNU_SOURCE
#define TEST_SIZE 100
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <link.h>

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

    int size = strlen(name) + strlen(suite) + strlen(fw->separator) + 1;
    //int size = 100;
    char *funcname = (char *)malloc(size); // final function name with size of name+_+suite
    strcpy(funcname, suite);               //funcname = suite
    strcat(funcname, fw->separator);       //funcname = suite_
    strcat(funcname, name);                //funcame = suite_name

    void *handle = dlopen(NULL, RTLD_LAZY); // open program exec

    if (!handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    dlerror(); /* Clear any existing error */

    testfw_func_t func = (testfw_func_t)dlsym(handle, funcname);

    dlclose(handle);
    if (func == NULL)
    {
        fprintf(stderr, "func pointer is NULL, impossible to register\n");
        exit(EXIT_FAILURE);
    }
    free(funcname); //free malloc funcname
    return testfw_register_func(fw, suite, name, *func);
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
    strcat(cmd, fw->separator);          // grep <suite>_*
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
    printf("RUN_ALL\n");
    char *message = (char *)malloc(100);
    int nb_tests_failed = 0;

    // check pointers
    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register a suite in it\n");
        exit(EXIT_FAILURE);
    }
    /*Cette fonction retourne le nombre de tests qui échouent ou zéro en cas de succès. En outre, elle affiche sur sa sortie standard (sauf en mode silent) le résultat des tests avec le format suivant, illustré par quelques tests fournis dans sample.c :

[SUCCESS] run test "test.hello" in 0.47 ms (status 0)
[TIMEOUT] run test "test.infiniteloop" in 2000.19 ms (status 124)
[KILLED] run test "test.segfault" in 0.14 ms (signal "Segmentation fault")
[FAILURE] run test "test.failure" in 0.40 ms (status 1)


* FAILURE: return EXIT_FAILURE or 1 (or any value different of EXIT_SUCCESS)
* KILLED: killed by any signal (SIGSEGV, SIGABRT, ...)
* TIMEOUT: after a time limit, return an exit status of 124 (following the convention used in *timeout* command)
*/

    /*
  * *timeout* : la commande est interrompue si elle dépasse une limite de temps (et elle renvoie le status 124) ; 
  * *logfile* : la sortie standard du test (et sa sortie d'erreur) sont redirigées dans un fichier ;
  * *cmd* : la sortie standard du test (et sa sortie d'erreur) sont redirigés sur l'entrée standard d'une commande shell grâce aux fonctions popen/pclose (cf. man).*/

    //https://stackoverflow.com/questions/840501/how-do-function-pointers-in-c-work
    if (mode == TESTFW_FORKS)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            printf("SON\n");
            for (int i = 0; i < fw->tests_length; i++)
            {
                printf("LAUNCHING FUNCTION\n");

                int status = 0;
                testfw_func_t functionPtr = fw->tests[i].func;

                Dl_info d;
                dladdr(functionPtr, &d);
                printf("%s\n", d.dli_sname);

                status = (*functionPtr)(argc, argv);

                printf("DONE\n");

                if (!fw->silent)
                {
                    if (status == EXIT_SUCCESS)
                        message = "SUCCESS";
                    else
                    {
                        if (status == EXIT_FAILURE) // AND OTHER VALUES
                            message = "FAILURE";
                        else
                            message = "ELSE";

                        nb_tests_failed++;
                    }

                    printf("[%s] run test \"%s.%s\" in %d ms (status %d)\n", message, fw->tests[i].suite, fw->tests[i].name, -1, status);
                }
            }
        }
       // wait(NULL);
        waitpid(pid, NULL, NULL);
        printf("DAD\n");
    }
    else if (mode == TESTFW_NOFORK)
    {
        //same thing
    }
    free(message);

    return nb_tests_failed;
}
