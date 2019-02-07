#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <sys/mman.h>
#include <unistd.h>

// #define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))
// #define DEBUG
//config
#define STACK_SIZE 4096
#define HEAP_MAX_SIZE 8192
#define NO_GEN_REGISTERS 8
#define CPU_FLAG_ZF 0
#define CPU_FLAG_SF 1
#define CPU_FLAG_OF 2


#if NO_GEN_REGISTERS < 3
    #warning "Not enough general registers, the cpu may not work properly with some programs."
#endif

// ╔════════════════════════════════════════════════════════╗
// ║             Duchy's VCPU version 0.1 beta              ║
// ╠════════╦══════════════╦══════════════════╦═════════════╣
// ║ OPCODE ║     ARGS     ║                  ║ INSTRUCTION ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x00  ║       -      ║         -        ║     HLT     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x01  ║ uint8_t reg1 ║   uint8_t reg2   ║     MOV     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x02  ║ uint8_t reg1 ║   uint8_t reg2   ║     XOR     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x03  ║ uint8_t reg1 ║   uint8_t reg2   ║     ADD     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x04  ║  uint8_t reg ║     int value    ║    STORE    ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x05  ║  uint8_t reg ║         -        ║     PUSH    ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x06  ║  uint8_t reg ║         -        ║     POP     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x07  ║  int offset  ║         -        ║     JMP     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x08  ║  uint8_t reg ║   uint8_t ASCII  ║    PRINT    ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║  0x09  ║  int offset  ║         -        ║     CALL    ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║   0xA  ║       -      ║         -        ║     RET     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║   0xB  ║ uint8_t reg1 ║   uint8_t reg2   ║     CMP     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║   0xC  ║ uint8_t reg1 ║         -        ║     TST     ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║   0xD  ║  int offset  ║         -        ║     JE      ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║   0xE  ║  int offset  ║         -        ║     JNEG    ║
// ╠════════╬══════════════╬══════════════════╬═════════════╣
// ║   0xF  ║  int offset  ║         -        ║     JPOS    ║
// ╚════════╩══════════════╩══════════════════╩═════════════╝

typedef struct cpu_info{
    unsigned char * code;
    uint32_t code_len;
    int stack[STACK_SIZE];
    uint32_t sp;
    uint32_t ip;
    int reg[NO_GEN_REGISTERS];
    uint8_t hlt;
    uint8_t cpu_flag[3]; // ZF, SF, OF
} cpu_info;

cpu_info *cpu_base;

uint8_t __check_overflow(int x, int y){
    if ((y > 0 && x > INT_MAX - y) || (y < 0 && x < INT_MIN - y)){
        return 1;
    }else{
        return 0;
    }
}

