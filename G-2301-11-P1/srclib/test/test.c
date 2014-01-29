#include <stdio.h>
#include "errors.h"
#include "internal.h"

int test(int a) {
	printf("In test!!\n");
	return a + return_this_int();
}
