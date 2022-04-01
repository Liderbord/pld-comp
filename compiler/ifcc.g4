grammar ifcc;

axiom: prog;

prog: fn+ EOF;
fn: TYPE VARNAME '(' argsDef? ')' '{' content? '}';
content: (init | affectation | ifElse | whileDo | returnValue) content?;
value: CONST | VARNAME;
returnValue: 'return' expression ';';
init: TYPE declaration; 
declaration: dec (',' dec)* ';' ; 
dec: VARNAME ('=' expression)? ;
affectation: VARNAME '=' expression ';' # affectationExpr;
expression:
	expression MULTDIV expression # expressionMultDiv
	| expression ADDSUB expression # expressionAddSub
	| expression ('&=' | '&&') expression # expressionAnd
	| expression ('|=' | '||') expression # expressionOr
	| expression '^=' expression # expressionXor
	| expression '==' expression # expressionEqual
	| expression '!=' expression # expressionNotEqual
	| expression '>' expression # expressionGreater
	| expression '<' expression # expressionLess
	| expression '>=' expression # expressionGreaterEqual
	| expression '<=' expression # expressionLessEqual
	| '(' expression ')' # expressionPar
	| VARNAME '(' args? ')' # expressionFn
	| value # expressionValue;
ifElse: 'if' '(' expression ')' '{' content? '}' ( 'else' '{' content? '}' )?;
whileDo: 'while' '(' expression ')' '{' content '}';
args: (expression) (',' expression)*;
argsDef: (TYPE VARNAME) (',' TYPE VARNAME)*;

CONST: [0-9]+;
ADDSUB: '+' | '-';
MULTDIV: '*' | '/';
COMMENT: '/*' .*? '*/' -> skip;
ONELINECOMMENT: '//' .*? '\n' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
ARITH: '+' | '-' | '*' | '/' | '%';
REF: '&';
TYPE: 'int' | 'float' | 'double';
VARNAME: [a-zA-Z_]([a-zA-Z0-9_]| '\\u' HEX4 |'\\U' HEX4 HEX4)*;
HEX: [0-9a-fA-F];
HEX4: HEX HEX HEX HEX;