# ILOC Compiler

## Problem Statement
ILOC is a simple assembly language which uses a series of instructions to read, write, and perform arithmetic operations on registers. For example, the following shows ILOC instructions used to add three constant numbers together.

```
loadI 5 => r0
loadI 7 => r1
loadI 9 => r2
add r0, r1 => r3
add r3, r2 => r4
```

When this code is run on a computer with four registers, it will compute correctly, storing the value of the operation in r4. However, a problem arises when this code is executed on a machine with three registers. Instruction five will attempt to store the result of the addition between r3 and r2 into r4, which does not exist. To circumvent this problem, the ILOC code can be changed to save the value of a register in memory, freeing it for use.

```
loadI 5 => r2
loadI 7 => r1
loadI 32768 => r0
store r1 => r0
loadI 9 => r1
loadI 32772 => r0
store r1 => r0
loadI 32768 => r0
load r0 => r1
add r2, r1 => r1
loadI 32772 => r0
load r0 => r2
add r1, r2 => r2
```

The block above reserves r0 to move code in and out of memory, saving the values of the registers to memory until they are next used in the sequence of instructions. This process is called “spilling” a register. The goal of this project is to automate this task such that any input of ILOC instructions can be transformed so that the initial virtual registers are assigned real physical registers so that the output can execute on a system with k physical registers.

## Methods
The overall implementation of the register allocator closely follows the naive algorithm outlined by Keith D. Cooper and Linda Torczon in Engineering a Compiler.

Before executing this algorithm on the instructions, the next use for each operand is computed in linear time by referencing lookup tables. The algorithm begins execution at the last instruction, allocating each operand’s virtual register to a physical one, creating instructions to spill an existing register into memory if needed. After allocation, if the virtual register is not used again, it frees its physical register for further use.

When spilling a register, the implementation selects the virtual register with the furthest next use from the current instruction in an attempt to reduce the number of spills that occur.

Multiple key differences in implementation were made to this initial algorithm. First, the implementation of operands tracks its source, virtual, and physical registers together such that the allocated instruction does not need to be re-computed. Second, Instead of modifying the source block to insert spill instructions, the implementation appends instructions to a new block instead. This increases compiler memory usage, but decreases the difficulty of indexing instructions. Finally, multiple register classes are not supported.

Because the implementation stores multiple objects tracking allocation state during execution, it never has to look back to compute current needs, making the time complexity linear to the number of instructions in the input block, or O(n).

## References
Cooper, Keith D., and Linda Torczon. “Register Allocation.” Engineering a Compiler, 2nd ed., vol. 1, Morgan Kaufmann, San Francisco, CA, 2011, pp. 679–723.

Testing utilities provided by Seth Fogarty of Trinity University.
