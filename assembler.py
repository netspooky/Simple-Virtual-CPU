import struct
import sys


def usage():
    sys.exit("Usage: assembler.py <input file> <output file>")


def init_size_and_labels(file_parsed):
    length = 0
    label_addresses = {}
    for x in file_parsed:
        opcode = x[0].lower()
        if opcode == "hlt":
            length += 1  # 1 byte for opcode
        elif opcode == "mov":
            length += 1 + 2  # 1 byte opcode + 2 bytes of register numbers
        elif opcode == "xor":
            length += 1 + 2  # 1 byte opcode + 2 bytes of register numbers
        elif opcode == "add":
            length += 1 + 2  # 1 byte opcode + 2 bytes of register numbers
        elif opcode == "store":
            length += 1 + 1 + 4  # 1 byte opcode
            #                    + 1 byte of register number
            #                    + 4 bytes of int
        elif opcode == "push":
            length += 1 + 1  # 1 byte for opcode + 1 byte for register number
        elif opcode == "pop":
            length += 1 + 1  # 1 byte for opcode + 1 byte for register number
        elif opcode == "jmp":
            length += 1 + 4  # 1 byte for opcode
            #                + 4 bytes for offset
        elif opcode == "prnt":
            length += 1 + 1 + 1  # 1 byte for opcode
            #                    + 1 byte for register number
            #                    + 1 byte for ascii bool
        elif opcode == "call":
            length += 1 + 4  # 1 byte for opcode
            #                + 4 bytes for offset
        elif opcode == "ret":
            length += 1  # 1 byte for opcode
        elif opcode == "cmp":
            length += 1 + 2  # 1 byte opcode + 2 bytes of register numbers
        elif opcode == "tst":
            length += 1 + 1  # 1 byte for the opcode
            #                + 1 byte for the register number
        elif opcode == "je":
            length += 1 + 4  # 1 byte for opcode
            #                + 4 bytes for offset
        elif opcode == "jneg":
            length += 1 + 4  # 1 byte for opcode
            #                + 4 bytes for offset
        elif opcode == "jpos":
            length += 1 + 4  # 1 byte for opcode
            #                + 4 bytes for offset
        elif opcode == "addi":
            length += 1 + 1 + 4 # 1 byte opcode
            #                   + 1 byte of register number
            #                   + 4 bytes for value
        elif opcode == "label":
            label_addresses[x[1][0]] = length
    return label_addresses


def write_opcodes(file_parsed, filename, label_addresses):
    with open(filename, "w") as f:
        f.write("")
    filestream = open(filename, "ab+")
    for x in file_parsed:
        opcode = x[0].lower()
        if opcode == "hlt":
            filestream.write(b"\x00")
        elif opcode == "mov":
            filestream.write(struct.pack("<3B", 1, int(x[1][0]), int(x[1][1])))
        elif opcode == "xor":
            filestream.write(struct.pack("<3B", 2, int(x[1][0]), int(x[1][1])))
        elif opcode == "add":
            filestream.write(struct.pack("<3B", 3, int(x[1][0]), int(x[1][1])))
        elif opcode == "store":
            filestream.write(struct.pack("<2Bi", 4, int(x[1][0]), int(x[1][1])))
        elif opcode == "push":
            filestream.write(struct.pack("<2B", 5, int(x[1][0])))
        elif opcode == "pop":
            filestream.write(struct.pack("<2B", 6, int(x[1][0])))
        elif opcode == "jmp":
            if x[1][0] in label_addresses.keys():
                filestream.write(struct.pack("<Bi", 7, label_addresses[x[1][0]]))
            else:
                filestream.write(struct.pack("<Bi", 7, int(x[1][0])))
        elif opcode == "prnt":
            filestream.write(struct.pack("<3B", 8, int(x[1][0]), int(x[1][1])))
        elif opcode == "call":
            if x[1][0] in label_addresses.keys():
                filestream.write(struct.pack("<Bi", 9, label_addresses[x[1][0]]))
            else:
                filestream.write(struct.pack("<Bi", 9, int(x[1][0])))
        elif opcode == "ret":
            filestream.write(struct.pack("<B", 10))
        elif opcode == "cmp":
            filestream.write(struct.pack("<3B", 11, int(x[1][0]), int(x[1][1])))
        elif opcode == "tst":
            filestream.write(struct.pack("<2B", 12, int(x[1][0])))
        elif opcode == "je":
            if int(x[1][0]) in label_addresses.keys():
                filestream.write(struct.pack("<Bi", 13, label_addresses[x[1][0]]))
            else:
                filestream.write(struct.pack("<Bi", 13, int(x[1][0])))
        elif opcode == "jneg":
            if int(x[1][0]) in label_addresses.keys():
                filestream.write(struct.pack("<Bi", 14, label_addresses[x[1][0]]))
            else:
                filestream.write(struct.pack("<Bi", 14, int(x[1][0])))
        elif opcode == "jpos":
            if x[1][0] in label_addresses.keys():
                filestream.write(struct.pack("<Bi", 15, label_addresses[x[1][0]]))
            else:
                filestream.write(struct.pack("<Bi", 15, int(x[1][0])))
        elif opcode == "addi":
            filestream.write(struct.pack("<2Bi", 16, int(x[1][0]), int(x[1][1])))
        elif opcode == "mul":
            filestream.write(struct.pack("<3B", 17, int(x[1][0]), int(x[1][1])))
        elif opcode == "muli":
            filestream.write(struct.pack("<2Bi", 18, int(x[1][0]), int(x[1][1])))
        elif opcode == "label":
            pass
    filestream.close()

if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()
        
    file_parsed = []

    with open(sys.argv[1], "r") as f:
        if f.read(14) != "--BEGIN ASM--\n":
            sys.exit("Missing --BEGIN ASM-- header")
        file = f.read().strip().replace(" ", "").replace("\t", "").split("\n")

        for x in file:
            if len(x.split(":")) > 1:
                file_parsed.append([x.split(":")[0], x.split(":")[1].split(",")])
            else:
                file_parsed.append([x])
        label_addresses = init_size_and_labels(file_parsed)
        write_opcodes(file_parsed, sys.argv[2], label_addresses)
