#include "simulator.hpp"

simulator::simulator(int LengthOfBinary, char* Memblock, bool& InputSuccess) : memory(LengthOfBinary, Memblock, InputSuccess){
    //set program controls
    programCounter = 0x10000000;
    pcOffSet = 0;

    //set branch/jump variables
    jump = false;
    branch = false;
    delayedBranch = false;
    delayedJump = false;
}
//------CPU Control Methods------//
    bool simulator::finished_sim(){
        //condition 1: pc has jumped to zero
        if(programCounter == 0) return true;
        else                    return false;
    }
    void simulator::updatePC(){
        programCounter = programCounter + 4;

        if(branch || jump){
            if(branch)      delayedBranch = true;       //stage delayed branch/jump for next cycle
            else if(jump)   delayedJump = true;         //redundant elif in case something is BOTH a branch and jump(impossible)

            branch = false;
            jump = false;
            return;
        }

        else if(delayedBranch || delayedJump){
            if(delayedBranch) programCounter = programCounter + pcOffSet - 4;
            else if(delayedJump)    programCounter = pcOffSet;

            delayedBranch = false;
            delayedJump = false;
            pcOffSet = 0;
        }
    }
    void simulator::update_exit_code(int& exitCode){
        exitCode = (regFile.get_reg(2)) & 0xFF;
    }
    int simulator::fetch(){
        int check = programCounter;               //CHECK that PC is in an executable area
        check = memory.addressmap(check);
        int instruction = 0;
        if((check > 1) || (programCounter%4 != 0) || (check == -1))  std::exit(-11);         //11: executing ADDRESS that cannot be executed. different from 12

        else{
            for(int i=0; i<4; i++){                     //fetch and append 4 bytes to create a full 32 byte instruction
                int temp = memory.get_byte((programCounter + i));
                temp = temp & 0xFF;
                temp = temp << (8*(3-i));
                instruction = instruction | temp;
            }
            return instruction;
        }
    }
    int simulator::decode(int instruction){
        unsigned int opcode = ((unsigned int)instruction >> 26) & 0x3F;
        switch(opcode){
            //--------R Instructions--------//
            case 0b000000: return simulator::r_classification(instruction);
            //--------J Instructions--------//
            case 0b000010: return 17;   //j
            case 0b000011: return 19;   //jal
            //--------I Instructions--------//
            case 0b001000: return 2;    //addi
            case 0b001001: return 3;    //addiu

            case 0b001100: return 6;    //andi

            case 0b000100: return 7;    //beq

            case 0b000001: return simulator::branch_classification(instruction);             //bgez, bgezal, bltz, bltzal

            case 0b000111: return 10;   //bgtz
            case 0b000110: return 11;   //blez

            case 0b000101: return 14;   //bne

            case 0b100000: return 21;   //lb
            case 0b100100: return 22;   //lbu
            case 0b100001: return 23;   //lh
            case 0b100101: return 24;   //lhu
            case 0b001111: return 25;   //lui
            case 0b100011: return 26;   //lw
            case 0b100010: return 27;   //lwl
            case 0b100110: return 28;   //lwr

            case 0b001101: return 36;   //ori

            case 0b101000: return 37;   //sb
            case 0b101001: return 38;   //sh

            case 0b001010: return 42;   //slti
            case 0b001011: return 43;   //sltiu
            case 0b101011: return 51;   //sw

            case 0b001110: return 53;   //xori

            default:    std::cerr<<"Instruction decoding failed - invalid instruction. Exiting program...\n";
                        std::exit(-12);
                        break;

        }
    }
    int simulator::r_classification(int instruction){
        instruction = instruction & 0b111111;
        switch(instruction){
            case 0b100000: return 1;    //ADD

            case 0b100001: return 4;    //ADDU

            case 0b100100: return 5;    //AND

            case 0b011010: return 15;   //DIV

            case 0b011011: return 16;   //DIVU

            case 0b001001: return 18;   //JALR

            case 0b001000: return 20;   //JR

            case 0b010000: return 29;   //MFHI

            case 0b010010: return 30;   //MFLO

            case 0b010001: return 31;   //MTHI

            case 0b010011: return 32;   //MTLO

            case 0b011000: return 33;   //MULT

            case 0b011001: return 34;   //MULTU

            case 0b100101: return 35;   //OR

            case 0b000000: return 39;   //SLL

            case 0b000100: return 40;   //SLLV

            case 0b101010: return 41;   //SLT

            case 0b101011: return 44;   //SLTU

            case 0b000011: return 45;   //SRA

            case 0b000111: return 46;   //SRAV

            case 0b000010: return 47;   //SRL

            case 0b000110: return 48;   //SRLV

            case 0b100010: return 49;   //SUB

            case 0b100011: return 50;   //SUBU

            case 0b100110: return 52;   //XOR

            default:    std::cerr<<"R_instruction decoding failed - invalid instruction. Exiting program...";
                        std::exit(-12);
                        break;
        }
    }
    int simulator::branch_classification(int instruction){

        instruction = instruction >> 16;
        int destination = instruction & 0b11111;

        switch(destination){
            case 0b00001: return 8;  //BGEZ
            case 0b10001: return 9;  //BGEZAL
            case 0b00000: return 12;  //BLTZ
            case 0b10000: return 13;  //BLTZAL
            default:    std::cerr<<"Branch instruction decoding failed - invalid instruction. Exiting program...";
                        std::exit(-12);
        }
    }
    void simulator::execute(int index, int instruction){
        switch(index){
        //--------R Instructions--------//
            case 1: simulator::r_add(instruction);
                break;
            case 4: simulator::r_addu(instruction);
                break;
            case 5: simulator::r_and(instruction);
                break;
            case 15: simulator::r_div(instruction);
                break;
            case 16: simulator::r_divu(instruction);
                break;
            case 18: simulator::r_jalr(instruction);
                break;
            case 20: simulator::r_jr(instruction);
                break;
            case 29: simulator::r_mfhi(instruction);
                break;
            case 30: simulator::r_mflo(instruction);
                break;
            case 31: simulator::r_mthi(instruction);
                break;
            case 32: simulator::r_mtlo(instruction);
                break;
            case 33: simulator::r_mult(instruction);
                break;
            case 34: simulator::r_multu(instruction);
                break;
            case 35: simulator::r_or(instruction);
                break;
            case 39: simulator::r_sll(instruction);
                break;
            case 40: simulator::r_sllv(instruction);
                break;
            case 41: simulator::r_slt(instruction);
                break;
            case 44: simulator::r_sltu(instruction);
                break;
            case 45: simulator::r_sra(instruction);
                break;
            case 46: simulator::r_srav(instruction);
                break;
            case 47: simulator::r_srl(instruction);
                break;
            case 48: simulator::r_srlv(instruction);
                break;
            case 49: simulator::r_sub(instruction);
                break;
            case 50: simulator::r_subu(instruction);
                break;
            case 52: simulator::r_xor(instruction);
                break;

        //--------I Instructions--------//
            case 2: simulator::i_addi(instruction);
                break;
            case 3: simulator::i_addiu(instruction);
                break;
            case 6: simulator::i_andi(instruction);
                break;
            case 7: simulator::i_beq(instruction);
                break;
            case 8: simulator::i_bgez(instruction);
                break;
            case 9: simulator::i_bgezal(instruction);
                break;
            case 10: simulator::i_bgtz(instruction);
                break;
            case 11: simulator::i_blez(instruction);
                break;
            case 12: simulator::i_bltz(instruction);
                break;
            case 13: simulator::i_bltzal(instruction);
                break;
            case 14: simulator::i_bne(instruction);
                break;
            case 21: simulator::i_lb(instruction);
                break;
            case 22: simulator::i_lbu(instruction);
                break;
            case 23: simulator::i_lh(instruction);
                break;
            case 24: simulator::i_lhu(instruction);
                break;
            case 25: simulator::i_lui(instruction);
                break;
            case 26: simulator::i_lw(instruction);
                break;
            case 27: simulator::i_lwl(instruction);
                break;
            case 28: simulator::i_lwr(instruction);
                break;
            case 36: simulator::i_ori(instruction);
                break;
            case 37: simulator::i_sb(instruction);
                break;
            case 38: simulator::i_sh(instruction);
                break;
            case 42: simulator::i_slti(instruction);
                break;
            case 43: simulator::i_sltiu(instruction);
                break;
            case 51: simulator::i_sw(instruction);
                break;
            case 53: simulator::i_xori(instruction);
                break;

            //--------J Instructions--------//
            case 17: simulator::j_j(instruction);
                break;
            case 19: simulator::j_jal(instruction);
                break;
        }
    }


