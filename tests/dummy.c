
void _start(){
  volatile int a = 10;
  volatile int b = 20;
  volatile int c = 30;

  __asm__ volatile("ebreak");
}
