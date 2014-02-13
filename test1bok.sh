#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
echo BEGIN;
echo THIS SHOULD PRINT || echo THIS SHOULD NOT PRINT
echo THIS SHOULD PRINT && echo THIS SHOULD PRINT TOO
#echo THIS SHOULD NOT PRINT
echo qwertytuiopasdfghjklzxcvbnm > new.txt
cat < new.txt | tr a-z A-Z
echo 1 && (echo 2 || echo 3) && echo 4;
echo 1;
#COMMENT IS HERE
echo 2 echo 3;
#COMMENT IS HERE
echo 4
#COMMENT IS HERE
echo 567890;
EOF

cat >test.exp <<'EOF'
BEGIN
THIS SHOULD PRINT
THIS SHOULD PRINT
THIS SHOULD PRINT TOO
QWERTYTUIOPASDFGHJKLZXCVBNM
1
2
4
1
2 echo 3
4
567890
EOF

../timetrash test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