//--------R Instructions--------//
    void simulator::r_parse(int instruction, int& rs, int& rt, int& rd){
        rs = (instruction >> 21) & 0x1F;
        rt = (instruction >> 16) & 0x1F;
        rd = (instruction >> 11) & 0x1F;
    }
    void simulator::r_parse(int instruction, int& rs, int& rt, int& rd, int& shamt){
        rs = (instruction >> 21) & 0x1F;
        rt = (instruction >> 16) & 0x1F;
        rd = (instruction >> 11) & 0x1F;
        shamt = (instruction >> 6) & 0x1F;
    }
    void simulator::r_add(int instruction){
        bool overflow = false;
        int rs,rt,rd,shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if(shamt != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        int result = rt+rs;

        //if both operands are same sign, set overflow if result sign is different
        if(((rs >> 31) == (rt >>31)) && (result>>31 != rs>>31)) overflow = true;

        if(overflow)    std::exit(-10);
        else            regFile.set_reg(result, rd);
    }
    void simulator::r_addu(int instruction){
        int rs,rt,rd,shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if(shamt != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        regFile.set_reg(rt+rs, rd);             //no overflow
    }
    void simulator::r_and(int instruction){
        int rs,rt,rd,shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if(shamt != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        regFile.set_reg(rt & rs, rd);
    }
    void simulator::r_div(int instruction){
        int rs,rt,rd, shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if((rd != 0) && (shamt != 0)) std::exit(-12);
        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        if(rt != 0){
            regFile.set_lo(rs/rt);//quotient into LO
            regFile.set_hi(rs%rt);//remainder into HI
        }
    }
    void simulator::r_divu(int instruction){
        int rs,rt,rd, shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if((rd != 0) && (shamt != 0)) std::exit(-12);

        rs = regFile.get_reg(rs);               //src1
        rt = regFile.get_reg(rt);               //src2

        if(rt != 0){
            regFile.set_lo((uint32_t)rs/(uint32_t)rt);                  //quotient into LO
            regFile.set_hi((uint32_t)rs%(uint32_t)rt);                  //remainder into HI
        }
    }
    void simulator::r_jalr(int instruction){ //CHECK
        int rs, rt, rd, hint;
        r_parse(instruction, rs, rt, rd, hint);
        rs = regFile.get_reg(rs);                   //fetch register value
        if(rd == 0) rd = 31;

        if((rt+hint) != 0)  std::exit(-12);

        jump = true;                                //set jump (for delayed branching)
        pcOffSet = rs;                              //set regval into pcOffSet
        regFile.set_reg(programCounter+8, rd);
    }
    void simulator::r_jr(int instruction){
        int rs, rt, rd, hint;
        r_parse(instruction, rs, rt, rd, hint);
        rs = regFile.get_reg(rs);                   //fetch register value

        if((rt+rd+hint) != 0)  std::exit(-12);

        jump = true;                                //set jump (for delayed branching)
        pcOffSet = rs;                              //set regval into pcOffSet
    }
    void simulator::r_mfhi(int instruction){
        int rs, rt, rd, shamt;
        r_parse(instruction, rs, rt, rd, shamt);

        if((rs+rt+shamt) != 0)  std::exit(-12);
        regFile.set_reg(regFile.get_hi(), rd);        //destination
    }
    void simulator::r_mflo(int instruction){
      int rs, rt, rd, shamt;
      r_parse(instruction, rs, rt, rd, shamt);

      if((rs+rt+shamt) != 0)  std::exit(-12);

        regFile.set_reg(regFile.get_lo(), rd);        //destination
    }
    void simulator::r_mthi(int instruction){
        int rs, rt, rd, shamt;
        r_parse(instruction, rs, rt, rd, shamt);
        if((rt+rd+shamt) != 0)  std::exit(-12);

        regFile.set_hi(regFile.get_reg(rs));        //destination
    }
    void simulator::r_mtlo(int instruction){
        int rs, rt, rd, shamt;
        r_parse(instruction, rs, rt, rd, shamt);
        if((rt+rd+shamt) != 0)  std::exit(-12);

        regFile.set_lo(regFile.get_reg(rs));        //destination
    }
    void simulator::r_mult(int instruction){
        int rs,rt,rd,shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if((rd+shamt) != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        int64_t result = (int64_t)rt*(int64_t)rs;
        int tophalf = (int32_t)(result>>32);
        int btmhalf = (int32_t)(result&0xffffffff);

        regFile.set_hi(tophalf);
        regFile.set_lo(btmhalf);
    }
    void simulator::r_multu(int instruction){
        int rs,rt,rd,shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if((rd+shamt) != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        uint64_t result = (uint64_t)(uint32_t)rt*(uint64_t)(uint32_t)rs;
        uint32_t tophalf = (uint32_t)(result>>32);
        uint32_t btmhalf = (uint32_t)(result&0xffffffff);

        regFile.set_hi(tophalf);
        regFile.set_lo(btmhalf);
    }
    void simulator::r_or(int instruction){
        int rs,rt,rd,shamt;
        r_parse(instruction,rs,rt,rd,shamt);

        if(shamt != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);               //src1
        rt = regFile.get_reg(rt);               //src2

        regFile.set_reg((rs|rt), rd);
    }
    void simulator::r_slt(int instruction){
        int rs, rt, rd, shamt;
        r_parse(instruction, rs, rt, rd, shamt);

        if(shamt != 0)  std::exit(-12);

        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        int comparison;
        if(rs<rt)   comparison = 1;
        else        comparison = 0;

        regFile.set_reg(comparison, rd);
    }
    void simulator::r_sltu(int instruction){
        int rs, rt, rd, shamt;
        r_parse(instruction, rs, rt, rd, shamt);

        if(shamt != 0)  std::exit(-12);
        uint32_t rsu = (uint32_t)regFile.get_reg(rs);
        uint32_t rtu = (uint32_t)regFile.get_reg(rt);

        int comparison;

        if(rsu<rtu)     comparison = 1;
        else            comparison = 0;

        regFile.set_reg(comparison, rd);
    }
    void simulator::r_sll(int instruction){
        int rs, rt, rd, sa;
        r_parse(instruction, rs,rt,rd,sa);

        if(rs != 0) std::exit(-12);

        rt = regFile.get_reg(rt);

        int result = rt << sa;
        regFile.set_reg(result, rd);
    }
    void simulator::r_sllv(int instruction){
        int rs, rt, rd, sa;
        r_parse(instruction, rs,rt,rd, sa);

        if(sa != 0) std::exit(-12);

        rs = regFile.get_reg(rs);
        rs = rs & 0x1F;
        rt = regFile.get_reg(rt);

        int result = rt << rs;
        regFile.set_reg(result, rd);
    }
    void simulator::r_sra(int instruction){
        int rs, rt, rd, sa;
        r_parse(instruction, rs, rt, rd, sa);

        if(rs != 0) std::exit(-12);

        rt = regFile.get_reg(rt);

        rt = rt>>sa;
        regFile.set_reg(rt, rd);
    }
    void simulator::r_srav(int instruction){    //g++ compiler does arithmetic shift for signed data types
        int rs, rt, rd, sa;
        r_parse(instruction, rs, rt, rd, sa);
        if(sa != 0) std::exit(-12);

        rs = regFile.get_reg(rs);
        rs = rs & 0x1F; 	        //last 5 bits of rs

        rt = regFile.get_reg(rt);   //val of rt

        rt = rt>>rs;                //shifted val of rt
        regFile.set_reg(rt, rd);    //into rd
    }
    void simulator::r_srl(int instruction){
        int rs, rt, rd, sa;
        r_parse(instruction, rs,rt,rd,sa);
        if(rs != 0) std::exit(-12);

        rt = regFile.get_reg(rt);

        unsigned int result = (unsigned)rt >> (unsigned)sa; //try removing unsigned casting for sa!
        regFile.set_reg(result, rd);
    }
    void simulator::r_srlv(int instruction){    //FIXED, RS WAS MISSING 5 BIT MASK
        int rs, rt, rd, sa;
        r_parse(instruction, rs,rt,rd, sa);
        if(sa != 0) std::exit(-12);

        rt = regFile.get_reg(rt);
        rs = regFile.get_reg(rs) & 0x1F;

        unsigned int result = (unsigned)rt >> (unsigned)rs; //try removing unsigned casting for rs!

        regFile.set_reg(result, rd);
    }
    void simulator::r_sub(int instruction){
        bool overflow = false;
        int rs,rt,rd, sa;
        r_parse(instruction,rs,rt,rd, sa);
        if(sa != 0) std::exit(-12);

        rs = regFile.get_reg(rs);               //src1
        rt = regFile.get_reg(rt);               //src2

        int result = rs-rt;
        //if both operands are of different signs, set overflow if result sign is different from first operand
        if(((rs >> 31) != (rt >>31)) && (result>>31 != rs>>31)) overflow = true;
        if(overflow)    std::exit(-10);
        else    regFile.set_reg(result, rd);
    }
    void simulator::r_subu(int instruction){
        int rs,rt,rd, sa;
        r_parse(instruction,rs,rt,rd,sa);

        if(sa != 0) std::exit(-12);
        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        regFile.set_reg(rs-rt, rd);
    }
    void simulator::r_xor(int instruction){
        int rs,rt,rd, sa;
        r_parse(instruction,rs,rt,rd,sa);
        if(sa != 0) std::exit(-12);

        rs = regFile.get_reg(rs);               //src1
        rt = regFile.get_reg(rt);               //src2

        regFile.set_reg((rs^rt), rd);
    }

//--------I Instructions--------//
    void simulator::i_parse(int instruction, int& rs, int& rt, int& imm){
        rs = (instruction >> 21) & 0x1F;    //index number of rs
        rt = (instruction >> 16) & 0x1F;    //index number of rt
        imm = instruction & 0xFFFF;         //zero extended by default
    }
    int simulator::sign_extend(int imm){   //use if you want a signed value
        int output;
        if((imm>>15) == 1)     output = 0xFFFF0000 | imm;
        else                   output = imm;
        return output;
    }
    void simulator::i_addi(int instruction){
        bool overflow = false;
        int rs, rt, imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm);
        rs = regFile.get_reg(rs);               //value of rs

        int result = rs + imm;
        //if both operands are same sign, set overflow if result sign is different
        if(((rs >> 31) == (imm >> 31)) && (result>>31 != rs>>31))   overflow = true;

        if(overflow)    std::exit(-10);
        else            regFile.set_reg(result, rt);
    }
    void simulator::i_addiu(int instruction){
        int rs, rt, imm;
        i_parse(instruction, rs, rt, imm);
        rs = regFile.get_reg(rs);
        imm = sign_extend(imm);

        int result = rs + imm;
        regFile.set_reg(result, rt);
    }
    void simulator::i_andi(int instruction){
        int rs, rt, imm;
        i_parse(instruction, rs, rt, imm);
        rs = regFile.get_reg(rs);

        regFile.set_reg(rs & imm, rt);
    }
    void simulator::i_beq(int instruction){
        int rs,rt, imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;

        rs = regFile.get_reg(rs);               //src1
        rt = regFile.get_reg(rt);               //src2

        if(rs == rt){
            branch = true;
            pcOffSet = imm;
        }
    }
    void simulator::i_bgez(int instruction){
        int rs,rt, imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);

        if(rs >= 0){
            branch = true;
            pcOffSet = imm;
        }
    }
    void simulator::i_bgezal(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);                   //src1

        if(rs >= 0){
            branch = true;                          //branch
            pcOffSet = imm;
        }
        regFile.set_reg(programCounter+8, 31);
    }
    void simulator::i_bgtz(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);                   //src1

        if(rs > 0){
            branch = true;
            pcOffSet = imm;
        }
    }
    void simulator::i_blez(int instruction){
        int rs,rt, imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);

        if(rs <= 0){
            branch = true;
            pcOffSet = imm;
        }
    }
    void simulator::i_bltz(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);

        if(rs < 0){
            branch = true;
            pcOffSet = imm;
        }
    }
    void simulator::i_bltzal(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);

        if(rs < 0){
            branch = true;
            pcOffSet = imm;
        }
        regFile.set_reg(programCounter+8, 31);
    }
    void simulator::i_bne(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm) << 2;
        rs = regFile.get_reg(rs);
        rt = regFile.get_reg(rt);

        if(rs != rt){
            branch = true;
            pcOffSet = imm;
        }
    }
    void simulator::i_lb(int instruction){
        int base, rt, imm;
        i_parse(instruction, base, rt, imm);
        imm = sign_extend(imm);
        base = regFile.get_reg(base);

        int memoryAddress = base + imm;
        if (memoryAddress >= 0x30000000 && memoryAddress <= 0x30000003) memory.io_read();

        int output = (int32_t)memory.get_byte(memoryAddress);

        regFile.set_reg(output, rt);
        memory.io_clear();
    }
    void simulator::i_lbu(int instruction){
        int base, rt, imm;
        i_parse(instruction, base, rt, imm);
        imm = sign_extend(imm);
        base = regFile.get_reg(base);

        int memoryAddress = base + imm;

        if (memoryAddress >= 0x30000000 && memoryAddress <= 0x30000003) memory.io_read();

        int output = (uint32_t)(unsigned char)memory.get_byte(memoryAddress);

        regFile.set_reg(output, rt);
        memory.io_clear();
    }
    void simulator::i_lh(int instruction){
        int base, rt, imm;
        i_parse(instruction, base, rt, imm);
        imm = sign_extend(imm);
        base = regFile.get_reg(base);

        int memoryAddress = base + imm;

        if ((memoryAddress % 2) != 0)   std::exit(-11);
        if (memoryAddress == 0x30000000 || memoryAddress == 0x30000002) memory.io_read();

        int hword = (int32_t)memory.get_byte(memoryAddress);
        hword = hword << 8;
        hword = hword | (uint32_t)(unsigned char)memory.get_byte(memoryAddress + 1);

        regFile.set_reg(hword, rt);
        memory.io_clear();
    }
    void simulator::i_lhu(int instruction){
        int base, rt, imm;
        i_parse(instruction, base, rt, imm);
        imm = sign_extend(imm);
        base = regFile.get_reg(base);

        int memoryAddress = base + imm;

        if ((memoryAddress % 2) != 0)   std::exit(-11);
        if (memoryAddress == 0x30000000 || memoryAddress == 0x30000002) memory.io_read();

        uint32_t hword = (uint32_t)(unsigned char)memory.get_byte(memoryAddress);
        hword = hword << 8;
        hword = hword | (uint32_t)(unsigned char)memory.get_byte(memoryAddress + 1);

        regFile.set_reg(hword, rt);
        memory.io_clear();
    }
    void simulator::i_lui(int instruction){
        int rs, rt, imm;
        i_parse(instruction, rs, rt, imm);
        if(rs != 0) std::exit(-12);

        imm = (imm << 16) & 0xFFFF0000;

        regFile.set_reg(imm, rt);
    }
    void simulator::i_lw(int instruction){
        int base, rt, offset;
        i_parse(instruction, base, rt, offset);
        offset = sign_extend(offset);
        base = regFile.get_reg(base);

        int memoryAddress = base + offset;
        if ((memoryAddress % 4) != 0)   std::exit(-11);

        if (memoryAddress == 0x30000000)    memory.io_read();

        int input = (uint32_t)(unsigned char)memory.get_byte(memoryAddress);
        input <<= 8;

        input |= (uint32_t)(unsigned char)memory.get_byte(memoryAddress + 1);
        input <<= 8;

        input |= (uint32_t)(unsigned char)memory.get_byte(memoryAddress + 2);
        input <<= 8;

        input |= (uint32_t)(unsigned char)memory.get_byte(memoryAddress + 3);

        regFile.set_reg(input, rt);
        memory.io_clear();
    }
    void simulator::i_lwl(int instruction){
        signed short int offset = instruction & 0xFFFF;

        int base = (instruction >> 21) & 0x1F;
        base = regFile.get_reg(base);

        int address = base + offset;

        int rt = (instruction >> 16) & 0x1F;

        int moduAmount = address % 4;
        int input = 0;

        if (address >= 0x30000000 && address <= 0x30000003)
            memory.io_read();

        for (int i = 3 - moduAmount; i >= 0; i--)
        {
            int temp = (unsigned char)memory.get_byte(address + i);
            temp <<= 8 * (3 - i);
            input |= temp;
        }

        regFile.lwl_set_reg(input, rt, moduAmount);
        memory.io_clear();
    }
    void simulator::i_lwr(int instruction){
        int base, rt, imm;
        i_parse(instruction, base, rt, imm);
        base = regFile.get_reg(base);
        imm = sign_extend(imm);

        int address = base + imm;

        int moduAmount = address % 4;
        int input = 0;

        if (address >= 0x30000000 && address <= 0x30000003)
            memory.io_read();

        for (int i = moduAmount; i >= 0; i--)
        {
            int temp = (int32_t)(unsigned char)memory.get_byte(address - i);
            temp <<= 8 * (i);
            input |= temp;
        }

        regFile.lwr_set_reg(input, rt, moduAmount);
        memory.io_clear();
    }
    void simulator::i_ori(int instruction){
        int rs,rt,imm;
        i_parse(instruction,rs,rt,imm);
        rs = regFile.get_reg(rs);

        regFile.set_reg((rs|imm), rt);
    }
    void simulator::i_sb(int instruction){
        int base, rt, imm;
        i_parse(instruction, base, rt, imm);
        imm = sign_extend(imm);

        base = regFile.get_reg(base);    //value of base
        rt = regFile.get_reg(rt) & 0xFF; //value of rt (LS 8 bits)
        int effectiveAddr = base + imm;  //effective address

        memory.set_byte(effectiveAddr, rt);

        if (effectiveAddr >= 0x30000004 && effectiveAddr < 0x30000008)
            memory.io_write();
    }
    void simulator::i_sh(int instruction){
        int base, rt, offset;
        i_parse(instruction, base, rt, offset);
        offset = sign_extend(offset);

        base = regFile.get_reg(base);
        rt = regFile.get_reg(rt) & 0xFFFF;

        int effectiveAddr = base + offset;

        if ((effectiveAddr % 2) != 0)
            std::exit(-11);

        char msb = (rt >> 8) & 0xFF; //msb to be loaded into memory first
        memory.set_byte(effectiveAddr, msb);

        char lsb = rt & 0xFF; //lsb then loaded into memory
        memory.set_byte((effectiveAddr + 1), lsb);

        if (effectiveAddr == 0x30000004 || effectiveAddr == 0x30000006)
            memory.io_write();
    }
    void simulator::i_sw(int instruction){
        int base, rt, offset;
        i_parse(instruction, base, rt, offset);
        offset = sign_extend(offset);
        base = regFile.get_reg(base);
        rt = regFile.get_reg(rt);

        int memoryAddress = base + offset;

        if ((memoryAddress % 4) != 0)
            std::exit(-11);

        unsigned char byte = ((rt >> 24) & 0xFF);
        memory.set_byte((memoryAddress), byte);

        byte = ((rt >> 16) & 0xFF);
        memory.set_byte((memoryAddress + 1), byte);

        byte = ((rt >> 8) & 0xFF);
        memory.set_byte((memoryAddress + 2), byte);

        byte = ((rt & 0xFF));
        memory.set_byte((memoryAddress + 3), byte);

        if (memoryAddress == 0x30000004)
            memory.io_write();
    }
    void simulator::i_slti(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        imm = sign_extend(imm);
        rs = regFile.get_reg(rs);

        int comparison;
        if(rs<imm)  comparison = 1;
        else        comparison = 0;

        regFile.set_reg(comparison, rt);
    }
    void simulator::i_sltiu(int instruction){
        int rs,rt,imm;
        i_parse(instruction, rs, rt, imm);
        rs = regFile.get_reg(rs);
        imm = sign_extend(imm);

        int comparison;
        if((uint32_t)rs<(uint32_t)imm)    comparison = 1;
        else                comparison = 0;

        regFile.set_reg(comparison, rt);
    }
    void simulator::i_xori(int instruction){
        int rs, rt, imm;
        i_parse(instruction, rs, rt, imm);
        rs = regFile.get_reg(rs);

        regFile.set_reg((rs^imm), rt);
    }

//--------J Instructions--------//
    void simulator::j_j(int instruction){
        int instr = (instruction & 0x03FFFFFF) << 2; //extract lower 26 bits
        jump = true;
        pcOffSet = instr | ((programCounter+4) & 0xF0000000);
    }

    void simulator::j_jal(int instruction){
        simulator::j_j(instruction);            //jump
        regFile.set_reg(programCounter+8, 31);  //and link
    }
