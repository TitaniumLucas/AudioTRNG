import sys


def parse(line):
    line = line.replace("*", "").replace("\t", " ")
    parts = list(filter(bool, line.split(" ")))
    assert len(parts) == 13
    return [part.strip() for part in parts][10:]


def parse_ratio(ratio):
    x, y = ratio.split("/")
    return int(x) / int(y)


def main():
    tests = {}

    with open(sys.argv[1], "r") as f:
        for line in f.readlines():
            passed = "*" not in line
            p, prop, name = parse(line)
            p = float(p)
            ratio = parse_ratio(prop)

            if name in tests:
                oldp, oldratio, _ = tests[name]
                if ratio < oldratio:
                    tests[name] = (p, ratio, passed)
            else:
                tests[name] = (p, ratio, passed)
    
    for name, (p, ratio, passed) in tests.items():
        print(name, p, ratio, passed)


if __name__ == "__main__":
    main()
