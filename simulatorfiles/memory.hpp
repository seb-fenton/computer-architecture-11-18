#ifndef MEMORY_HPP
#define MEMORY_HPP
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <cstdlib>


class sim_reg{
    public:
        sim_reg();
        int get_reg(int regNum) const;
        void set_reg(int input, int regNum);
        void lwl_set_reg(int input, int regNum, int moduAmount);
        void lwr_set_reg(int input, int regNum, int moduAmount);
        int get_hi() const;
        void set_hi(int input);
        int get_lo() const;
        void set_lo(int input);


    private:
        int reg[32];
        int hi;
        int lo;
};

class sim_mem{
    private:
        void set_instruc_byte(int address, char value, bool &success);
        std::vector<char> addr_instr;   //0x1000000
        std::map <int,char> addr_data;  //0x4000000
        char addr_getc[4];
        char addr_putc[4];
        //0x4 to 0x0FFF FFFF            : Blank
        //0x1000 0000 to 0x10FF FFFF    : Instruction Space
        //0x1100 0000 to 0x11FF FFFF    : Blank
        //0x2000 0000 to 0x23FF FFFF    : Data Space
        //0x2400 0000 to 0x2FFF FFFF    : Blank
        //0x3000 0000 to 0x3000 0003    : GetC                  READ ONLY, input via stdin
        //0x3000 0004 to 0x3000 0007    : PutC                  Write Only, output via stdout
        //0x3000 0008 to 0xFFFF FFFF    : Blank

    public:
        sim_mem(int LengthOfBinary, char* Memblock, bool& InputSuccess);

        void io_read();
        void io_write();
        void io_clear();

        int addressmap(int &address);
        char get_byte(int address);
        void set_byte(int address, char value);

};
#endif
