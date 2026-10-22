void _start(){
    __asm__ volatile ("addi x1,x0,4");
    __asm__ volatile ("addi x2,x0,5");
    __asm__ volatile ("addi x3,x0,0xffffffff");
    __asm__ volatile ("add x4,x1,x2");
    __asm__ volatile ("sub x5,x2,x1");
    __asm__ volatile ("sll x6,x2,x1");
    __asm__ volatile ("slt x7,x1,x3");
    __asm__ volatile ("sltu x8,x1,x3");
    __asm__ volatile ("xor x9,x1,x2");
    __asm__ volatile ("sra x10,x3,x1");
    __asm__ volatile ("srl x11,x3,x1");
    __asm__ volatile ("or x12,x1,x2");
    __asm__ volatile ("and x13,x1,x2");
    __asm__ volatile ("sub x14,x1,x2");
    
    __asm__ volatile ("add x0,x1,x2");
    __asm__ volatile ("ebreak");
}