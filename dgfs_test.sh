

make
rm dgfs_result.txt
rm DGFS.bin
./DGFS create DGFS.bin >> dgfs_result.txt
./DGFS create DGFS.bin >> dgfs_result.txt

echo >> dgfs_result.txt


rm hello.txt
rm foo.txt
rm bar.txt

echo "Hello world!" >> hello.txt
echo "Foobar" >> foo.txt
echo "Foobar" >> bar.txt



./DGFS DGFS.bin add hello.txt >> dgfs_result.txt
echo >> dgfs_result.txt

./DGFS DGFS.bin add foo.txt >> dgfs_result.txt
echo >> dgfs_result.txt


./DGFS DGFS.bin add bar.txt >> dgfs_result.txt
echo >> dgfs_result.txt

./DGFS DGFS.bin add bar.txt >> dgfs_result.txt
echo >> dgfs_result.txt

./DGFS DGFS.bin add os.bin >> dgfs_result.txt
echo >> dgfs_result.txt

./DGFS DGFS.bin ls >> dgfs_result.txt
echo >> dgfs_result.txt


./DGFS DGFS.bin remove hello.txt >> dgfs_result.txt
echo >> dgfs_result.txt

./DGFS DGFS.bin ls >> dgfs_result.txt
echo >> dgfs_result.txt