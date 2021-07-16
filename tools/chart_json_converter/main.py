import json
import sys
import random
import os

if len(sys.argv) <= 1:
    print("please input file name")
    sys.exit(0)

name = sys.argv[1][:-5]
FPS = 30
def getType(s):
    if s == "Single":
        return 0
    elif s == "Long":
        return 1
    elif s == "Slide":
        return 2
    return 3

with open(sys.argv[1]) as f, open(f'{name}l.txt', 'w') as leftF, open(f'{name}r.txt', 'w') as rightF:
    data = json.load(f)
    left = []
    right = []
    for i in data:
        if i["type"] == "Note":
            val = (getType(i["note"]), int(i["time"] * FPS))
            if int(i["lane"]) <= 3:
                if left and left[-1][1] == val[1]:
                    continue
                left.append(val)
            elif int(i["lane"]) >= 5:
                if right and right[-1][1] == val[1]:
                    continue
                right.append(val)
            else:
                if random.random() < 0.5:
                    if left and left[-1][1] == val[1]:
                        continue
                    left.append(val)
                else:
                    if right and right[-1][1] == val[1]:
                        continue
                    right.append(val)
    for i in left:
        print(i[0], i[1], file=leftF)
    for i in right:
        print(i[0], i[1], file=rightF)
    # print("{", end='')
    # print(",".join(left))
    # print("}")
    # print(", {", end='')
    # print(",".join(right))
    # print("}")
os.system(f"./main {name}")