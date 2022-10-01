#include <iostream>


#include "catch.hpp"
#include "chip32_assembler.h"

void hexdump(void *ptr, int buflen);

static const std::string test1 = R"(; jump over the data, to our entry label
    jump         .entry

$imageBird          DC8  "example.bmp", 8  ; data
$someConstant       DC32  12456789

; DSxx to declare a variable in RAM, followed by the number of elements
$RamData1           DV32    1 ; one 32-bit integer
$MyArray            DV8    10 ; array of 10 bytes

; label definition
.entry:   ;; comment here should work
    mov      r0, r2  ; copy R2 into R0 (blank space between , and R2)
mov R0,R2  ; copy R2 into R0 (NO blank space between , and R2)

    jump .entry
    halt
)";


TEST_CASE( "Check various indentations and typos" ) {

    std::vector<uint8_t> program;
    Chip32Assembler assembler;
    AssemblyResult result;

    bool parseResult = assembler.Parse(test1);

    REQUIRE( parseResult == true );

    assembler.BuildBinary(program, result);

    result.Print();

    uint16_t progSize = program.size();

    hexdump(program.data(), program.size());


}
