#include "cpu.h"

// holds and defines the layout of the virtual memory
static unsigned char *ram;
static unsigned test_end;
static unsigned data_end;
static unsigned heap_end;
static unsigned mem_size;

// holds the program counter
static unsigned pc;
// holds the location of the next instruction
static unsigned npc;

// reads the next instruction from the text segment of memory
// sets the state of the instruction output lines
static inline void read_inst();

// instruction outputs
static uint8_t op;
static uint8_t rs;
static uint8_t rt;
static uint8_t rd;
static uint8_t shamt;
static uint8_t funct;
static uint16_t imm;
static uint32_t addr;

// holds all the registers, reg[0] == zero, reg[1] == at, etc
struct {
    union {
        uint32_t reg[32];
        struct {
            uint32_t
                    zero,
                    at,
                    v0, v1,
                    a0, a1, a2, a3,
                    t0, t1, t2, t3, t4, t5, t6, t7,
                    s0, s1, s2, s3, s4, s5, s6, s7,
                    t7, t8,
                    k0, k1,
                    gp, sp, fp, ra;
        };
    };
    float f0;
} static reg_file;
// defines the inputs and outputs of the register file
static uint32_t read_reg1;
static uint32_t read_reg2;
static uint32_t write_reg;
static uint32_t write_dat;
static uint32_t read_dat1;
static uint32_t read_dat2;

// sets the state of the control lines
static inline void control();

// the main alu for for the cpu
static inline void main_alu();

// computes the value of the address to jump to
static inline void jump_alu();

// defines the control lines
static bool reg_dst;
static bool jump;
static bool branch;
static bool mem_read;
static bool memto_red;
static bool alu_op;
static bool write_reg;
static bool alu_src;
static bool reg_data;

// defines all the mutexes
// sets the state of the write register
static inline void wr_mux();
// sets the state of the write data line
static inline void mem_mux();

bool init_cpu(unsigned len, uint32_t *program) {
    reg_file.zero = 0;
    reg_file.reg[0] = 0;
    return 0;
}

int step(){
    return 0;
}

int run() {
    return 0;
}
