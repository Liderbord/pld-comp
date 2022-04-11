# 🏗️ Developer documentation

## Overview of the project structure

You can find below the structure of the main elements of the project.

```
pld-comp/
├── .antlr/
├── build/
├── generated/
├── compiler/
    ├── ifcc.g4
    ├── CodeGenVisitor.h
    ├── CodeGenVisitor.cpp
    ├── main.cpp
    ├── Makefile
    ├── runmake_fedora.sh
    ├── runmake_mac.sh
    ├── runmake_ubuntu.sh
├── tests/
    ├── testfiles/
    ├── ifcc-test-output/
    ├── ifcc-test.py
    ├── ifcc-wrapper.sh
├─ README.md
```

### The `compiler` section

In this section you will find all the code and functionality for the compiler. As mentioned in the readme, you can run the `runmake` script that corresponds to your OS.

#### ifcc.g4

iffcc.g4: This file stores the grammar of the language that will be identified by antlr4.

[More information about ifcc.g4 and the grammar here.](./grammar.md)

### CodeGenVisitor class

CodeGenVisitor class: This class stores how the grammar is handled how to handle each expression in the grammar

You can find an in depth analysis of each function of the class in [the dedicated CodeGenVisitor documentation](./CodeGenVisitor.md).

### The `tests` section

## Grammar
More information about ifcc.g4 and the grammar [here](./grammar.md).

## Language Features

All language features can be found in the dedicated section for [language features](./language-features.md)

## Compiler Features 
All compiler Features can be found in the dedicated compiler features section