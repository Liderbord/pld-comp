grammar ifcc;

axiom: prog;

prog:
	TYPE 'main' '(' ')' '{' content RETURN value ';' '}'
	| TYPE 'main' '(' ')' '{' RETURN value ';' '}';
value: CONST | VARNAME;

content: init*;

init: TYPE declaration; 
declaration: dec (',' dec)* ';' ; 
dec: VARNAME ('=' expression)? ;

affectation: VARNAME '=' VARNAME # affectationVar
	| VARNAME '=' CONST # affectationConst
	| VARNAME '=' expression # affectationExpr;

expression:
	expression '*' expression # expressionMult
	| expression '/' expression # expressionDiv
	| expression '+' expression # expressionAdd
	| '(' expression ')' # expressionPar
	| expression '-' expression # expressionSub
	| expression '&=' expression # expressionAnd
	| expression '|=' expression # expressionOr
	| expression '^=' expression # expressionXor
	| expression '==' expression # expressionEqual
	| expression '!=' expression # expressionNotEqual
	| expression '>' expression # expressionGreater
	| expression '<' expression # expressionLess
	| value # expressionValue;

	 

RETURN: 'return';
CONST: [0-9]+;
COMMENT: '/*' .*? '*/' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
ARITH: '+' | '-' | '*' | '/' | '%';
TYPE: 'int' | 'float' | 'double';
VARNAME: [a-zA-Z_][a-zA-Z0-9_]*; 