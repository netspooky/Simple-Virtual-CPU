import struct

class ASM:
    def __init__(self, filename):
        with open(filename, "w") as f:
            f.write("")
        self.filestream = open(filename, "a+")

    def __del__(self):
        self.filestream.close()

    def hlt(self):
        self.filestream.write("\x00")

    def mov(self, reg1, reg2):
        self.filestream.write(struct.pack("<3B", 1, reg1, reg2))

    def xor(self, reg1, reg2):
        self.filestream.write(struct.pack("<3B", 2, reg1, reg2))

    def add(self, reg1, reg2):
        self.filestream.write(struct.pack("<3B", 3, reg1, reg2))

    def store(self, reg1, value):
        self.filestream.write(struct.pack("<2Bi", 4, reg1, value))

    def push(self, reg1):
        self.filestream.write(struct.pack("<2B", 5, reg1))

    def pop(self, reg1):
        self.filestream.write(struct.pack("<2B", 6, reg1))

    def jmp(self, offset, relative):
        self.filestream.write(struct.pack("<BiB", 7, offset, relative))

    def prnt(self, reg1, ascii):
        self.filestream.write(struct.pack("<3B", 8, reg1, ascii))

    def call(self, offset, relative):
        self.filestream.write(struct.pack("<BiB", 9, offset, relative))

    def ret(self):
        self.filestream.write(struct.pack("<B", 10))

    def cmp(self, reg1, reg2):
        self.filestream.write(struct.pack("<3B", 11, reg1, reg2))

    def tst(self, reg1):
        self.filestream.write(struct.pack("<2B", 12, reg1))

    def je(self, offset, relative):
        self.filestream.write(struct.pack("<BiB", 13, offset, relative))

    def jneg(self, offset, relative):
        self.filestream.write(struct.pack("<BiB", 14, offset, relative))

    def jpos(self, offset, relative):
        self.filestream.write(struct.pack("<BiB", 15, offset, relative))
