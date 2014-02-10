/* Base: http://www.jera.com/techinfo/jtns/jtn002.html */

#define MU_PASSED 1
#define MU_ERR 0

#define mu_assert(message, test) do { if (!(test)) { mu_fail(message); } } while (0)
#define mu_perror(message) printf("Error %s: %s\n", __FUNCTION__, message);
#define mu_psyserror(message) do { perror(message); mu_perror(message); } while (0)
#define mu_fail(message) do { mu_perror(message); return MU_ERR; } while(0)
#define mu_end return MU_PASSED
#define mu_sysfail(message) do { perror(message); mu_fail(message); } while(0)
#define mu_assert_eq(actual, expected, message) do { char _meqstr[200]; sprintf(_meqstr, "%s: expected %d, got %d.", message, expected, actual); mu_assert(_meqstr, expected == actual); } while(0)
#define mu_run_test(test) do { int result = test(); tests_run++; tests_passed += result; } while (0)
#define mu_cleanup_fail(label, message) do { mu_perror(message); retval = MU_ERR; goto label; } while(0)
#define mu_cleanup_sysfail(label, message) do { mu_psyserror(message); retval = MU_ERR; goto label; } while(0)
