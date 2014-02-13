echo f;
(sleep 3; echo fail); (sleep 2; echo not);
echo first > test.txt; echo second > test.txt; echo abcdefg | tr a A > out.txt; (sleep 3; cat < out.txt);(sleep 2; cat < out.txt | tr A-Z a-z) ; echo should not print > out.txt
