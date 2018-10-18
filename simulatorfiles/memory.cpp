#include "memory.hpp"
#include <iostream>
using namespace std;

//SIMULATOR_REGISTERS FUNCTION DEFINITIONS//

//Constructor to initialise all register values to 0
simulator_registers::simulator_registers(){
    for(int i = 0; i<31; i++){
        registers[i] = 0;
    }
}

char simulator_registers::get_register(int address){
    char output = registers[address];
    return output;
}

void simulator_registers::set_register(char input, int address){
    if(input == 0 || 1 || 26 || 27){
        cerr<<"Fatal error encountered: exit code -11";
        exit; 
    }
    else if(input<32 && input>-1){
        registers[address] = input;
    }
    else{
        cerr<<"Fatal error encountered: exit code -11";
        exit;
    }
}

//SIMULATOR_MEMORY FUNCTION DEFINITIONS//