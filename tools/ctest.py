#!/usr/bin/python

import sys

def addsuitetoglob(glob_test_file, suite_name):
	globtest = open(glob_test_file, "r")
	globtest_contents = globtest.readlines()
	globtest.close()
	globtest = open(glob_test_file, "w")

	globtest.write('#include "test_{0}.h"\n'.format(suite_name))

	in_test_region = False

	for line in globtest_contents:
		if in_test_region:
			globtest.write("\tif(include_test(\"{0}\", argc, argv))\n\t\trun += test_{0}_suite(&error, &success);\n".format(suite_name))
			in_test_region = False
		globtest.write(line)

		if "/* BEGIN TEST REGION */" in line:
			in_test_region = True

	globtest.close()

def addtesttosuite(suite_file, testname):
	suite = open(suite_file, "r")
	suite_contents = suite.readlines()
	suite.close()
	suite = open(suite_file, "w")

	in_test_region = False
	in_test_exec_region = False

	for line in suite_contents:	
		if in_test_region:
			suite.write("int t_{0}() {{\n\n\tmu_fail(\"Not implemented\");\n\tmu_end;\n}}\n".format(testname))
			in_test_region = False
		elif in_test_exec_region:
			suite.write("\tmu_run_test(t_{0});\n".format(testname))
			in_test_exec_region = False

		suite.write(line)

		if "/* BEGIN TESTS */" in line:
			in_test_region =  True
		elif "/* BEGIN TEST EXEC */" in line:
			in_test_exec_region = True

	suite.close()

if sys.argv[1] == "addtesttosuite":
	addtesttosuite(sys.argv[2], sys.argv[3])
elif sys.argv[1] == "addsuitetoglob":
	addsuitetoglob(sys.argv[2], sys.argv[3])
else:
	print "It wasn't that hard to enter a correct argument, wasn't it?"

