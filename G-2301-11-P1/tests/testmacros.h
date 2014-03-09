/* Base: http://www.jera.com/techinfo/jtns/jtn002.html */

#include "termcolor.h"

#define MU_PASSED 1
#define MU_ERR 0

#define mu_assert(message, test) do { \
	if (!(test)) { \
			mu_fail(message); \
		} \
	} while (0)

#define mu_perror(message) printf(TBOLD TRED "ERR\nFail at line %d: " TRESET "%s\n\n", __LINE__, message);

#define mu_psyserror(message) do { perror(message); mu_perror(message); } while (0)

#define mu_fail(message) do { mu_perror(message); return MU_ERR; } while(0)

#define mu_end do { \
	printf(TGREEN "OK\n" TRESET); \
	return MU_PASSED; \
} while(0)

#define mu_sysfail(message) do { perror(message); mu_fail(message); } while(0)

#define mu_assert_eq(actual, expected, message) do { \
	char _meqstr[200]; \
	sprintf(_meqstr, "%s: expected %d, got %d.", message, expected, actual); \
	mu_assert(_meqstr, (expected) == (actual)); \
} while(0)

#define mu_assert_streq(actual, expected, message) do { \
	char _meqstr[200]; \
	sprintf(_meqstr, "%s: expected %s, got %s.", message, expected, actual); \
	mu_assert(_meqstr, strcmp((expected), (actual)) == 0); \
} while(0)

#define mu_run_test(test) do { int result; \
	printf(TBOLD #test TRESET " "); \
	result = test(); \
	tests_run++; \
	tests_passed += result; \
} while (0)

#define mu_cleanup_fail(label, message) do { mu_perror(message); retval = MU_ERR; goto label; } while(0)
#define mu_cleanup_sysfail(label, message) do { mu_psyserror(message); retval = MU_ERR; goto label; } while(0)
