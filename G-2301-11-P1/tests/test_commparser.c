#include "test_commparser.h"
#include "testmacros.h"
#include "commparser.h"
#include <stdio.h>

const char* _cmds[] = { "cmd1", "cmd2", "cmd3"};

static int _test_func(void* data)
{
	return *(int*)data;
}

/* BEGIN TESTS */
int t_parse_exec_command__findsmatch_nofunc__noexec() {
	char *cmd = "cmd2";
	cmd_action funcs[] = { _test_func, NULL, NULL };
	int result;

	result = parse_exec_command(cmd, _cmds, funcs, 3, NULL);

	mu_assert_eq(result, -1, "");
	mu_end;
}
int t_parse_exec_command__findsmatch__returns_func_retval() {
	char *cmd = "cmd1";
	cmd_action funcs[] = { _test_func, NULL, NULL };
	int result;
	int arg = 167;
	result = parse_exec_command(cmd, _cmds, funcs, 3, &arg);

	mu_assert_eq(result, arg, "");
	mu_end;
}
int t_parse_exec_command__nonexisting__noexec() {
	char *cmd = "ashjkd";
	cmd_action funcs[] = { _test_func, NULL, NULL };
	int result;

	result = parse_exec_command(cmd, _cmds, funcs, 3, NULL);

	mu_assert_eq(result, -1, "");
	mu_end;
}
int t_parse_exec_command__nullstr__noexec() {
	char *cmd = NULL;
	cmd_action funcs[] = { _test_func, NULL, NULL };
	int result;

	result = parse_exec_command(cmd, _cmds, funcs, 3, NULL);

	mu_assert_eq(result, -1, "");
	mu_end;	
}
int t_parse_command__nullstr_returnsM1() {
	char *cmd = NULL;
	int result;

	result = parse_command(cmd, _cmds, 3);

	mu_assert_eq(result, -1, "");
	mu_end;
}
int t_parse_command__unknowncmd_returnsM1() {
	char *cmd = "cmd4";
	int result;

	result = parse_command(cmd, _cmds, 3);

	mu_assert_eq(result, -1, "");
	mu_end;
}
int t_parse_command__correct_cmd() {
	char *cmd = "cmd1";
	int result;

	result = parse_command(cmd, _cmds, 3);

	mu_assert_eq(result, 0, "");
	mu_end;
}

/* END TESTS */

int test_commparser_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_commparser suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_parse_exec_command__findsmatch_nofunc__noexec);
	mu_run_test(t_parse_exec_command__findsmatch__returns_func_retval);
	mu_run_test(t_parse_exec_command__nonexisting__noexec);
	mu_run_test(t_parse_exec_command__nullstr__noexec);
	mu_run_test(t_parse_command__unknowncmd_returnsM1);
	mu_run_test(t_parse_command__nullstr_returnsM1);
	mu_run_test(t_parse_command__correct_cmd);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_commparser suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_commparser suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}
