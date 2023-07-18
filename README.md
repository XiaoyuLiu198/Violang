# Violang
Build a simplified language from scratch - C++.

### Example of the language
(var x 15)

(if (x>10) (set x (+ x 10)))

(while (x<100) (set x (+ x 10)))

### Diagram of how scripts are interpreted
<img width="691" alt="Screen Shot 2023-07-18 at 10 33 47 AM" src="https://github.com/XiaoyuLiu198/Violang/assets/65391883/3930a951-55bf-48cf-8d67-78b7989f1d07">


### Parser
Generated LALR(1) parser with the help of syntax_cli generator.

Grammar: 
- E -> A | L
- L -> ( L' )
- L' -> L' E
- L' -> ε
- A -> SYMBOL | NUMBER | STRING

### Compiler
Implemented compiler to improve execution efficiency by interpreting with bytecode instead of AST. The functionality of the compiler includes 
- 1. Allocate objects and variables.
- 2. Generate operator codes as instructions for virtual machine.

### Stack based VM
Built as an interpreter of a special bytecode for execution on the CPU. 
The main data structures for a stack VM are:
- 1. Value stack maintaining calculated result.
- 2. Call stack maintaining subroutine pointers.
- 3. Constant pool.

#### Example: how to implement interpreter of an arithmetic expression
Calculate ```(var x (+ x 10))``` after defining x as 15.

<img width="372" alt="Screen Shot 2023-07-17 at 6 16 47 PM" src="https://github.com/XiaoyuLiu198/Violang/assets/65391883/2f5caf89-f659-49e2-820f-6e7973cd2ebc">


- Step 1: Get variable x from variables with index, push to value stack
- Step 2: push constant 10 to constant pool and to the value stack.
- Step 3: add the top 2 values from value stack and push result back to value stack
- Step 4: Set variable x as result 25

### Mark-sweep Garbage collection

In ```marking``` phase garbage collector starts by assuming that all objects in memory are garbage. It then identifies the root objects, suc h as global variables, local variables on the stack, and any other references that are known to be in use. Starting from the root objects, the garbage collector traverses the object graph, following references and marking all reachable objects as not garbage. 

In ```sweeping``` phase, the garbage collector performs a cleanup operation. The garbage collector iterates over the entire memory heap, examining each object. Any objects that are not marked are freed.

## Continuing...
Supporting of scoped names binding via closure, and supporting of OOP is in-progress ...
