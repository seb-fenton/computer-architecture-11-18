#!/bin/bash

###OPTIONAL INITIALISATION OF TEST DIRECTORY AND FILES IF CSV NEEDED###

#mkdir -p test/output
#touch test/output/output.csv
#touch test/temp.csv
#echo "TestId , Instruction , Status , Author , Message" #>> test/output/output.csv

###TAKES FILE INPUT###                                           
commandline_args=("$@")

###CALLS BINARIES WITH SIMULATOR EXECUTABLE###

###START OF BASIC FUNCTION TESTS###

FILES="test/test_instruction_src/*.txt"
for f in $FILES; do
    bool="fail"

    exec 5< $f                                                  #reads first 4 lines of our txt files to retrieve metadata

    read -r line <&5
    expectedOutcome="${line:1:${#line}-1}"
    read -r line <&5
    testIndex="${line:1:${#line}-1}"
    read -r line <&5
    test="${line:1:${#line}-1}"
    read -r line <&5
    message="${line:1:${#line}-1}"
    
    $commandline_args test/test_instruction_src/$testIndex.bin              #executes next executable
    output=$?
    if [ $output -eq $expectedOutcome ]; then
        bool="pass"
    fi
    echo "$testIndex , $test , $bool , $USER , || Expected outcome: "$expectedOutcome" | Actual outcome: "$output" || $message ||" #>> test/output/output.csv          

    if [ $bool = "fail" ]; then                                #prints in console whether or not particular test has faile
        echo "Test failed: $testIndex, output: $output"                          
    fi
done

###END OF BASIC FUNCTION TESTS###

###START OF BASIC SIMULATOR FUNCTION TESTS###

FILES="test/test_simulator_instruction_src/*.txt"
for f in $FILES; do
    bool="fail"

    exec 5< $f                                                  #reads first 4 lines of our txt files to retrieve metadata

    read -r line <&5
    expectedOutcome="${line:1:${#line}-1}"
    read -r line <&5
    testIndex="${line:1:${#line}-1}"
    read -r line <&5
    test="${line:1:${#line}-1}"
    read -r line <&5
    message="${line:1:${#line}-1}"
    
    $commandline_args test/test_simulator_instruction_src/$testIndex.bin              #executes next executable
    output=$?
    if [ $output -eq $expectedOutcome ]; then
        bool="pass"
    fi
    echo "$testIndex , $test , $bool , $USER , || Expected outcome: "$expectedOutcome" | Actual outcome: "$output" || $message ||" #>> test/output/output.csv          

    if [ $bool = "fail" ]; then                                #prints in console whether or not particular test has faile
        echo "Test failed: $testIndex, output: $output"                          
    fi
done

###END OF BASIC SIMULATOR FUNCTION TESTS###

###START OF OUTPUT FUNCTION TESTS###

FILES="test/test_io_instruction_src/*.txt"
for f in $FILES; do
    bool="fail"

    exec 5< $f                                                  #reads first 4 lines of our txt files to retrieve metadata

    read -r line <&5
    expectedOutcome="${line:1:${#line}-1}"
    read -r line <&5
    testIndex="${line:1:${#line}-1}"
    read -r line <&5
    test="${line:1:${#line}-1}"
    read -r line <&5
    message="${line:1:${#line}-1}"

    output=$($commandline_args test/test_io_instruction_src/$testIndex.bin)           #executes next executable

    if [ "$output" = "$expectedOutcome" ]; then
        bool="pass"
    fi

    echo "$testIndex , $test , $bool , $USER , || Expected outcome: "$expectedOutcome" | Actual outcome: "$output" || $message ||" #>> test/output/output.csv

    if [ $bool = "fail" ]; then                                #prints in console whether or not particular test has failed
        echo "Test failed: $testIndex, output: $output"                          
    fi
done

###END OF OUTPUT FUNCTION TESTS###

###START OF 'MANUAL' (aka INPUT) FUNCTION TESTS###

FILES="test/test_io_instruction_src_manual/*.txt"
for f in $FILES; do
    bool="fail"

    exec 5< $f                                                  #reads first 4 lines of our txt files to retrieve metadata

    read -r line <&5
    expectedOutcome="${line:1:${#line}-1}"
    read -r line <&5
    testIndex="${line:1:${#line}-1}"
    read -r line <&5
    test="${line:1:${#line}-1}"
    read -r line <&5
    message="${line:1:${#line}-1}"
    read -r line <&5
    expectedInput="${line:1:${#line}-1}"

    if [ "$expectedInput" = "0" ]; then
        echo "0" | $commandline_args test/test_io_instruction_src_manual/$testIndex.bin
        output=$?
    fi

    if [ "$expectedInput" = "D" ]; then
        echo "D" | $commandline_args test/test_io_instruction_src_manual/$testIndex.bin
        output=$?
    fi

    if [ "$expectedInput" = "E" ]; then
        echo "E" | $commandline_args test/test_io_instruction_src_manual/$testIndex.bin
        output=$?
    fi

    if [ "$expectedInput" = "F" ]; then
        echo "F" | $commandline_args test/test_io_instruction_src_manual/$testIndex.bin
        output=$?
    fi

    if [ "$expectedInput" = "G" ]; then
        echo "G" | $commandline_args test/test_io_instruction_src_manual/$testIndex.bin
        output=$?
    fi

    if [ "$expectedInput" = "H" ]; then
        echo "H" | $commandline_args test/test_io_instruction_src_manual/$testIndex.bin
        output=$?
    fi



    if [ "$output" = "$expectedOutcome" ]; then
        bool="pass"
    fi

    echo "$testIndex , $test , $bool , $USER , || Expected outcome: "$expectedOutcome" | Actual outcome: "$output" || $message ||" #>> test/output/output.csv

    if [ $bool = "fail" ]; then                                #prints in console whether or not particular test has failed
        echo "Test failed: $testIndex, output: $output"                          
    fi
done

#SUBSECTION: START OF NO_INPUT AND EOF TESTS#

$commandline_args
output=$?
bool="fail"

if [ $output -eq 235 ]; then
    bool="pass"
fi

echo "noinput , nop , $bool , $USER , || Expected outcome: 235 | Actual outcome: $output || Testing no-input error || Dependencies: ||" #>> test/output/output.csv

cat test/eof.txt | $commandline_args test/test_io_instruction_src_manual/lhio1.bin
output=$?
bool="fail"

if [ $output -eq 255 ]; then
    bool="pass"
fi

echo "eof1 , lh , $bool , $USER , || Expected outcome: 255 | Actual outcome: $output || Testing eof output || Dependencies: lui, jr ||" #>> test/output/output.csv

cat test/eof.txt | $commandline_args test/test_io_instruction_src_manual/lwio1.bin
output=$?
bool="fail"

if [ $output -eq 255 ]; then
    bool="pass"
fi

echo "eof2 , lw , $bool , $USER , || Expected outcome: 255 | Actual outcome: $output || Testing eof output || Dependencies: lui, jr ||" #>> test/output/output.csv

#SUBSECTION: END OF NO_INPUT AND EOF TESTS#

###END OF 'MANUAL' (aka INPUT) FUNCTION TESTS###