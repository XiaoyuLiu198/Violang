# Violang
Build a simplified language from scratch - C++ and C

## Diagram of how scripts are executed
<img width="1116" alt="Screen Shot 2023-01-16 at 12 18 08 AM" src="https://user-images.githubusercontent.com/65391883/212610919-df4f5e5f-6309-418c-aca0-4b75f0e64cb7.png">

## Stack based VM
Built as an interpreter of a special bytecode for execution on the CPU. 
The main two data structures for a stack VM are the code listing(with the instruction pointer), and the stack data.

#### Example: how to implement interpreter for if-else control flow on virtual machine
Script input is "if(> 5 10) 1 2". In python it can be represented as "1 if 10 >5 else 2"

The key of stack based VM for is-else control flow is that: we jump to corresponding lines when certain condition is satisfied. 

<img width="561" alt="Screen Shot 2023-01-16 at 12 26 36 AM" src="https://user-images.githubusercontent.com/65391883/212612080-cf61f9c3-50e3-472a-85c5-939100cc77e4.png">

## Parser
Tokenizer and parser are essential components of the parsing process. 

```Tokenizer```(lexical analyzer) is responsible for breaking input to individual tokens. The tokenizer implemented used regular expression to identify and classify the tokens to different types such as 'number', 'string', etc. 

```Parser(syntactic analyser)``` takes the stream of tokens produced by the tokenizer and analyzes their structure based on LR parsing logic. Parser would generate an abstract syntax tree (AST) representing the structure of the input.

## Mark-sweep Garbage collection

In ```marking``` phase garbage collector starts by assuming that all objects in memory are garbage. It then identifies the root objects, suc h as global variables, local variables on the stack, and any other references that are known to be in use. Starting from the root objects, the garbage collector traverses the object graph, following references and marking all reachable objects as not garbage. 

In ```sweeping``` phase, the garbage collector performs a cleanup operation. The garbage collector iterates over the entire memory heap, examining each object. Any objects that are not marked are freed.
