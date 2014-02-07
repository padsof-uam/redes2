#include "daemon.h"
#include "daemonize.h"
#include "testmacros.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#define SEMKEY_A 2561
#define SEMKEY_B 1623
#define BUF_SIZE 500

/* BEGIN TESTS */
static int _get_parent_proc(pid_t pid)
{
    FILE *p;
    char cmd[100];
    char line[BUF_SIZE];

    sprintf(cmd, "ps -o ppid= -p %d", pid);
    p = popen(cmd, "r");

    if (fgets(line, BUF_SIZE, p))
        return atoi(line);
    else
        return -1;
}

int t_hangs_on_init()
{
    pid_t child_pid, secchild_pid;
    int exit_status;
    int semid;
    int status;
    struct sembuf sb;
    int child_ppid;
    int parent_proc;
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
            printf("YujuHijoReportando\n");
            /* Esperamos hasta que nos digan que nos podemos cerrar. */
            sb.sem_flg = 0;
            sb.sem_num = 0;
            sb.sem_op = -1;

            semop(semid, &sb, 1);

            exit(EXIT_SUCCESS);
        }
        else
        {   
            printf("adfsklhj\n");
            exit(secchild_pid);
        }
    }
    else
    {
        waitpid(child_pid, &status, 0);

        if (WIFEXITED(status))
            parent_proc = WEXITSTATUS(status);
        else
            perror("exit fail");

        getchar();

        /* Le decimos al hijo que ya puede salir. */
        sb.sem_num = 0;
        sb.sem_op = 1;
        sb.sem_flg = 0;

        semop(semid, &sb, 1);

        if (semctl(semid, 0, IPC_RMID, NULL) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    mu_assert_eq(parent_proc, 1, "Process doesn't hanmg on init");

    mu_end;
}

static int _has_fds_open(pid_t pid)
{
    FILE *p;
    char cmd[100];
    char line[BUF_SIZE];
    int fd_num = 0;

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
    int status;
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

int daemon_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin daemon suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_hangs_on_init);
    mu_run_test(t_no_fds_open);

    /* END TEST EXEC */
    printf("End daemon suite. %d/%d\n", tests_passed, tests_run);

    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}
