EXTRA_DIST = \
  bIsMatrix.hex \
  error.hex \
  matheTest.hex \
  noSolution.hex \
  symbols.hex \
  undetermined.hex \
  vectorWithArity3.hex \
  tests/run-dlvhex-tests.sh \
  tests/mathematicaplugintests.test \
  tests/bIsMatrix.out \
  tests/matheTest.out \
  tests/noSolution.out \
  tests/symbols.out \
  tests/undetermined.out \
  tests/vectorWithArity3.out

TESTS = tests/run-dlvhex-tests.sh
TESTS_ENVIRONMENT = DLVHEX=dlvhex TESTDIR=$(top_srcdir)/examples/tests PARAMETERS="-s --plugindir=$(top_builddir)/src"
