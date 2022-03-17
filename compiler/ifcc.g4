grammar ifcc;

axiom: prog;

prog:
	TYPE 'main' '(' ')' '{' content RETURN value ';' '}'
	| TYPE 'main' '(' ')' '{' RETURN value ';' '}';
value: CONST | VARNAME;
content: declaration*;
init: TYPE VARNAME '=' expression ';';

declaration: TYPE VARNAME (',' VARNAME)* ';' ;

expression:
	expression '*' expression # expressionMult
	| expression '+' expression # expressionAdd
	| '(' expression ')' # expressionPar
	| value # expressionValue;

	 

RETURN: 'return';
CONST: [0-9]+;
COMMENT: '/*' .*? '*/' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
ARITH: '+' | '-' | '*' | '/' | '%';
TYPE: 'int' | 'float' | 'double';
VARNAME: [a-zA-Z_][a-zA-Z0-9_]*; 