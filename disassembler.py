from struct import *
from collections import namedtuple
from pathlib import Path
import sys 

MAXLEN = 12 # Maximum length of single instruction
MAXINS = 5  # Maximum instruction mnemonic size
bPos   = 0  # Initializing file position
data   = Path(sys.argv[1]).read_bytes() # File to disassemble

isa = {
   "0": { "ins":   "hlt", "len": 1, "ops": 1, "bStruct":   "<B" },
   "1": { "ins":   "mov", "len": 3, "ops": 3, "bStruct":  "<3B" },
   "2": { "ins":   "xor", "len": 3, "ops": 2, "bStruct":  "<3B" },
   "3": { "ins":   "add", "len": 3, "ops": 3, "bStruct":  "<3B" },
   "4": { "ins": "store", "len": 6, "ops": 3, "bStruct": "<2Bi" },
   "5": { "ins":  "push", "len": 2, "ops": 2, "bStruct":  "<2B" },
   "6": { "ins":   "pop", "len": 2, "ops": 2, "bStruct":  "<2B" },
   "7": { "ins":   "jmp", "len": 3, "ops": 2, "bStruct":  "<Bi" },
   "8": { "ins":  "prnt", "len": 3, "ops": 3, "bStruct":  "<3B" },
   "9": { "ins":  "call", "len": 5, "ops": 2, "bStruct":  "<Bi" },
  "10": { "ins":   "ret", "len": 1, "ops": 1, "bStruct":   "<B" },
  "11": { "ins":   "cmp", "len": 3, "ops": 3, "bStruct":  "<3B" },
  "12": { "ins":   "tst", "len": 2, "ops": 2, "bStruct":  "<2B" },
  "13": { "ins":    "je", "len": 4, "ops": 2, "bStruct":  "<Bi" },
  "14": { "ins":  "jneg", "len": 5, "ops": 2, "bStruct":  "<Bi" },
  "15": { "ins":  "jpos", "len": 5, "ops": 2, "bStruct":  "<Bi" },
  "16": { "ins":  "addi", "len": 6, "ops": 3, "bStruct": "<2Bi" },
  "17": { "ins":   "mul", "len": 3, "ops": 3, "bStruct":  "<3B" },
  "18": { "ins":  "muli", "len": 6, "ops": 3, "bStruct": "<2Bi" }
}

def dAsm(rawOp,bStruct,ops):
	if ops == 1:
		ins = namedtuple("ins","opcode")
	if ops == 2:
		ins = namedtuple("ins","opcode register")
	if ops == 3:
		ins = namedtuple("ins","opcode register number")
	out = ins._make(unpack(bStruct,rawOp))
	return out

def printIns(curs0r):
	global bPos
	sC = str(curs0r)
	if sC in isa.keys():
		insName = isa[sC]["ins"]
		insLen  = isa[sC]["len"] 
		insOps  = isa[sC]["ops"]
		bStruct = isa[sC]["bStruct"]
		bindata = data[bPos:bPos+insLen]
		binHex  = bindata.hex()
		padding = ' '*(12 - len(binHex))
		disas   = dAsm(bindata,bStruct,insOps)
		o0      = disas[0]
		if insOps == 1:
			print("{:08X}: {}{} {}".format(bPos,binHex,padding,insName))
		if insOps == 2:
			o1 = disas[1]
			print("{:08X}: {}{} {}: {}".format(bPos,binHex,padding,insName,o1))
		if insOps == 3:
			o1 = disas[1]
			o2 = disas[2]
			print("{:08X}: {}{} {}: {}, {}".format(bPos,binHex,padding,insName,o1,o2))
		bPos = bPos + insLen

while True:
	try:
		curs = data[bPos]
		printIns(curs)
	except Exception as e:
		#print(e) # for debug
		exit()