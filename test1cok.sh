#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
echo f;
(sleep 3; echo fail); (sleep 2; echo not);
echo first > ttest.txt; echo second > ttest.txt; echo abcdefg | tr a A > out.txt; (sleep 3; cat < out.txt);(sleep 2; cat < out.txt | tr A-Z a-z) ; echo should not print > out.txt
EOF

cat >test.exp <<'EOF'
f
not
fail
abcdefg
Abcdefg
EOF

../timetrash -t  test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