void mov(uint8_t reg1, uint8_t reg2){
    if(reg1 > NO_GEN_REGISTERS || reg2 > NO_GEN_REGISTERS){
        printf("mov register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    // move the contents of a register into another register
    cpu_base->reg[reg1] = cpu_base->reg[reg2];
}

void _xor(uint8_t reg1, uint8_t reg2){
    if(reg1 > NO_GEN_REGISTERS || reg2 > NO_GEN_REGISTERS){
        printf("xor register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    //eXclusive OR
    //  a  |  b
    //  1  |  1  -->  0
    //  1  |  0  -->  1
    //  0  |  1  -->  1
    //  0  |  0  -->  0
    cpu_base->reg[reg1] = (cpu_base->reg[reg1] ^ cpu_base->reg[reg2]);
}

void add(uint8_t reg1, uint8_t reg2){
    if(reg1 > NO_GEN_REGISTERS || reg2 > NO_GEN_REGISTERS){
        printf("add register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    //set the CPU flags
    cpu_base->cpu_flag[CPU_FLAG_OF] = __check_overflow(cpu_base->reg[reg1], cpu_base->reg[reg2]);
    if ((cpu_base->reg[reg1] + cpu_base->reg[reg2]) < 0){
        cpu_base->cpu_flag[CPU_FLAG_SF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_SF] = 0;
    }
    if ((cpu_base->reg[reg1] + cpu_base->reg[reg2]) == 0){
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 0;
    }
    //... addition ...
    cpu_base->reg[reg1] = (cpu_base->reg[reg1] + cpu_base->reg[reg2]);
}

void store(uint8_t reg1, int value){
    if(reg1 > NO_GEN_REGISTERS){
        printf("store register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
#ifdef DEBUG
    printf("Stored at: %u\n", cpu_base->ip);
#endif
    //does this really need an explanation?!
    cpu_base->reg[reg1] = value;
}

void push(uint8_t reg1){
    if(reg1 > NO_GEN_REGISTERS){
        printf("push register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    if(cpu_base->sp >= STACK_SIZE){
        puts("static stack size reached, quitting");
        return;
    }
    #ifdef DEBUG
        printf("Pushed at: %u\n", cpu_base->ip);
    #endif
    //put a value of the register on top of the stack
    cpu_base->stack[cpu_base->sp] = cpu_base->reg[reg1];
    cpu_base->sp += 1;
}

void pop(uint8_t reg1){
    if(reg1 > NO_GEN_REGISTERS){
        printf("pop register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    //load the top value from the stack to a register and lower the stack pointer --> the value can be overwritten
    if(cpu_base->sp == 0){
        printf("can't pop, because sp is 0\nip: %u\n", cpu_base->ip);
        return;
    }
    cpu_base->sp -= 1;
    cpu_base->reg[reg1] = cpu_base->stack[cpu_base->sp];
}

void jmp(int offset){
    // from current position of reg<ip>
#ifdef DEBUG
    printf("JMP from %u to offset %i\n", cpu_base->ip, offset);
#endif
    // from start
    if((offset < 0) || (offset > cpu_base->code_len)){
        printf("invalid jmp offset\nip: %u\n", cpu_base->ip);
        return;
    }
    cpu_base->ip = offset;
    return;
}

void prnt(uint8_t reg1, uint8_t ascii){
    //print to stdout
    if(reg1 > NO_GEN_REGISTERS){
        printf("prnt register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    //ascii only output
    if(ascii && (((cpu_base->reg[reg1] > 31) && (cpu_base->reg[reg1] < 128)) || cpu_base->reg[reg1] == 10 || cpu_base->reg[reg1] == 13)){
        printf("%c", cpu_base->reg[reg1]);
    }else{
        printf("register %u: %i\n", reg1, cpu_base->reg[reg1]);
    }
}

void hlt(){
    cpu_base->hlt = 1; // stops execution loop
#ifdef DEBUG
    printf("Halted at ip: %u\n", cpu_base->ip);
#endif
    return;
}

void call(int offset){
    // move the instruction pointer after the call instruction, so we can restore to after the instruction
    store(0, cpu_base->ip + 1 + sizeof(int));
    push(0);                          // save the instruction pointer
    store(0, 0);
#ifdef DEBUG
    printf("call ip: %u\noffset: %i\n", cpu_base->ip, offset);
#endif
    jmp(offset);            // jump to function location --> expect ret at the end of the function
}

void ret(){
    pop(0);     // return value will be stored in the r0 general register
    pop(1);     // pop the original instruction pointer --> expects the function to do it's thing and restore the stack to the original state
    cpu_base->ip = cpu_base->reg[1];
    store(1, 0);
}

void cmp(uint8_t reg1, uint8_t reg2){
    // set the CPU flags
    // overflow flag --> INT_MAX + 1 = INT_MIN
    cpu_base->cpu_flag[CPU_FLAG_OF] = __check_overflow(cpu_base->reg[reg1], cpu_base->reg[reg2]);
    // signed flag --> x-y < 0
    if ((cpu_base->reg[reg1] - cpu_base->reg[reg2]) < 0){
        cpu_base->cpu_flag[CPU_FLAG_SF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_SF] = 0;
    }
    // zero flag --> x-y = 0 --> x = y
    if ((cpu_base->reg[reg1] - cpu_base->reg[reg2]) == 0){
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 0;
    }
}

void tst(uint8_t reg1){
    //same as cmp, but just for a single reg
    cpu_base->cpu_flag[CPU_FLAG_OF] = 0;
    if ((cpu_base->reg[reg1]) < 0){
        cpu_base->cpu_flag[CPU_FLAG_SF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_SF] = 0;
    }
    if ((cpu_base->reg[reg1]) == 0){
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 0;
    }
}

// CONDITIONAL JUMPS

void je(int offset){
    // from start
    if((offset < 0) || (offset > cpu_base->code_len)){
        printf("invalid jmp offset\nip: %u\n", cpu_base->ip);
        return;
    }
    //if the flag is set
    if(cpu_base->cpu_flag[CPU_FLAG_ZF]){
        cpu_base->ip = offset;
    }else{
        cpu_base->ip += 1 + sizeof(int);
    }
    return;
}

void jneg(int offset){
    // same as above, but with a different flag
    // from start
    if((offset < 0) || (offset > cpu_base->code_len)){
        printf("invalid jmp offset\nip: %u\n", cpu_base->ip);
        return;
    }
    if(cpu_base->cpu_flag[CPU_FLAG_SF]){
        cpu_base->ip = offset;
    }else{
        cpu_base->ip += 1 + sizeof(int);
    }
    return;
}


void jpos(int offset){
    // same as above, but with a different flag
    // from start
    if((offset < 0) || (offset > cpu_base->code_len)){
        printf("invalid jmp offset\nip: %u\n", cpu_base->ip);
        return;
    }
    if(!cpu_base->cpu_flag[CPU_FLAG_SF]){
        cpu_base->ip = offset;
    }else{
        cpu_base->ip += 1 + sizeof(int);
    }
    return;
}

void addi(uint8_t reg1, int value){
    if(reg1 > NO_GEN_REGISTERS){
        printf("add register invalid\nip: %u\n", cpu_base->ip);
        return;
    }
    //set the CPU flags
    cpu_base->cpu_flag[CPU_FLAG_OF] = __check_overflow(cpu_base->reg[reg1], value);
    if (cpu_base->reg[reg1] + value < 0){
        cpu_base->cpu_flag[CPU_FLAG_SF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_SF] = 0;
    }
    if (cpu_base->reg[reg1] + value == 0){
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 1;
    }else{
        cpu_base->cpu_flag[CPU_FLAG_ZF] = 0;
    }
    //... addition ...
    cpu_base->reg[reg1] = (cpu_base->reg[reg1] + value);
}

void parse_instructions(){
    int offset;
    int number;
    while(cpu_base->hlt == 0){
        unsigned char instruction = cpu_base->code[cpu_base->ip];
#ifdef DEBUG
        printf("Check for instruction at: %u\n", cpu_base->ip);
        printf("sp:%u\n", cpu_base->sp);
#endif
        switch (instruction) {
            case 0x00://halt
                hlt();
                break;
            case 0x01://mov
                mov(cpu_base->code[cpu_base->ip + 1], cpu_base->code[cpu_base->ip + 2]);
                cpu_base->ip += 3;
                break;
            case 0x02://xor
                _xor(cpu_base->code[cpu_base->ip + 1], cpu_base->code[cpu_base->ip + 2]);
                cpu_base->ip += 3;
                break;
            case 0x03://add
                add(cpu_base->code[cpu_base->ip + 1], cpu_base->code[cpu_base->ip + 2]);
                cpu_base->ip += 3;
                break;
            case 0x04://store
                number = *(int*)(cpu_base->code + cpu_base->ip + 2);
                store(cpu_base->code[cpu_base->ip + 1], number);
                cpu_base->ip += 2 + sizeof(int);
                break;
            case 0x05://push
                push(cpu_base->code[cpu_base->ip + 1]);
                cpu_base->ip += 2;
                break;
            case 0x06://pop
                pop(cpu_base->code[cpu_base->ip + 1]);
                cpu_base->ip += 2;
                break;
            case 0x07://jump
                offset = *(int*)(cpu_base->code + cpu_base->ip + 1);
                jmp(offset);
                break;
            case 0x08://print
                prnt(cpu_base->code[cpu_base->ip + 1], cpu_base->code[cpu_base->ip + 2]);
                cpu_base->ip += 3;
                break;
            case 0x09://call
                offset = *(int*)(cpu_base->code + cpu_base->ip + 1);
                call(offset);
                break;
            case 0xA://ret
                ret();
                break;
            case 0xB://compare
                cmp(cpu_base->code[cpu_base->ip + 1], cpu_base->code[cpu_base->ip + 2]);
                cpu_base->ip += 3;
                break;
            case 0xC://test
                tst(cpu_base->code[cpu_base->ip + 1]);
                cpu_base->ip += 2;
                break;
            case 0xD://jump equal
                offset = *(int*)(cpu_base->code + cpu_base->ip + 1);
                je(offset);
                break;
            case 0xE://jump negative
                offset = *(int*)(cpu_base->code + cpu_base->ip + 1);
                jneg(offset);
                break;
            case 0xF://jump positive
                offset = *(int*)(cpu_base->code + cpu_base->ip + 1);
                jpos(offset);
                break;
            case 0x10:// add intieger
                offset = *(int*)(cpu_base->code + cpu_base->ip + 2);
                addi(cpu_base->code[cpu_base->ip + 1], offset);
                cpu_base->ip += 6;
                break;
            default:
                printf("invalid instruction!\nip: %u", cpu_base->ip);
                return;
        }
    }
}


void init_cpu(void * code_ptr, uint32_t code_len){
    // alloc the cpu
    cpu_base = (cpu_info *)calloc(1, sizeof(cpu_info));
    cpu_base->code = code_ptr;
    cpu_base->code_len = code_len;
    cpu_base->hlt = 0;
}


void usage(){
    puts("Usage: ./cpu <filename>");
}

int main(int argc, char ** argv){
    if (argc != 2){
        usage();
        return -1;
    }
    FILE *code;
    code = fopen(argv[1], "r");
    int fd = fileno(code);
    // calculation of the filesize by jumping at the end and getting the offset
    uint32_t code_len = lseek(fd, 0, SEEK_END) + 1;
    // map a read-only memory region for our code
    unsigned char * code_data = mmap(0, code_len, PROT_READ, MAP_SHARED, fd, 0);
    printf("code length: %u\n", code_len);
    // the entrypoint can be set by jumping there in the beginning of the binary
    init_cpu(code_data, code_len);
    parse_instructions();
    munmap(code_data, code_len);
    fclose(code);
    return 0;
}
