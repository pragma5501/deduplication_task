import os

count_error = 0

os.system("man printf > test.txt")
target = "test.txt"

os.system("rm test.bin")
os.system("./DGFS create test.bin")
os.system("cp test.txt target.txt")
os.system("cp test.txt target_tmp.txt")

os.system("./DGFS test.bin add test.txt")
os.system("./DGFS test.bin ls")
size_before = os.path.getsize("test.bin")

target = "target_tmp.txt"
for i in range(100):
    tmp = "target_" + str(i) + ".txt"
    os.system("mv " + target + " " + tmp)
    os.system("./DGFS test.bin add " + tmp)
    os.system("./DGFS test.bin extract " + tmp + " target_" + str(i) + ".txt")
    diff = os.system("diff target.txt " + " target_" + str(i) + ".txt")
    if diff != 0:
        print("File recovery failed!")
        count_error += 1
    target = tmp

os.system("./DGFS test.bin ls")
size_after = os.path.getsize("test.bin")

print("Error:\t", count_error)
print("DGFS size before test:\t", size_before)
print("DGFS size after test:\t", size_after)
if size_before == size_after:
    print("Deduplication test passed!")
else:
    print("Deduplication test failed!")