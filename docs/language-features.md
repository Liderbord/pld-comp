# Language features

Language features are teh features of the C programming language that are implemented in our compiler. Some features are limited however, and not fully implemented so please read in detail to better understand what you can and what you cannot do.

## Declaration and affectation
- Declaration: `int foo;`
- Multiple declaration: `int foo, bar;`
- Declaration and assignment: `int foo = 3;`
- Multiple declaration and assignment: `int foo = 3, bar = 42;`
- Assignment / affectation: `foo = 2`, `foo = otherVar` 

All of these features are supported with the `char` type as well, more about that in the [char section](#characters).

- If a variable is declared twice, it will throw an error
- If a variable is declared but never used a warning will be given as a comment in the assembly code
- If a value is assigned to an undeclared variable, the compiler will throw an error.
- Invalid types also throw an error.

For more declaration support check out the [`var_decl` test suite](/tests/testfiles/var_decl/).

## Operations

We support a wide variety of operations:
- Arithmetic operations `+`, `-`, `*`, `%` and `/` 
- Boolean operators: `==`, `!=`, >, `<`, `>=`, `<=`, `||` and `&&`
- Brackets `(` and `)` are used to enforce priorities
- It is possible to add chars and ints

Operations of all kind are supported, if you would like to know their priorities and the supported operations please refer to the [grammar documentation](/docs/grammar.md).

For more declaration support check out the [`operations` test suite](/tests/testfiles/operations/).

## While loops

## If and else
- If statements work
- Nested `if` statement work
- `else` statements work
- It is possible to put any expression in the if statement, even a function

### Notes
- As per the grammar, curly brackets **must** be used with `if` and `else` statements.
- `else if`statements are now not supportated because we enforce the curly brackets.

If you desire to test the if/else functionalities check out the [`if` test suite](/tests/testfiles/if/)

## While 

While loops are supported but there are some limitations:
- You can have a maximum of 6 inputs.
- There are some issues with expressions that are false
- Returning within the loop doesn't work

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
- If an int is assigned to a char it will be casted as a char
- Same goes for char to int

#### Notes

- We discovered an issue in which characters that contain more than once symbol are considered as valid. As per our grammar and code visitor, they are not considered as valid : `char a = 'ab';` is recognized as valid C code by gcc, but not our compiler.

## Arrays

- Declaration : `int tab[2];`
- Declaration & Init : `int tab[2] = {1, 2}`
- Affectation, either for a constant or a variable : `int a = tab[1]; or int a = tab[i];`
- Return : `return tab[1];`
- While valid code works fine, invalid indexing of arrays had some unpredictable behaviours, which we tohught was fine as it should be avoided by developers.

If you desire to test the char functionalities check out the [`char` test suite](/tests/testfiles/char/)

## Functions

In order to make our compiler stand out, we decided to implement functions, they do have some limitations however:
- So far we are unsure about returning non int types
- There can be up to 6 inputs, not more
- Recursion doesn't work

Other features such as function parameters (using supported types), and local variables should work fine. Calling other functions within a function should also work.

## putchar

The putchar feature is implemented but it was not tested extensively, so there might be some unknown bugs.