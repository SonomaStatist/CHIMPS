#include "cpu.h"

// holds all the registers, reg[0] == zero, reg[1] == at, etc
static uint32_t reg[32];
// defines the inputs and outputs of the register file
static uint32_t read_reg1;
static uint32_t read_reg2;
static uint32_t *write_reg;

// holds the program counter
static uint32_t pc;

// reads the next instruction from the text segment of memory
// sets the state of the instruction output lines
static inline void load_inst();

// instruction outputs
static uint8_t op;
static uint8_t rs;
static uint8_t rt;
static uint8_t rd;
static uint8_t shamt;
static uint8_t funct;
static uint16_t imm;
static uint32_t addr;

static int32_t sign_imm;
static uint32_t alu_res;

static uint32_t mem_res;

// sets the state of the control lines
static inline void decode_inst();

// defines the control lines
static bool reg_dst;
static bool jump;
static bool branch;
static bool mem_read;
static bool memto_reg;
static bool alu_op1;
static bool alu_op2;
static bool reg_write;
static bool alu_src;
static bool mem_write;
// main alu zero out
static bool alu_zero;

// the main alu for for the cpu
static inline void main_alu();

// read or write to mem
static inline void access_mem();

// defines all the mutexes
// sets the state of the read1, read2, write registers
static inline void reg_mux();
// sets the state of the write data line
static inline void mem_mux();
// sets the state of the pc
static inline void pc_mux();

// sign extend from 16 -> 32 bit
static inline void sign_ext();

// holds and defines the layout of the virtual memory
static uint32_t text_end;
static uint32_t data_end;
static uint32_t heap_end;
static uint32_t mem_size;
static uint8_t *memory;

static bool err_flag;

bool init_cpu(unsigned len, uint32_t *program) {
    return true;
}

bool step() {
    load_inst();
    decode_inst();
    reg_mux();
    sign_ext();
    main_alu();
    access_mem();
    mem_mux();
    pc_mux();

    return err_flag;
}

static inline void load_inst() {
    uint32_t inst = ((uint32_t) (memory[alu_res]) << 24) + ((uint32_t) (memory[alu_res + 1]) << 16) +
            ((uint32_t) (memory[alu_res + 2]) << 8) + ((uint32_t) (memory[alu_res + 3]));
    pc += 4;

    op = (uint8_t) (inst >> 26);
    rs = (uint8_t) ((inst >> 20) & 0x1f);
    rt = (uint8_t) ((inst >> 15) & 0x1f);
    shamt = (uint8_t) ((inst >> 6) & 0x3f);
    funct = (uint8_t) (inst & 0x3f);
    imm = (uint16_t) inst;
    addr = inst & 0x3ffffff;
}

static inline void decode_inst() {
    // set intermediate boolean values
    bool rfmt = op == 0x0;
    bool lw = op == 0x13;
    bool sw = op == 0x1b;
    bool beq = op == 0x4;
    // instruction is R-type
    reg_dst = rfmt;
    alu_src = lw || sw;
    memto_reg = lw;
    reg_write = rfmt || lw;
    mem_read = lw;
    mem_write = sw;
    branch = beq;
    alu_op1 = rfmt;
    alu_op2 = beq;
}

static inline void reg_mux() {
    read_reg1 = reg[rs];
    read_reg2 = reg[rt];
    write_reg = &reg[reg_dst ? rd : rs];
}

static inline void main_alu() {
    if (alu_op1) {
        switch (funct) {
            case 0x20:
                alu_res = (uint32_t) (read_reg1 + (alu_src ? sign_imm : (int32_t) (read_reg2)));
                alu_zero = false;
                break;
            case 0x22:
                alu_res = (uint32_t) (read_reg1 - (alu_src ? sign_imm : (int32_t) (read_reg2)));
                alu_zero = false;
                break;
            case 0x24:
                alu_res = (uint32_t) (read_reg1 & (alu_src ? sign_imm : (int32_t) (read_reg2)));
                alu_zero = false;
                break;
            case 0x25:
                alu_res = (uint32_t) (read_reg1 | (alu_src ? sign_imm : (int32_t) (read_reg2)));
                alu_zero = false;
                break;
            case 0x2a:
                alu_res = (uint32_t) (read_reg1 - (alu_src ? sign_imm : (int32_t) (read_reg2)));
                alu_zero = 0 < (0x80000000 & alu_res);
                break;
            default:
                err_flag = true;
                break;
        }
    } else if (alu_op2) {
        alu_res = (uint32_t) (read_reg1 - (alu_src ? sign_imm : (int32_t) (read_reg2)));
        alu_zero = !alu_res;
    } else {
        alu_res = (uint32_t) (read_reg1 + (alu_src ? sign_imm : (int32_t) (read_reg2)));
    }
}

static inline void sign_ext() {
    sign_imm = (0xffff & imm);
    if (0x8000 & imm) {
        sign_imm += 0xffff0000;
    }
}

static inline void access_mem() {
    if (mem_read) {
        if (alu_res >= mem_size) {
            err_flag = true;
        } else {
            mem_res = ((uint32_t) (memory[alu_res]) << 24) + ((uint32_t) (memory[alu_res + 1]) << 16) +
                    ((uint32_t) (memory[alu_res + 2]) << 8) + ((uint32_t) (memory[alu_res + 3]));
        }
    } else if (mem_write) {
        if (alu_res >= mem_size) {
            err_flag = true;
        } else {
            memory[alu_res] = (uint8_t) (alu_res >> 24);
            memory[alu_res + 1] = (uint8_t) (alu_res >> 16);
            memory[alu_res + 2] = (uint8_t) (alu_res >> 8);
            memory[alu_res + 3] = (uint8_t) alu_res;
        }
    }
}

static inline void mem_mux() {
    if (reg_write) {
        *write_reg = memto_reg ? mem_res : alu_res;
    }
}

static inline void pc_mux() {
    if (jump) {
        pc = (0xf0000000 & pc) + (addr << 2);
    } else if (alu_zero && branch) {
        pc = (uint32_t)(((int32_t)pc + (sign_imm << 2)));
    }
}