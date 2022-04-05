grammar ifcc;

axiom: prog;

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
	| fnCall # expressionFn
	| value # expressionValue;
fnCall: VARNAME '(' args? ')';
ifElse: 'if' '(' expression ')' '{' content? '}' ( 'else' '{' content? '}' )?;
whileDo: 'while' '(' expression ')' '{' content '}';
args: (expression) (',' expression)*;
argsDef: (TYPE VARNAME) (',' TYPE VARNAME)*;

COMMENT: '/*' .*? '*/' -> skip;
ONELINECOMMENT: '//' .*? '\n' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
CONST: [0-9]+;
ADDSUB: '+' | '-';
MULTDIVMOD: '*' | '/' | '%';
CHAR: '\'' .? '\'';
REF: '&';
TYPE: 'int' | 'char' | 'float' | 'double' | 'void';
VARNAME: [a-zA-Z_]([a-zA-Z0-9_]| '\\u' HEX4 |'\\U' HEX4 HEX4)*;
HEX: [0-9a-fA-F];
HEX4: HEX HEX HEX HEX;
