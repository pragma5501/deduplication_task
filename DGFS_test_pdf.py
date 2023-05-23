import os

count_error = 0

# Famous letter from Dijkstra, Goto statement considered harmful
url = "https://homepages.cwi.nl/~storm/teaching/reader/Dijkstra68.pdf"
target = "test.pdf"

if os.path.isfile("test.pdf") == False:
    os.system("wget -O " + target + " " + url)

os.system("rm test.bin")
os.system("./DGFS create test.bin")
os.system("cp test.pdf target.pdf")
os.system("cp test.pdf target_tmp.pdf")

os.system("./DGFS test.bin add test.pdf")
os.system("./DGFS test.bin ls")
size_before = os.path.getsize("test.bin")

target = "target_tmp.pdf"
for i in range(16):
    tmp = "target_" + str(i) + ".pdf"
    os.system("mv " + target + " " + tmp)
    os.system("./DGFS test.bin add " + tmp)
    os.system("./DGFS test.bin extract " + tmp + " target_" + str(i) + ".pdf")
    diff = os.system("diff target.pdf " + " target_" + str(i) + ".pdf")
    if diff != 0:
        print("File recovery failed!")
        count_error += 1
    target = tmp

os.system("./DGFS test.bin ls")
size_after = os.path.getsize("test.bin")

print("Error:\t", count_error)
print("DGFS size before test:\t", size_before)
print("DGFS size after test:\t", size_after)

# Should consider data block for indirect pointer
if size_before + 4096*16 > size_after:
    print("Deduplication test passed!")
else:
    print("Deduplication test failed!")