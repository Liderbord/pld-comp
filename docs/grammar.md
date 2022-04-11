# 🔣 Grammar

## Terminals

```python
COMMENT: '/*' .*? '*/' -> skip;
ONELINECOMMENT: '//' .*? '\n' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
CONST: [0-9]+;
MULTDIVMOD: '*' | '/' | '%';
CHAR: '\'' .? '\'';
REF: '&';
TYPE: 'int' | 'char' | 'void';
VARNAME: [a-zA-Z_]([a-zA-Z0-9_])*;
```

- `ONELINECOMMENT` and `COMMENT` are use to ignore comments.
- `WS` is used to ignore whites paces.
- `CONST` is used to match numbers.
- `ADDSUB` (used later), `MULTDIVMOD` are used to math operators. Respecting their priority.
- `CHAR` is used to match characters, only single characters are recognized, not strings, this is intentional as we believed multiple character chars were a confusing (yet valid in C) concept.
- `REF` is used to match references. This was necessary to generate an error when the symbol is used in the name of a variable.
- `TYPE` is used to match types of the variables and functions.
- `VARNAME` is used to match names of the variables and functions.

## Rules

```python
prog: fn+ EOF;
fn: TYPE VARNAME '(' argsDef? ')' '{' content? '}';
content: (init | arrayDec | affectation ';' | ifElse | whileDo | returnValue ';' | fnCall ';') content?;
value: CONST | VARNAME | CHAR | VARNAME '[' expression ']';
returnValue: 'return' expression;
init: TYPE declaration;
declaration: dec (',' dec)* ';' ;
dec: VARNAME ('=' expression)? ;
arrayDec: TYPE VARNAME '['CONST']'  ('=' '{' CONST (',' CONST)* '}' )?  ';' #arrayDeclaration ;
affectation: VARNAME '=' expression # affectationExpr
	| VARNAME '[' expression ']' '=' expression #affectationArray;
expression:
	expression MULTDIVMOD expression # expressionMultDivMod
	| expression ADDSUB=('+' | '-') expression # expressionAddSub
	| expression ('&' | '&&') expression # expressionAnd
	| expression ('|' | '||') expression # expressionOr
	| expression '^' expression # expressionXor
	| expression '==' expression # expressionEqual
	| expression '!=' expression # expressionNotEqual
	| expression '>' expression # expressionGreater
	| expression '<' expression # expressionLess
	| expression '>=' expression # expressionGreaterEqual
	| expression '<=' expression # expressionLessEqual
	| '!' expression # expressionNegation
	| '-' expression # expressionNegative
	| '(' expression ')' # expressionPar
	| fnCall # expressionFn
	| value # expressionValue;
fnCall: VARNAME '(' args? ')';
ifElse: 'if' '(' expression ')' '{' content? '}' ( 'else' '{' content? '}' )?;
whileDo: 'while' '(' expression ')' '{' content '}';
args: (expression) (',' expression)*;
argsDef: (TYPE VARNAME) (',' TYPE VARNAME)*;
```

- The `prog` rule is the most general and where the parsing starts and ends. It is composed of multiple functions and EOF to indicate the end of the file.
- The `fn rule` is the functions definition, where it can three return types (int, char, void), a name, a list of arguments and its content.
- The `content` rule is the content of the function, where it can be a declaration, an affectation, an ifElse, a whileDo or a returnValue.
- The `value` rule is the value of the expression, where it can be a constant (int, char) or a variable. The function called is going to return a Element object, that contains the value in two different ways: as the register index for the variable or as the value itself for the constant. This is done to be able to do optimizations in expressions and to check the type of the values.
- The `returnValue` rule is the return of the function, where it can be a value. It will check that the return value is the same type as the function.
- The `init` rule is the declaration of one or multiple variables of the same type.
- The `declaration` rule is the declaration of one or multiple variables of the same type, used by the init rule.
- The `arrayDec` rule is the declaration of an array of fixed size.
- The `affectation` rule is the affectation of a variable with a new value. It can be an array or a simple variable.
- The `expression` rule computes different types of operations, it can return a value or execute a function call, mathematical operations, comparison (equality and inequality), unary and binary operation, etc.
- The `fnCall` rule is the call of a function, where it can be a function with arguments or a function without arguments.
- The `ifElse` rule is the if statement, where it can be a if statement with an condition (expression) and a content, or a if statement with an condition (expression) and a content and an else statement with a content. Elif was not implemented but it's possible to put an else statement with an if statement (nested).
- The `whileDo` rule is the while statement, where it can be a while statement with an condition (expression) and a content.
- The `args` rule is the list of arguments of a function call, where it can be a list of arguments.
- The `argsDef` rule is the list of arguments of a function definition, where it can be a list of arguments with their types.
