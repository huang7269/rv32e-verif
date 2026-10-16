void _start(){
    __asm__ volatile("addi x1,x0,4");
    __asm__ volatile("addi x2,x0,5");
    __asm__ volatile("addi x3,x0,0xffffffff");
    __asm__ volatile("slti x4,x1,0x5");
    __asm__ volatile("slti x5,x1,-1");
    __asm__ volatile("sltiu x6,x1,-1");
    __asm__ volatile("xori x7,x1,-1");
    __asm__ volatile("ori x8,x1,-1");
    __asm__ volatile("andi x9,x1,-1");
    __asm__ volatile("slli x10,x1,0x2");
    __asm__ volatile("srl x11,x1,0x2");
    __asm__ volatile("srl x12,x3,0x2");
    __asm__ volatile("sra x13,x1,0x2");
    __asm__ volatile("sra x14,x3,0x2");

    __asm__ volatile("ebreak");
}