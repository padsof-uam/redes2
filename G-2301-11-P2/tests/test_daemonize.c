#include <stdio.h>
#include "test_daemonize.h"
#include "daemonize.h"
#include "testmacros.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#define SEMKEY_A 2561
#define SEMKEY_B 1623
#define BUF_SIZE 500

#define PID_FILE "/tmp/pidfile_test_r2"

/* BEGIN TESTS */
int t_hangs_on_init()
{
    pid_t child_pid, secchild_pid;
    int semid;
    struct sembuf sb;
    int child_ppid = -1;
    FILE* f = NULL;
        
    semid = semget(SEMKEY_A, 1, IPC_CREAT | IPC_EXCL | 0666);

    if (semid == -1)
    {
        mu_sysfail("semget");
    }

    child_pid = fork();

    if (child_pid < 0)
    {
        if (semctl(semid, 0, IPC_RMID, NULL) == -1)
        {
            perror("semctl");
            exit(1);
        }

        mu_sysfail("fork");
    }


    if (child_pid == 0)
    {
        secchild_pid = unlink_proc();

        if (secchild_pid == 0)
        {   
            f = fopen(PID_FILE, "w+");

            if(f)
            {
                fprintf(f, "%d", getppid());
                fclose(f);
            }
            else
            {
                perror("fopen");
            }

            /* Le decimos al proceso de test que ya puede escribir. */
            sb.sem_flg = 0;
            sb.sem_num = 0;
            sb.sem_op = 1;

            semop(semid, &sb, 1);

            exit(EXIT_SUCCESS);
        }
        else
        {   
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        /* Esperamos hasta que escriba PID en fichero */
        sb.sem_num = 0;
        sb.sem_op = -1;
        sb.sem_flg = 0;

        semop(semid, &sb, 1);

        f = fopen(PID_FILE, "r");
        
        if(f)
        { 
            fscanf(f, "%d", &child_ppid);
            fclose(f);
            remove(PID_FILE);
        }

        if (semctl(semid, 0, IPC_RMID, NULL) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    if(child_ppid == -1)
    {
        mu_sysfail("fopen");
    }

    mu_assert_eq(child_ppid, 1, "Process doesn't hang on init");

    mu_end;
}

static int _has_fds_open(pid_t pid)
{
    FILE *p;
    char cmd[100];
    char line[BUF_SIZE];

    sprintf(cmd, "lsof -p %d | awk '{print $4}' | egrep \"\\d+[urw]\"", pid);
    p = popen(cmd, "r");

    if (fgets(line, BUF_SIZE, p) && strlen(line) > 0)
        return 1;
    else
        return 0;
}

int t_no_fds_open()
{
    pid_t child_pid;
    int semid;
    struct sembuf sb;

    semid = semget(SEMKEY_B, 2, IPC_CREAT | IPC_EXCL | 0666);

    if (semid == -1)
    {
        mu_sysfail("semget");
    }

    child_pid = fork();

    if (child_pid < 0)
    {
        if (semctl(semid, 0, IPC_RMID, NULL) == -1)
        {
            perror("semctl");
            exit(1);
        }

        mu_sysfail("fork");
    }


    if (child_pid == 0)
    {
        close_open_fds();

        /* Indicamos a padre que ya puede ver qué fd tenemos abiertos. */
        sb.sem_flg = 0;
        sb.sem_num = 0;
        sb.sem_op = 1;

        semop(semid, &sb, 1);

        /* Esperamos hasta que nos digan que nos podemos cerrar. */
        sb.sem_flg = 0;
        sb.sem_num = 1;
        sb.sem_op = -1;

        semop(semid, &sb, 1);

        exit(EXIT_SUCCESS);
    }
    else
    {
        /* Esperamos hasta que el hijo cierre sus fds. */
        sb.sem_flg = 0;
        sb.sem_num = 0;
        sb.sem_op = -1;

        semop(semid, &sb, 1);
        mu_assert("Process didn't close all fd", !(_has_fds_open(child_pid)));

        /* Le decimos al hijo que ya puede salir. */
        sb.sem_num = 1;
        sb.sem_op = 1;

        semop(semid, &sb, 1);

        if (semctl(semid, 0, IPC_RMID, NULL) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    mu_end;
}

/* END TESTS */

int test_daemonize_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_daemonize suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_hangs_on_init);
    mu_run_test(t_no_fds_open);

    /* END TEST EXEC */
    if(tests_passed == tests_run)
        printf("End test_daemonize suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_daemonize suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);

    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}
