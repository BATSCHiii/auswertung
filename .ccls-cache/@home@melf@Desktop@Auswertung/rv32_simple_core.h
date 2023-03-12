#ifndef __RV32_SIMPLE_CORE_H__
#define __RV32_SIMPLE_CORE_H__

/******************************************************************
    rv32_simple_core.h
    This file is generated from rv32_simple_core.v by V2SC
    (c) Copryight 2007 by Ali Haj Abutalebi & Leila Mahmoudi Ayough
    Mazdak and Alborz Design Automation
    email: info@mazdak-alborz.com
    website: www.mazdak-alborz.com
    modified by: waielal
*******************************************************************/

#include "systemc.h"

enum ctrl_alu_op_t {
    CTRL_ALU_OP__NOP,
    CTRL_ALU_OP__ADD,
    CTRL_ALU_OP__SUB,
    CTRL_ALU_OP__OR,
    CTRL_ALU_OP__AND,
    CTRL_ALU_OP__XOR,
    CTRL_ALU_OP__SLL,
    CTRL_ALU_OP__SRL,
    CTRL_ALU_OP__SRA,
    CTRL_ALU_OP__EQ,
    CTRL_ALU_OP__NEQ,
    CTRL_ALU_OP__LT,
    CTRL_ALU_OP__GE,
    CTRL_ALU_OP__LTU,
    CTRL_ALU_OP__GEU,
    CTRL_ALU_OP__OP1,
    CTRL_ALU_OP__OP2,
};

enum ctrl_alu_src_t {
    CTRL_ALU_SRC__REG,
    CTRL_ALU_SRC__IMM,
    CTRL_ALU_SRC__PC,
    CTRL_ALU_SRC__4,
};

enum ctrl_rd_src_t {
    CTRL_RD_SRC__NO_WRITE,
    CTRL_RD_SRC__ALU_RESULT,
    CTRL_RD_SRC__MEM_DATA,
};

enum ctrl_pc_src_t {
    CTRL_PC_SRC__PC_NEXT,
    CTRL_PC_SRC__PC_IMM,
    CTRL_PC_SRC__RS1_IMM,
};

enum ctrl_mem_cmd_t {
    CTRL_MEM_CMD__WRITE_B,
    CTRL_MEM_CMD__WRITE_H,
    CTRL_MEM_CMD__WRITE_W,
    CTRL_MEM_CMD__READ_B,
    CTRL_MEM_CMD__READ_H,
    CTRL_MEM_CMD__READ_W,
    CTRL_MEM_CMD__READ_BU,
    CTRL_MEM_CMD__READ_HU,
    CTRL_MEM_CMD__NOP,
};

enum opcode_t {
    OPCODE_LUI    = 0b0110111,
    OPCODE_AUIPC  = 0b0010111,
    OPCODE_JAL    = 0b1101111,
    OPCODE_JALR   = 0b1100111,
    OPCODE_LOAD   = 0b0000011,
    OPCODE_STORE  = 0b0100011,
    OPCODE_BRANCH = 0b1100011,
    OPCODE_ARTH_I = 0b0010011,
    OPCODE_ARTH_R = 0b0110011,
};

enum mem_rw_t {
    MEM_RW_READ  = 0,
    MEM_RW_WRITE = 1,
};

enum mem_mode_t {
    MEM_MODE_B  = 0b000,
    MEM_MODE_H  = 0b001,
    MEM_MODE_W  = 0b010,
    MEM_MODE_BU = 0b011,
    MEM_MODE_HU = 0b111,
};

enum br_mode_t {
    BR_MODE_EQ  = 0b000,
    BR_MODE_NE  = 0b001,
    BR_MODE_GE  = 0b100,
    BR_MODE_LT  = 0b101,
    BR_MODE_GEU = 0b110,
    BR_MODE_LTU = 0b111,
};

