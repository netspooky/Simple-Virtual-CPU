from vcpu_asm import ASM
c = ASM("fibonacci.bin")

c.store(0, 72)
c.prnt(0)
c.store(0, 69)
c.prnt(0)
c.store(0, 108)
c.prnt(0)
c.store(0, 108)
c.prnt(0)
c.store(0, 79)
c.prnt(0)
c.call(55, 1)
c.store(0, 32) # space
c.prnt(0)
c.store(0, 119)
c.prnt(0)
c.store(0, 79)
c.prnt(0)
c.store(0, 82)
c.prnt(0)
c.store(0, 76)
c.prnt(0)
c.store(0, 68)
c.prnt(0)
c.hlt()
#36 + 12 + 1 bytes


#init
c.store(0, 0) #1 + 1 + 4 = 6bytes
c.prnt(0)
c.store(1, 1)
c.prnt(1)

#loop
c.mov(2, 0) #3bytes
c.add(2, 1) #3bytes
c.prnt(2)   #2bytes
c.mov(0, 1) #3bytes
c.mov(1, 2) #3bytes
c.tst(2) #2bytes
c.jpos(-16, 1)
c.store(0, 1)
c.push(0) #satify calling convention --> return value will be stored in reg0
#end
c.ret()
