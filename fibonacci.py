from vcpu_asm import ASM
c = ASM("fibonacci.bin")

c.store(0, 72) # H
c.prnt(0, 1)
c.store(0, 69) # E
c.prnt(0, 1)
c.store(0, 76) # L
c.prnt(0, 1)
c.store(0, 76) # L
c.prnt(0, 1)
c.store(0, 79) # O
c.prnt(0, 1)
c.store(0, 10) # \n
c.prnt(0, 1)
# call = 6bytes, store = 6bytes, prnt = 3 bytes
# --> 6 + 6*6 + 3*6 + 1
c.call(61, 1)
#call to init
c.store(0, 87) # W
c.prnt(0, 1)
c.store(0, 79) # O
c.prnt(0, 1)
c.store(0, 82) # R
c.prnt(0, 1)
c.store(0, 76) # L
c.prnt(0, 1)
c.store(0, 68) # D
c.prnt(0, 1)
c.store(0, 10) # \n
c.prnt(0, 1)
c.hlt()


#init
c.store(0, 0) #1 + 1 + 4 = 6bytes
c.prnt(0, 0)
c.store(1, 1)
c.prnt(1, 0)

#loop
c.mov(2, 0) #3bytes
c.add(2, 1) #3bytes
c.prnt(2, 0)#3bytes
c.mov(0, 1) #3bytes
c.mov(1, 2) #3bytes
c.tst(2)    #2bytes
c.jpos(-17, 1)
c.store(0, 1)
c.push(0) #satify calling convention --> return value will be stored in reg0

#end
c.ret()
