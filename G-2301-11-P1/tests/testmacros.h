/* Base: http://www.jera.com/techinfo/jtns/jtn002.html */

#define mu_assert(message, test) do { if (!(test)) { mu_fail(message); } } while (0)
#define mu_fail(message) do { printf("Error %s:%d: %s\n", __FILE__, __LINE__, message); return 0; } while(0)
#define mu_end return 1
#define mu_sysfail(message) do { perror(message); mu_fail(message); } while(0)
#define mu_assert_eq(actual, expected, message) do { char _meqstr[200]; sprintf(_meqstr, "%s: expected %d, got %d.", message, expected, actual); mu_assert(_meqstr, expected == actual); } while(0)
#define mu_run_test(test) do { int result = test(); tests_run++; tests_passed += result; } while (0)

