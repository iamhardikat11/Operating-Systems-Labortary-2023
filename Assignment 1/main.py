groupno = 11
files = []
for i in [1, 2, 4, 7, 9]:
    files.append(f"./Q{i}/Assgn1_{i}_{11}.sh")
wc = 0
for file in files:
    with open(file) as f:
        content = f.read().replace('=', ' = ').replace('|', ' | ').replace(';', ' ; ').replace(',', ' , ').replace('>', ' > ').replace('<', ' < ')
        wc += len(content.split())
        print(file, len(content.split()))
print("Total: ", wc)