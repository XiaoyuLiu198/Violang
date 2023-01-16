# Violang
Build a simplified language from scratch - C++ and C

## Diagram of how scripts are executed
<img width="1116" alt="Screen Shot 2023-01-16 at 12 18 08 AM" src="https://user-images.githubusercontent.com/65391883/212610919-df4f5e5f-6309-418c-aca0-4b75f0e64cb7.png">

## Stack based VM
Built as an interpreter of a special bytecode for execution on the CPU. 
The main two data structures for a stack VM are the code listing, with the instruction pointer, and the stack data, which is accessed only via the stack pointer. 
### Example: how to implement interpreter for if-else control flow on virtual machine
Script input is "if(> 5 10) 1 2". In python it can be represented as "1 if 10 >5 else 2"

The key of stack based VM for is-else control flow is that: we jump to corresponding lines when certain condition is satisfied. 
<img width="561" alt="Screen Shot 2023-01-16 at 12 26 36 AM" src="https://user-images.githubusercontent.com/65391883/212612080-cf61f9c3-50e3-472a-85c5-939100cc77e4.png">
