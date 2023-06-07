#!/bin/bash

echo "Test with simple text files" > test.txt
echo >> test.txt
echo "Test first method" >> test.txt
echo >> test.txt
rm hello_1.txt
rm hello_2.txt
echo "Case 1: Text file with same contents" >> test.txt
echo "Hello world!" > hello_1.txt
echo "Hello world!" > hello_2.txt
./file_dedup byte hello_1.txt hello_2.txt >> test.txt

echo >> test.txt
rm hello_1.txt
rm hello_2.txt
echo "Case 2: Text file with same size but different contents" >> test.txt
echo "Hello world!" > hello_1.txt
echo "Hello Hello!" >> hello_2.txt
./file_dedup byte hello_1.txt hello_2.txt >> test.txt

echo >> test.txt
rm hello_1.txt
rm hello_2.txt
echo "Case 3: Text file with same contents and aditional contents" >> test.txt
echo "Hello world!" > hello_1.txt
echo "Hello world!" >> hello_2.txt
echo "Hello world!" >> hello_2.txt
./file_dedup byte hello_1.txt hello_2.txt >> test.txt

echo >> test.txt
echo "Test second method" >> test.txt
echo >> test.txt
rm hello_1.txt
rm hello_2.txt
echo "Case 1: Text file with same contents" >> test.txt
echo "Hello world!" > hello_1.txt
echo "Hello world!" > hello_2.txt
./file_dedup hash hello_1.txt hello_2.txt >> test.txt
md5sum hello_1.txt >> text.txt
md5sum hello_2.txt >> text.txt

echo >> test.txt
rm hello_1.txt
rm hello_2.txt
echo "Case 2: Text file with same size but different contents" >> test.txt
echo "Hello world!" > hello_1.txt
echo "Hello Hello!" >> hello_2.txt
./file_dedup hash hello_1.txt hello_2.txt >> test.txt
md5sum hello_1.txt >> text.txt
md5sum hello_2.txt >> text.txt

echo >> test.txt
rm hello_1.txt
rm hello_2.txt
echo "Case 3: Text file with same contents and aditional contents" >> test.txt
echo "Hello world!" > hello_1.txt
echo "Hello world!" >> hello_2.txt
echo "Hello world!" >> hello_2.txt
./file_dedup hash hello_1.txt hello_2.txt >> test.txt

diff test_ans.txt test.txt
