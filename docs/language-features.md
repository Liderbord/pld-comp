# 💬 Language features

## Basic features

Declaration and affectation work, all the lines below are valid syntax.
```c
    char a;
    a = 0;
    char b = 1;
    char c = 2, d = 3;
```
- If a variable is declared twice, it will throw an error
- If a variable is declared but never used a warning will be given as a comment in the assembly code
- If a value is assigned to an undeclared variable, the compiler will throw an error.
- Invalid types also throw an error.

For more declaration support check out the [`var_decl` test suite](/tests/testfiles/var_decl/).

## Operations
Operations of all kind are supported, if you would like to know their priorities and the supported operations please refer to the [grammar documentation](/docs/grammar.md).

For more declaration support check out the [`operations` test suite](/tests/testfiles/operations/).

## If and else
- If statements work
- Nested `if` statement work
- `else` statements work

### Notes
- As per the grammar, curly brackets **must** be used with `if` and `else` statements.
- `else if`statements are now not supportated because we enforce the curly brackets.

If you desire to test the if/else functionalities check out the [`if` test suite](/tests/testfiles/if/)

## While 

While loops are supported but there are some limitations:
- You can have a maximum of 6 inputs.
## Characters

- `char` type is supported
- chars variable are stored with one byte, so please note that the max char valuess are `255`, then it will look back to `0`
- It is possible to assign and do operations with char values:

```c
    char c = '0';
    int a = 0;
    char b = a + c;
    return b;
```

If you desire to test the char functionalities check out the [`char` test suite](/tests/testfiles/char/)

## Functions

