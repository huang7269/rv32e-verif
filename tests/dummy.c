
void _start(){
  __asm__ volatile ("addi x10, x0, 1");
  __asm__ volatile ("addi x10, x10, 2");
  __asm__ volatile ("addi x10, x10, 3");
  __asm__ volatile("ebreak");
}
