on peut pas faire de else if car les accolades sont forcées

if else marche apr conre

# Language features

## If and else
- If statements work
- Nested `if` statement work
- `else` statements work

### Notes
- As per the grammar, curly brackets **must** be used with `if` and `else` statements.
- `else if`statements are now not supportated because we enforce the curly brackets.

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

- 