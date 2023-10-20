#!/usr/bin/python3 -B
"""test spiral algoritm"""


AREA = [[-1] * 16 for x in range(16)]

# based on: https://stackoverflow.com/questions/398299/looping-in-a-spiral
x = 0
y = 0
dx = 0
dy = -1
for i in range(256):
    try:
        AREA[y+7][x+7] = i
    except IndexError:
        pass
    if (x == y) or (x < 0 and x == -y) or (x > 0 and x == 1 - y):
        dx, dy = -dy, dx
    x += dx
    y += dy


for rowval in AREA:
    print(*("%3d" % v for v in rowval))
