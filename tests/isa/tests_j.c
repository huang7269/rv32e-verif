void _start(){
    __asm__ volatile("jal x1, r1\n"
                    "addi x2, x0, 1\n"
                    "ebreak\n"
                    "r1:\n"
                    "addi x3, x0, 2\n"

                    "jalr x4, x1, 0\n"
                    "addi x5, x0, 1\n"
                    "ebreak"
    );
}