enum rv32_state_t {
    CPU_STATE_INIT,
    CPU_STATE_IF,
    CPU_STATE_IF_WAIT,
    CPU_STATE_ID,
    CPU_STATE_EX,
    CPU_STATE_MEM,
    CPU_STATE_MEM_WAIT,
    CPU_STATE_WB
};

SC_MODULE(rv32_simple_core) 
{
    public:
    //Interface Ports...
    sc_in_clk    clk;
    sc_in<bool>  reset_n;

    sc_out<bool>      w_imem_valid;
    sc_out<sc_bv<32>> w_imem_addr;
    sc_in <sc_bv<32>> w_imem_rdata;
    sc_in <bool>      w_imem_ready;
    
    sc_out<bool>      w_dmem_valid;
    sc_out<sc_bv<32>> w_dmem_addr;
    sc_out<bool>      w_dmem_rw;
    sc_out<sc_bv<32>> w_dmem_wdata;
    sc_out<sc_bv< 4>> w_dmem_wstrb;
    sc_in <sc_bv<32>> w_dmem_rdata;
    sc_in <bool>      w_dmem_ready;

    //Internal Signals...
    sc_signal<rv32_state_t> state;

    sc_signal<sc_uint<32>> id;
    sc_signal<sc_uint<32>> pc;
    sc_signal<sc_uint<32>> instr;
    uint32_t regs[32];
    sc_signal<sc_uint<32>> rs1;
    sc_signal<sc_uint<32>> rs2;
    sc_signal<sc_uint<32>> imm;
    sc_signal<sc_uint<32>> alu_result;
    sc_signal<sc_uint<32>> alu_result_reg;

    sc_signal<sc_uint<2>> ctrl_alu_src1;
    sc_signal<sc_uint<2>> ctrl_alu_src2;
    sc_signal<sc_uint<8>> ctrl_alu_op;
    sc_signal<sc_uint<5>> ctrl_mem_cmd;
    sc_signal<bool>       ctrl_mem_wr;
    sc_signal<bool>       ctrl_mem_rd;
    sc_signal<sc_uint<2>> ctrl_rd_src;
    sc_signal<sc_uint<2>> ctrl_pc_src;
    sc_signal<bool>       ctrl_branch;

    //Tasks & Functions Declaration...
    sc_uint<7> opcode   () { return instr.read().range( 6, 0); }
    sc_uint<5> rd_addr  () { return instr.read().range(11, 7); }
    sc_uint<3> funct3   () { return instr.read().range(14,12); }
    sc_uint<5> rs1_addr () { return instr.read().range(19,15); }
    sc_uint<5> rs2_addr () { return instr.read().range(24,20); }
    sc_uint<7> funct7   () { return instr.read().range(31,25); }

    //Processes Declaration...
    void process_alu() {
        sc_uint<32> alu_op1;
        switch (ctrl_alu_src1.read()) {
            case CTRL_ALU_SRC__REG : alu_op1 = rs1.read(); break;
            case CTRL_ALU_SRC__IMM : alu_op1 = imm.read(); break;
            case CTRL_ALU_SRC__PC  : alu_op1 = pc.read();  break;
            case CTRL_ALU_SRC__4   : alu_op1 = 4;          break;
        }
        sc_int<32> sgn_op1 = sc_int<32>(alu_op1);

        sc_uint<32> alu_op2;
        switch (ctrl_alu_src2.read()) {
            case CTRL_ALU_SRC__REG : alu_op2 = rs2.read(); break;
            case CTRL_ALU_SRC__IMM : alu_op2 = imm.read(); break;
            case CTRL_ALU_SRC__PC  : alu_op2 = pc.read();  break;
            case CTRL_ALU_SRC__4   : alu_op2 = 4;          break;
        }
        sc_int<32> sgn_op2 = sc_int<32>(alu_op2);

        alu_result = 0;
        switch( ctrl_alu_op.read() ) {
            case CTRL_ALU_OP__NOP : alu_result = (0); break;
            case CTRL_ALU_OP__OP1 : alu_result = (alu_op1); break;
            case CTRL_ALU_OP__OP2 : alu_result = (alu_op2); break;
            case CTRL_ALU_OP__ADD : alu_result = (alu_op1 +  alu_op2); break;
            case CTRL_ALU_OP__SUB : alu_result = (alu_op1 -  alu_op2); break;
            case CTRL_ALU_OP__OR  : alu_result = (alu_op1 |  alu_op2); break;
            case CTRL_ALU_OP__AND : alu_result = (alu_op1 &  alu_op2); break;
            case CTRL_ALU_OP__XOR : alu_result = (alu_op1 ^  alu_op2); break;
            case CTRL_ALU_OP__EQ  : alu_result = (alu_op1 == alu_op2) ? 1 : 0; break;
            case CTRL_ALU_OP__NEQ : alu_result = (alu_op1 != alu_op2) ? 1 : 0; break;
            case CTRL_ALU_OP__LT  : alu_result = (sgn_op1 <  sgn_op2) ? 1 : 0; break;
            case CTRL_ALU_OP__GE  : alu_result = (sgn_op1 >= sgn_op2) ? 1 : 0; break;
            case CTRL_ALU_OP__LTU : alu_result = (alu_op1 <  alu_op2) ? 1 : 0; break;
            case CTRL_ALU_OP__GEU : alu_result = (alu_op1 >= alu_op2) ? 1 : 0; break;
            case CTRL_ALU_OP__SLL : alu_result = (alu_op1 << alu_op2.range(4,0)); break;
            case CTRL_ALU_OP__SRL : alu_result = (alu_op1 >> alu_op2.range(4,0)); break;
            case CTRL_ALU_OP__SRA : alu_result = (sgn_op1 >> alu_op2.range(4,0)); break;
        }
    }

    void process_imm_decode() {
        sc_uint<32> ir = instr.read();

        switch (opcode()) {
            case OPCODE_LUI:
            case OPCODE_AUIPC:  imm = (ir.range(31, 12), sc_uint<12>(0)); break;
            case OPCODE_JAL:    imm = (sc_uint<12>(-ir[31]), ir.range(19, 12), ir[20], ir.range(30, 21), sc_uint<1>(0)); break;
            case OPCODE_JALR:
            case OPCODE_LOAD:
            case OPCODE_ARTH_I: imm = (sc_uint<20>(-ir[31]), ir.range(31, 20)); break;
            case OPCODE_BRANCH: imm = (sc_uint<20>(-ir[31]), ir[7], ir.range(30, 25), ir.range(11, 8), sc_uint<1>(0)); break;
            case OPCODE_STORE:  imm = (sc_uint<20>(-ir[31]), ir.range(31, 25), ir.range(11, 7)); break;
            default:            imm = (sc_uint<32>(0)); break;
        }
    };

    void process_control_gen() {
        ctrl_mem_cmd  = CTRL_MEM_CMD__NOP;
        ctrl_alu_src1 = CTRL_ALU_SRC__REG;
        ctrl_alu_src2 = CTRL_ALU_SRC__REG;
        ctrl_alu_op   = CTRL_ALU_OP__NOP;
        ctrl_rd_src   = CTRL_RD_SRC__NO_WRITE;
        ctrl_pc_src   = CTRL_PC_SRC__PC_NEXT;
        ctrl_mem_wr   = 0;
        ctrl_mem_rd   = 0;
        ctrl_branch   = 0;

        switch (opcode()) {
            case OPCODE_LUI:
                ctrl_alu_src1 = CTRL_ALU_SRC__IMM;
                ctrl_alu_op   = CTRL_ALU_OP__OP1;
                ctrl_rd_src   = CTRL_RD_SRC__ALU_RESULT;
                break;

            case OPCODE_AUIPC:
                ctrl_alu_src1 = CTRL_ALU_SRC__PC;
                ctrl_alu_src2 = CTRL_ALU_SRC__IMM;
                ctrl_alu_op   = CTRL_ALU_OP__ADD;
                ctrl_rd_src   = CTRL_RD_SRC__ALU_RESULT;
                break;

            case OPCODE_JAL:
                ctrl_pc_src   = CTRL_PC_SRC__PC_IMM;
                ctrl_alu_src1 = CTRL_ALU_SRC__PC;
                ctrl_alu_src2 = CTRL_ALU_SRC__4;
                ctrl_alu_op   = CTRL_ALU_OP__ADD;
                ctrl_rd_src   = CTRL_RD_SRC__ALU_RESULT;
                break;

            case OPCODE_JALR:
                ctrl_pc_src   = CTRL_PC_SRC__RS1_IMM;
                ctrl_alu_src1 = CTRL_ALU_SRC__PC;
                ctrl_alu_src2 = CTRL_ALU_SRC__4;
                ctrl_alu_op   = CTRL_ALU_OP__ADD;
                ctrl_rd_src   = CTRL_RD_SRC__ALU_RESULT;
                break;

            case OPCODE_BRANCH:
                ctrl_branch = 1;
                switch (funct3()) {
                    case 0b000: ctrl_alu_op = CTRL_ALU_OP__EQ;  break;
                    case 0b001: ctrl_alu_op = CTRL_ALU_OP__NEQ; break;
                    case 0b100: ctrl_alu_op = CTRL_ALU_OP__LT;  break;
                    case 0b101: ctrl_alu_op = CTRL_ALU_OP__GE;  break;
                    case 0b110: ctrl_alu_op = CTRL_ALU_OP__LTU; break;
                    case 0b111: ctrl_alu_op = CTRL_ALU_OP__GEU; break;
                }
                break;

            case OPCODE_LOAD:
                ctrl_alu_src1 = CTRL_ALU_SRC__REG;
                ctrl_alu_src2 = CTRL_ALU_SRC__IMM;
                ctrl_alu_op   = CTRL_ALU_OP__ADD;
                ctrl_mem_rd   = 1;
                switch (funct3()) {
                    case 0b000: ctrl_mem_cmd = CTRL_MEM_CMD__READ_B;  break;
                    case 0b001: ctrl_mem_cmd = CTRL_MEM_CMD__READ_H;  break;
                    case 0b010: ctrl_mem_cmd = CTRL_MEM_CMD__READ_W;  break;
                    case 0b100: ctrl_mem_cmd = CTRL_MEM_CMD__READ_BU; break;
                    case 0b101: ctrl_mem_cmd = CTRL_MEM_CMD__READ_HU; break;
                }
                ctrl_rd_src   = CTRL_RD_SRC__MEM_DATA;
                break;

            case OPCODE_STORE:
                ctrl_alu_src1 = CTRL_ALU_SRC__REG;
                ctrl_alu_src2 = CTRL_ALU_SRC__IMM;
                ctrl_alu_op   = CTRL_ALU_OP__ADD;
                ctrl_mem_wr   = 1;
                switch (funct3()) {
                    case 0b000: ctrl_mem_cmd = CTRL_MEM_CMD__WRITE_B; break;
                    case 0b001: ctrl_mem_cmd = CTRL_MEM_CMD__WRITE_H; break;
                    case 0b010: ctrl_mem_cmd = CTRL_MEM_CMD__WRITE_W; break;
                }
                break;

            case OPCODE_ARTH_I:
                ctrl_alu_src1 = CTRL_ALU_SRC__REG;
                ctrl_alu_src2 = CTRL_ALU_SRC__IMM;
                switch (funct3()) {
                    case 0b000: ctrl_alu_op = CTRL_ALU_OP__ADD; break;
                    case 0b001: ctrl_alu_op = CTRL_ALU_OP__SLL; break;
                    case 0b010: ctrl_alu_op = CTRL_ALU_OP__LT;  break;
                    case 0b011: ctrl_alu_op = CTRL_ALU_OP__LTU; break;
                    case 0b100: ctrl_alu_op = CTRL_ALU_OP__XOR; break;
                    case 0b101: ctrl_alu_op = funct7()[5] ? CTRL_ALU_OP__SRA : CTRL_ALU_OP__SRL; break;
                    case 0b110: ctrl_alu_op = CTRL_ALU_OP__OR;  break;
                    case 0b111: ctrl_alu_op = CTRL_ALU_OP__AND; break;
                }
                ctrl_rd_src   = CTRL_RD_SRC__ALU_RESULT;
                break;

            case OPCODE_ARTH_R:
                ctrl_alu_src1 = CTRL_ALU_SRC__REG;
                ctrl_alu_src2 = CTRL_ALU_SRC__REG;
                switch (funct3()) {
                    case 0b000: ctrl_alu_op = funct7()[5] ? CTRL_ALU_OP__SUB : CTRL_ALU_OP__ADD; break;
                    case 0b001: ctrl_alu_op = CTRL_ALU_OP__SLL; break;
                    case 0b010: ctrl_alu_op = CTRL_ALU_OP__LT;  break;
                    case 0b011: ctrl_alu_op = CTRL_ALU_OP__LTU; break;
                    case 0b100: ctrl_alu_op = CTRL_ALU_OP__XOR; break;
                    case 0b101: ctrl_alu_op = funct7()[5] ? CTRL_ALU_OP__SRA : CTRL_ALU_OP__SRL; break;
                    case 0b110: ctrl_alu_op = CTRL_ALU_OP__OR;  break;
                    case 0b111: ctrl_alu_op = CTRL_ALU_OP__AND; break;
                }
                ctrl_rd_src   = CTRL_RD_SRC__ALU_RESULT;
                break;
        }
    }

    sc_uint<32> p_mem_data = 0;

    void process_mem_interface() {
        w_imem_addr  = pc.read();

        w_dmem_addr = alu_result.read();
        w_dmem_wdata = rs2.read();
        w_dmem_rw = (ctrl_mem_wr.read()) ? MEM_RW_WRITE : MEM_RW_READ;

        sc_uint<2> dmem_addr_2 = alu_result.read().range(1, 0);
        sc_uint<32> dmem_rdata = w_dmem_rdata.read();

        w_dmem_wstrb = 0b0000;
        if (ctrl_mem_wr.read()) 
        switch (funct3()) {
            case MEM_MODE_W: 
                w_dmem_wstrb = 
                    (dmem_addr_2 == 0b00) ? 0b1111 : 0 ; break;
            case MEM_MODE_H: 
                w_dmem_wstrb = 
                    (dmem_addr_2 == 0b00) ? 0b0011 :
                    (dmem_addr_2 == 0b10) ? 0b1100 : 0 ; break;
            case MEM_MODE_B: 
                w_dmem_wstrb = 
                    (dmem_addr_2 == 0b00) ? 0b0001 :
                    (dmem_addr_2 == 0b01) ? 0b0010 :
                    (dmem_addr_2 == 0b10) ? 0b0100 :
                    (dmem_addr_2 == 0b11) ? 0b1000 : 0 ; break;
        }

        switch (funct3()) {
            case MEM_MODE_W: 
                p_mem_data = 
                    (dmem_addr_2 == 0b00) ? (dmem_rdata.range(31, 0)) : 0; break;
            case MEM_MODE_HU:
                p_mem_data = 
                    (dmem_addr_2 == 0b00) ? (sc_uint<16>(0), dmem_rdata.range(15,  0)) :
                    (dmem_addr_2 == 0b10) ? (sc_uint<16>(0), dmem_rdata.range(31, 16)) : 0; break;
            case MEM_MODE_H: 
                p_mem_data = 
                    (dmem_addr_2 == 0b00) ? (sc_uint<16>(-dmem_rdata[15]), dmem_rdata.range(15,  0)) :
                    (dmem_addr_2 == 0b10) ? (sc_uint<16>(-dmem_rdata[31]), dmem_rdata.range(31, 16)) : 0; break;
            case MEM_MODE_BU:
                p_mem_data = 
                    (dmem_addr_2 == 0b00) ? (sc_uint<24>(0), dmem_rdata.range( 7,  0)) :
                    (dmem_addr_2 == 0b01) ? (sc_uint<24>(0), dmem_rdata.range(15,  8)) :
                    (dmem_addr_2 == 0b10) ? (sc_uint<24>(0), dmem_rdata.range(23, 16)) :
                    (dmem_addr_2 == 0b11) ? (sc_uint<24>(0), dmem_rdata.range(31, 24)) : 0; break;
            case MEM_MODE_B: 
                p_mem_data = 
                    (dmem_addr_2 == 0b00) ? (sc_uint<24>(-dmem_rdata[ 7]), dmem_rdata.range( 7,  0)) :
                    (dmem_addr_2 == 0b01) ? (sc_uint<24>(-dmem_rdata[15]), dmem_rdata.range(15,  8)) :
                    (dmem_addr_2 == 0b10) ? (sc_uint<24>(-dmem_rdata[23]), dmem_rdata.range(23, 16)) :
                    (dmem_addr_2 == 0b11) ? (sc_uint<24>(-dmem_rdata[31]), dmem_rdata.range(31, 24)) : 0; break;
        }
    }

    void process_core() {
        if (!(reset_n.read())) {
            state        = CPU_STATE_INIT;
            id           = 0;
            pc           = 0x00000000;
            instr        = 0x00000000;
            rs1          = 0x00000000;
            rs2          = 0x00000000;
            w_imem_valid = 0;
            w_dmem_valid = 0;
        } else {
            rv32_state_t next_state = state;

            switch (state.read()) {
                case CPU_STATE_INIT: {
                    pc = 0x00000000;
                    w_imem_valid = 1;
                    next_state = CPU_STATE_IF;
                } break;

                case CPU_STATE_IF: {
                    // w_imem_valid = 1;
                    if (w_imem_ready.read() == 1) {
                        next_state = CPU_STATE_ID;
                        instr = w_imem_rdata.read();
                        w_imem_valid = 0;
                    } else {
                        next_state = CPU_STATE_IF;
                    }
                } break;

                case CPU_STATE_ID: {
                    rs1 = regs[rs1_addr()];
                    rs2 = regs[rs2_addr()];
                    next_state = CPU_STATE_EX;
                } break;

                case CPU_STATE_EX: {
                    alu_result_reg = alu_result.read();
                    w_dmem_valid = ctrl_mem_rd.read() || ctrl_mem_wr.read();
                    next_state = CPU_STATE_MEM;
                } break;

                case CPU_STATE_MEM: {
                    // w_dmem_valid = ctrl_mem_rd.read() || ctrl_mem_wr.read();
                    if (w_dmem_valid == 0){
                        next_state = CPU_STATE_WB;
                    } else if (w_dmem_ready.read() == 1) {
                        w_dmem_valid = 0;
                        next_state = CPU_STATE_WB;
                    } else {
                        next_state = CPU_STATE_MEM;
                    }
                } break;

                case CPU_STATE_WB: {
                    // w_dmem_valid = 0;

                    if( ctrl_branch.read() && (alu_result_reg.read() == 1) ) {
                        pc = pc.read() + imm.read();
                    } else {
                        switch( ctrl_pc_src.read() ) {
                            case CTRL_PC_SRC__PC_NEXT : pc = pc.read()  + 4; break;
                            case CTRL_PC_SRC__PC_IMM  : pc = pc.read()  + imm.read(); break;
                            case CTRL_PC_SRC__RS1_IMM : pc = rs1.read() + imm.read(); break; 
                        }
                        id = id.read() + 1;
                    }
                    if( (ctrl_rd_src.read() != CTRL_RD_SRC__NO_WRITE) && (rd_addr() != 0) ) {
                        switch( ctrl_rd_src.read() ) {
                            case CTRL_RD_SRC__MEM_DATA :   regs[rd_addr()] = p_mem_data; break;
                            case CTRL_RD_SRC__ALU_RESULT : regs[rd_addr()] = alu_result_reg.read(); break; 
                        } 
                    }
                    
                    w_imem_valid = 1;
                    next_state = CPU_STATE_IF;
                } break;

                default: 
                    next_state = CPU_STATE_INIT;
            }
            state.write(next_state);
        }
    }

    const char* state_str() {
        switch (state.read()) {
            case CPU_STATE_IF: return "IF";
            case CPU_STATE_ID: return "ID";
            case CPU_STATE_EX: return "EX";
            case CPU_STATE_MEM: return "MM";
            case CPU_STATE_MEM_WAIT: return "MW";
            case CPU_STATE_WB: return "WB";
            default: return "??";
        }
        return "??";
    }

    const char* disassemble() {
        // if (instr == INSTR_NOP) {
        //     return "---  ";
        // }
        switch (opcode()) {
            case OPCODE_LUI:
                return "LUI  ";
            case OPCODE_AUIPC:
                return "AUIPC";
            case OPCODE_JAL:
                return "JAL  ";
            case OPCODE_JALR:
                switch (funct3()) {
                    case 0b000:  return "JALR ";
                }
            case OPCODE_LOAD:
                switch (funct3()) {
                    case MEM_MODE_B:  return "LB   ";
                    case MEM_MODE_H:  return "LH   ";
                    case MEM_MODE_W:  return "LW   ";
                    case MEM_MODE_BU: return "LBU  ";
                    case MEM_MODE_HU: return "LHU  ";
                }
            case OPCODE_STORE:
                switch (funct3()) {
                    case MEM_MODE_B: return "SB   ";
                    case MEM_MODE_H: return "SH   ";
                    case MEM_MODE_W: return "SW   ";
                }
            case OPCODE_BRANCH:
                switch (funct3()) {
                    case BR_MODE_EQ:  return "BEQ  ";
                    case BR_MODE_NE:  return "BNE  ";
                    case BR_MODE_GE:  return "BGE  ";
                    case BR_MODE_LT:  return "BLT  ";
                    case BR_MODE_GEU: return "BGEU ";
                    case BR_MODE_LTU: return "BLTU ";
                }
            case OPCODE_ARTH_I:
                switch (funct3()) {
                    case 0b000:  return "ADDI ";
                    case 0b001:  return "SLLI ";
                    case 0b010:  return "SLTI ";
                    case 0b011:  return "SLTIU";
                    case 0b100:  return "XORI ";
                    case 0b101:  return (funct7()[5] ? "SRAI " : "SRLI ");
                    case 0b110:  return "ORI  ";
                    case 0b111:  return "ANDI ";
                }
            case OPCODE_ARTH_R:
                switch (funct3()) {
                    case 0b000:  return (funct7()[5] ? "SUB  " : "ADD  ");
                    case 0b001:  return "SLL  ";
                    case 0b010:  return "SLT  ";
                    case 0b011:  return "SLTU ";
                    case 0b100:  return "XOR  ";
                    case 0b101:  return (funct7()[5] ? "SRA  " : "SRL  ");
                    case 0b110:  return "OR   ";
                    case 0b111:  return "AND  ";
                }
        }
        return "???  ";
    }

    //Constructor Declaration...
    SC_CTOR(rv32_simple_core) {
        SC_METHOD(process_core);
        sensitive << clk.pos();

        SC_METHOD(process_control_gen);
        sensitive << instr;

        SC_METHOD(process_imm_decode);
        sensitive << instr;
        
        SC_METHOD(process_mem_interface);
        sensitive << pc << alu_result << alu_result_reg << rs2 << ctrl_mem_wr << w_dmem_rdata;

        SC_METHOD(process_alu);
        sensitive 
            << ctrl_alu_op
            << ctrl_alu_src1 << rs1 
            << ctrl_alu_src2 << rs2 
            << imm << pc ;
    }
};

#endif
