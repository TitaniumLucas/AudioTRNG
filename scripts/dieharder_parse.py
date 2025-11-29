import sys


def parse(line):
    parts = line.split("|", maxsplit=5)
    assert len(parts) == 6
    return [part.strip() for part in parts]


def main():
    assert len(sys.argv) > 1

    tests = {}

    with open(sys.argv[1]) as f:
        for line in f.readlines():
            name, _, _, _, p, out = parse(line)
            p = float(p)

            if name in tests:
                oldp, oldout = tests[name]
                oldd = abs(oldp - 0.5)
                newd = abs(p - 0.5)

                if newd > oldd:
                    tests[name] = (p, out)
            else:
                tests[name] = (p, out)

    for name, (p, out) in tests.items():
        name = name.split("_", maxsplit=1)[1].title().replace("_", " ")
        print(f"{name} & {p} & {out.title()} \\\\")



if __name__ == "__main__":
    main()
