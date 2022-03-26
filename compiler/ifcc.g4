grammar ifcc;

axiom: prog;

prog: fn+;
fn: TYPE VARNAME '(' argsDef? ')' '{' content '}';
content: (init | ifElse | whileDo | returnValue) content?;
value: CONST | VARNAME;
returnValue: 'return' value ';';
init: TYPE VARNAME '=' expression ';';
expression:
	expression '*' expression # expressionMult
	| expression '/' expression # expressionDiv
	| expression '+' expression # expressionAdd
	| expression '-' expression # expressionSub
	| expression '&=' expression # expressionAnd
	| expression '|=' expression # expressionOr
	| expression '^=' expression # expressionXor
	| expression '==' expression # expressionEqual
	| expression '!=' expression # expressionNotEqual
	| expression '>' expression # expressionGreater
	| expression '<' expression # expressionLess
	| VARNAME '(' args? ')' # expressionFn
	| value # expressionValue;
ifElse: 'if' '(' expression ')' '{' content '}' ( 'else' '{' content '}' )?;
whileDo: 'while' '(' expression ')' '{' content '}';
args: (expression) (',' expression)*;
argsDef: (TYPE VARNAME) (',' TYPE VARNAME)*;

CONST: [0-9]+;
COMMENT: '/*' .*? '*/' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
ARITH: '+' | '-' | '*' | '/' | '%';
TYPE: 'int' | 'float' | 'double';
VARNAME: [a-zA-Z_][a-zA-Z0-9_]*; 