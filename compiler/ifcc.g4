grammar ifcc;

axiom: prog;

prog:
	TYPE 'main' '(' ')' '{' content RETURN value ';' '}'
	| TYPE 'main' '(' ')' '{' RETURN value ';' '}';
value: CONST | VARNAME;
content: init | init content;
init: TYPE VARNAME '=' expression ';';
expression:
	expression '*' expression # expressionMult
	| expression '+' expression # expressionAdd
	| value # expressionValue;

RETURN: 'return';
CONST: [0-9]+;
COMMENT: '/*' .*? '*/' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
ARITH: '+' | '-' | '*' | '/' | '%';
TYPE: 'int' | 'float' | 'double';
VARNAME: [a-zA-Z_][a-zA-Z0-9_]*; 