#define _GNU_SOURCE
#define TEST_SIZE 100
#define NSIGNORT 32

#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <setjmp.h>
#include <fcntl.h>

//check non-necessary includes
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


enum test_statut_t
{
    SUCCESS,
    FAILURE,
    TIMEOUT,
    KILLED,
    DEFAULT
};

struct test_statement_t
{
    enum test_statut_t statut_test;
    int returned_statement;
    double duration;
    int signal_caught;
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

    char *cmd = (char *)malloc(200);            // CHECK SIZE
    strcpy(cmd, "nm --defined-only ");          // cmd = nm
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

jmp_buf buf;
int sig_caught = -1;
enum test_statut_t statut_test = DEFAULT;

static void expire(int sig)
{
    statut_test = TIMEOUT;
    siglongjmp(buf, 1);
}

static void handler(int sig)
{
    sig_caught = sig;
    statut_test = KILLED;
    siglongjmp(buf, 1);
}

static int testfw_run_display(struct testfw_t *fw, struct test_statement_t statement_test, char * suite, char * name, int nb_test_failed)
{

    switch (statement_test.statut_test)
    {
    case SUCCESS:
        printf("[SUCCESS] run test \"%s.%s\" in %f ms (status %d)\n", suite, name, statement_test.duration, statement_test.returned_statement);
        break;
    case FAILURE:
        printf("[FAILURE] run test \"%s.%s\" in %f ms (status %d)\n", suite, name, statement_test.duration, statement_test.returned_statement);
        nb_test_failed++;
        break;
    case TIMEOUT:
        printf("[TIMEOUT] run test \"%s.%s\" in %f ms (status 124)\n", suite, name, statement_test.duration);
        nb_test_failed++;
        break;
    case KILLED:
        printf("[KILLED] run test \"%s.%s\" in %f ms (signal \"%s\")\n", suite, name, statement_test.duration,  strsignal(statement_test.signal_caught));
        nb_test_failed++;
        break;
    default:
        fprintf(stderr, "Unknown status return by a function\n");
        exit(EXIT_FAILURE);
        break;
    }
    return nb_test_failed;
}

static int testfw_sequential_run(struct testfw_t *fw, int argc, char *argv[])
{
    int nb_tests_failed = 0; 

    for (int i = 0; i < fw->tests_length; i++)
    {
        int status = 0;
        struct timeval start, end;
        testfw_func_t functionPtr = fw->tests[i].func;

        struct sigaction act;
        act.sa_flags = 0;
        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);

        for (int i = 1; i < NSIGNORT; i++)
        {
            sigaction(i, &act, NULL);
        }

        struct sigaction s;
        s.sa_flags = 0;
        s.sa_handler = expire;
        sigemptyset(&s.sa_mask);
        sigaction(SIGALRM, &s, NULL);

        if (sigsetjmp(buf, 1) == 0)
        {
            alarm(fw->timeout);
            gettimeofday(&start, NULL);
            status = (functionPtr)(argc, argv);
            alarm(0);
            gettimeofday(&end, NULL);
            if (status == 0)
                statut_test = SUCCESS;
            else
            {
                statut_test = FAILURE;
                nb_tests_failed++;
            }
        }
        else
        {
            gettimeofday(&end, NULL);
            nb_tests_failed++;
        }

        double duration = (end.tv_sec - start.tv_sec) * 1000LL + (end.tv_usec - start.tv_usec) / 1000;

        if (!fw->silent){}
           // testfw_run_display(fw, duration, fw->tests[i].suite, fw->tests[i].name, status);
    }
    return nb_tests_failed;
}

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{

    if (fw == NULL)
    {
        fprintf(stderr, "testfw_t pointer is NULL, impossible to register a suite in it\n");
        exit(EXIT_FAILURE);
    }
    /*
  * *logfile* : la sortie standard du test (et sa sortie d'erreur) sont redirigées dans un fichier ;
  * *cmd* : la sortie standard du test (et sa sortie d'erreur) sont redirigés sur l'entrée standard d'une commande shell grâce aux fonctions popen/pclose (cf. man).
  * */
    int wstatus;
    pid_t pid;
    int p[2];
    pipe(p);
    struct test_statement_t statement_test;

    int nb_tests_failed = 0;
    switch (mode)
    {
    case TESTFW_FORKS:
        pid = fork();
        if (pid == 0)
        {
            //  nb_tests_failed = testfw_sequential_run(fw, argc, argv);
            struct timeval start, end;
            close(p[0]);
            int fd = open(fw->logfile, O_WRONLY | O_CREAT, 0644 ); //check logfile name
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);


            for (int i = 0; i < fw->tests_length; i++)
            {
                int status = 0;
                testfw_func_t functionPtr = fw->tests[i].func;

                struct sigaction act;
                act.sa_flags = 0;
                act.sa_handler = handler;
                sigemptyset(&act.sa_mask);
                for (int i = 1; i < NSIGNORT; i++)
                {
                    sigaction(i, &act, NULL);
                }

                struct sigaction s;
                s.sa_flags = 0;
                s.sa_handler = expire;
                sigemptyset(&s.sa_mask);
                sigaction(SIGALRM, &s, NULL);

                if (sigsetjmp(buf, 1) == 0)
                {
                    alarm(fw->timeout);
                    gettimeofday(&start, NULL);
                    status = (functionPtr)(argc, argv);
                    alarm(0);
                    gettimeofday(&end, NULL);

                    if (status == 0)
                        statut_test = SUCCESS;
                    else
                    {
                        statut_test = FAILURE;
                        nb_tests_failed++;
                    }
                }
                else
                {
                    gettimeofday(&end, NULL);
                    nb_tests_failed++;
                }
                double duration = (end.tv_sec - start.tv_sec) * 1000LL + (end.tv_usec - start.tv_usec) / 1000;

                statement_test.duration = duration;
                statement_test.returned_statement = status;
                statement_test.signal_caught = -1;
                statement_test.statut_test = statut_test;

                write(p[1], &statement_test, sizeof(statement_test));
            }
            close(p[1]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(p[1]);
            for (int i = 0; i < fw->tests_length; i++)
            {
                read(p[0], &statement_test, sizeof(statement_test));
                nb_tests_failed = testfw_run_display(fw, statement_test, fw->tests[i].suite, fw->tests[i].name, nb_tests_failed);
            }
            close(p[0]);
            waitpid(pid, &wstatus, NULL);
            if (wstatus != EXIT_SUCCESS)
                fprintf(stderr, "Error in the fork\n");
        }

        break;
    case TESTFW_NOFORK:
        return testfw_sequential_run(fw, argc, argv);
        break;
    case TESTFW_FORKP:
        //parallel fork as a BONUS
        break;
    default:
        fprintf(stderr, "Unknown mode, impossible to run\n");
        exit(EXIT_FAILURE);
    }

    return nb_tests_failed;
}
