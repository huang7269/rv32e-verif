void _start(){
    __asm__ volatile("lui x1,1");
    __asm__ volatile("auipc x2,1");
    __asm__ volatile("ebreak");
}