import sys
from matplotlib import pyplot as plt


def analyze(data):
    byte_buckets = [ 0 for _ in range(256) ]
    
    for byte in data:
        byte_buckets[int(byte)] += 1

    results = list(enumerate(byte_buckets))

    print("Byte Frequencies")
    for byte, n in results:
        print(f"{byte:02x} ({bin(byte)}): {n}")

    bit_buckets = [ 0 for _ in range(8) ]

    ones, zeroes = 0, 0
    for byte, n in results:
        for i in range(8):
            if byte & 0x1:
                bit_buckets[i] += n
                ones += n
            else:
                zeroes += n

            byte >>= 1

    print("Bit Frequencies: ")
    for bit, n in enumerate(bit_buckets):
        print(f"{bit}: {n}")

    print(f"{zeroes} 0's, {ones} 1's")

    plt.plot(range(256), byte_buckets)
    plt.show()


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} [filename]")
    filename = sys.argv[1]

    with open(filename, "rb") as file:
        data = file.read()
        analyze(data)


if __name__ == "__main__":
    main()
