// Base: http://www.jera.com/techinfo/jtns/jtn002.html

#define mu_assert(message, test) do { if (!(test)) { printf("Error %s:%s: %s\n", __FILE__, __LINE__, message); return 0; } while (0)
#define mu_end return 1
#define mu_run_test(test) do { int result = test(); tests_run++; tests_passed += result; } while (0)

