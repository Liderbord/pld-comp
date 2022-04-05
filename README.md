# 💻 PLD Compilateur

C Compiler written using ANTLR.

# 👩‍💻 Installation

Not that this compiler only works on Linux and Mac, it will not run on windows (but it will work on WSL), learn more about wsl [here](https://docs.microsoft.com/fr-fr/windows/wsl/install).

Installation instructions can be found in the [Installation documentation](/docs/Installation.md).

# 🔨 Usage

Once you have installed and compiled the project (compilation instructions are included in the [installation documentation](/docs/Installation.md)), you can start to compile c code:

```bash
cd compiler
./ifcc my_c_program.c
```

This will print out a source file in the console, if you desire to store the output, you can always run:

```bash
./ifcc my_c_program.c >> my_source_code.s
```

Now if you want to actually execute the code you must use an assembler. Note that only x86 assembly is supported as of now.

```bash
as my_source_code.s
```

That's it! If your code isn't too complex, it should compile just fine 😀