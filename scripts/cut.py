import sys

fn = sys.argv[1]
size = int(sys.argv[2])

with open(fn, "rb") as fi:
    with open("cut-" + fn, "wb") as fo:
        data: bytes = fi.read()
        fo.write(data[:size])

