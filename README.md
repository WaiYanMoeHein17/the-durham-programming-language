# Durham Language - Web Compiler

A web-based interpreter for the Durham Programming Language, allowing you to run Durham code directly in your browser!
This repository also includes VSCode language support for syntax highlighting and snippets and the compiler source code.

## Features

**Turing Complete** - Language is turing complete, so you can build anything although maybe with a bit of difficulty. 
**College Reference** - Numbers have been replaced by the colleges of Durham University.   
**VSCode Language Support** - Syntax highlighting and snippets for VSCode that you can download from this repository .vsix file

## Where 

Go to [Durham Language Web Compiler](https://durham-lang.github.io/web-compiler/) 

## Example Programs

### Hello World
```durham
tlc begin "Hello, Durham University!" end.
```

### Variables
```durham
number x is castle.
tlc begin x end.
```

### String Concatenation
```durham
text first is begin "Hello" end.
text second is begin " World" end.
text message is first durham second.
tlc begin message end.
```

### If-Else
```durham
number x is castle.
if begin x lesser grey end front
    tlc begin "x is less than 10" end.
back else front
    tlc begin "x is greater than or equal to 10" end.
back
```

### For Loop
```durham
for begin i is butler. i lesser snow. i is i durham chads end front
    tlc begin i end.
back
```

## College Number System

| College | Value | College | Value |
|---------|-------|---------|-------|
| butler | 0 | snow | 9 |
| chads | 1 | grey | 10 |
| marys | 2 | stephenson | 11 |
| collingwood | 3 | hatfield | 12 |
| johns | 4 | hildbede | 13 |
| castle | 5 | south | 14 |
| cuths | 6 | vanmildert | 15 |
| trevs | 7 | ustinov | 16 |
| aidans | 8 | | |

**Multi-digit numbers:** Use commas to concatenate digits  
Example: `marys,marys` = 22

## Language Documentation

### Operators
- **Addition:** `durham` (e.g., `x durham y`)
- **Subtraction:** `newcastle` (e.g., `x newcastle y`)
- **Multiplication:** `york` (e.g., `x york y`)
- **Division:** `edinburgh` (e.g., `x edinburgh y`)

### Comparisons
- **Less than:** `lesser` (e.g., `x lesser y`)
- **Greater than:** `greater` (e.g., `x greater y`)
- **Equals:** `equals` (e.g., `x equals y`)

### Control Flow
- **If statement:** `if begin condition end front ... back`
- **Else:** `back else front ... back`
- **For loop:** `for begin init. condition. increment end front ... back`

### Functions
```durham
function name begin params end front
    mcs begin return_value end.
back
```
