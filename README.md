# 💻 PLD Compilateur / Compiler project

C Compiler written using ANTLR.

## 👩‍💻 Installation

Not that this compiler only works on Linux and Mac, it will not run on windows (but it will work on WSL), learn more about wsl [here](https://docs.microsoft.com/fr-fr/windows/wsl/install).

Installation instructions can be found in the** [Installation documentation](/docs/Installation.md)**.

## 🔨 Usage

Once you have installed and compiled the project (compilation instructions are included in the **[installation documentation](/docs/Installation.md))**, you can start to compile c code:

```bash
cd compiler
./ifcc my_c_program.c
```

This will print out a source file in the console, if you desire totore the output, you can always run:

```bash
./ifcc my_c_program.c >> my_source_code.s
```

Now if you want to actually execute the code you must use an assembler. Note that only x86 assembly is supported as of now.

```bash
as my_source_code.s
```

That's it! If your code isn't too complex, it should compile just fine 😀

## 🧪 Tests

In order to run tests you need python 3. 

To run tests, navigate to the `tests` folder
```bash
cd tests
```

### Running all the tests

Run all tests by running the following command

```bash
./ifcc-test.py . 
```

### Multithreading

Since there are a lot of tests and waiting isn't very cool we there is a multithreading flag which will run the tests in parrallel. Note that the tests make take a bit longer to get started (but they will over run a lot faster), and that it might not work on every machine. 

To enable multithreading use the `-m` flag

```bash
./ifcc-test.py . -m
```

### Running specific tests

If you want to run a specific set of tests you can modify the `.` for the relative path of the file/ folder you want to run. Here is an example for running the tests in the `core` folder only:

```bash
./ifcc-test.py ./testfiles/core 
```

## 🏗️ Developer documentation

Want to contribute to the project? Checkout our **[developer documentation](/docs/dev-docs.md)** to make sure you understand how it works.
