#!/bin/sh

# Usage: test_logmich.sh EXENAME OUTPUTPREFIX ABSSOURCEDIR

set -e

TEST_STDOUT="$(mktemp)"
TEST_STDERR="$(mktemp)"

echo "Running $1..."

"$1" 2> "${TEST_STDERR}" > "${TEST_STDOUT}"

# strip the absolute path in the test output
sed -i "s#${3}/##" "${TEST_STDERR}" "${TEST_STDOUT}"

if cmp -s "${TEST_STDOUT}" "${2}.stdout"; then
  echo "stdout ok"
else
  echo "stdout mismtach:"
  diff -u "${2}.stdout" "${TEST_STDOUT}"
  exit 1
fi

if cmp -s "${TEST_STDERR}" "${2}.stderr"; then
  echo "stderr ok"
else
  echo "stderr mismtach:"
  diff -u "${2}.stderr" "${TEST_STDERR}"
  exit 1
fi

echo "$1 test finished successfully"

exit 0

# EOF #

