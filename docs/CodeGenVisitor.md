# Object Code Gen Visitor

## Helper Functions

### `Element::getValue()`

get the value (register or constant int/char) of an element in assembly code

```
@return string
```

### `Element CodeGenVisitor::getNewTempVariable()`

generate a new temporal variable and return its register index

```
@return Element (variable or const)
```

### `Element CodeGenVisitor::operationExpression(Element leftval, Element rightval, string operation)`


set the assembly code of the selected operation between leftval and rightval

```
@param Element leftval
@param Element rightval
@param string operation
@return Element (variable or const)
```

### `string CodeGenVisitor::getRegister(string type)`

get the respective register (AL, EAX) for the given type (`char` or `int`, defaults to `int`)

```
@param type
@return string
```

### `string CodeGenVisitor::getMove(string type)`

get the respective assembly Instruction (movl, movb) for the given type (`char` or `int`, defaults to `int`)

```
@param type
@return string
```

### `void CodeGenVisitor::setCurrentFunction(string name, string type)`

set the current function name and intialize it's stack

```
@param string name
```

### `string CodeGenVisitor::getCurrentFunctionType()`

get the current function type

```
@return string
```

### `map<string, Function> CodeGenVisitor::getFunctions()`

get all the functions and their stack

```
@return map<string, Function>
```

### `map<string, Variable> CodeGenVisitor::getVars()`

get the stack (vars) from the current function

```
@return map<string, int>
```

### `Variable CodeGenVisitor::getVar(string varname)`

get the register index of a selected variable of the current function

```
@param varname
@return Variable (register index)
```

### `void CodeGenVisitor::setVar(string varname, int index, string type)`

set the register index of a selected variable of the current function

```
@param varname
@param index
@param type
```

### `void CodeGenVisitor::setVarUsed(string varname)`

set that a ver was already used in the current function to avoid warnings

```
@param varname
```

### `bool CodeGenVisitor::isVarDeclarated(string varname)`

check if variable is not in the stack of the current function

```
@param varname
@return bool
```

### `string CodeGenVisitor::getJumpLabel()`

get a new jump label on assembly code, to jump from one part of the code to another

```
@return string
```

## Error function

### `bool CodeGenVisitor::getError()`

get a bool indicating if there's an error in the program

```
@return bool
```

### `void CodeGenVisitor::setError()`

set error to true to indicate there's an error in the program

## Visitor Functions

### `antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)`

get the value (register or constant int/char) of an element in assembly code

```
@return string
```

### `antlrcpp::Any CodeGenVisitor::visitArgsDef(ifccParser::ArgsDefContext *ctx)`

set all the argument declarations of the function and save them in the function's        stack through 
assembly code
```
@param ifccParser::ArgsDefContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitFn(ifccParser::FnContext *ctx)`

set the function's name, stack, arguments and body/content

```
@param ifccParser::FnContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)`

visit all the statements of the function, starting with the first one

```
@param ifccParser::ContentContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx)`

set the return value of the function to the register EAX and set the return value equal to EAX

```
@param ifccParser::ReturnValueContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitInit(ifccParser::InitContext *ctx)`

set the declaration of a variable and save it in the stack (vars) of the current function(currentFunction)

```
@param ifccParser::InitContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx)`

get a vector of pairs (varname, expression/nullptr) for all the declarations

```
@param ifccParser::DeclarationContext ctx
@return vector<pair<string, ifccParser::ExpressionContext *>>
```

### `antlrcpp::Any CodeGenVisitor::visitDec(ifccParser::DecContext *ctx)`

get the pair (varname, expression/nullptr) for the current declaration

```
@param ifccParser::DecContext ctx
@return pair<string, ifccParser::ExpressionContext *>
```

### `antlrcpp::Any CodeGenVisitor::visitAffectationExpr(ifccParser::AffectationExprContext *ctx)`

set a new value to a variable of the current function

```
@param ifccParser::AffectationExprContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx)`

return the register or constant ($) of the selected value it can be a varname, or a number/char respectively

```
@param ifccParser::ValueContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitIfElse(ifccParser::IfElseContext *ctx)`

set a if/else structure using jumps in assembly code

```
@param ifccParser::IfElseContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitWhileDo(ifccParser::WhileDoContext *ctx)`

set a while structure using jumps in assembly code

```
@param ifccParser::IfElseContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionMultDivMod(ifccParser::ExpressionMultDivModContext *ctx)`

set an multiplication or division operation between leftval and rightval according to the symbol of MULTDIV

```
@param ifccParser::ExpressionMultDivContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionAddSub(ifccParser::ExpressionAddSubContext *ctx)`

set an addition or substraction operation between leftval and rightval according to the symbol of ADDSUB

```
@param ifccParser::ExpressionAddSubContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionAnd(ifccParser::ExpressionAndContext *ctx)`

set the operation AND between leftval and rightval the symbol can be && or &=

```
@param ifccParser::ExpressionAndContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionOr(ifccParser::ExpressionOrContext *ctx)`

set the operation OR between leftval and rightval the symbol can be |& or |=

```
@param ifccParser::ExpressionOrContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionXor(ifccParser::ExpressionXorContext *ctx)`

set the operation XOR between leftval and rightval

```
@param ifccParser::ExpressionXorContext ctx
@return Element (variable or const)
```

### `Element CodeGenVisitor::operationCompExpression(Element leftval, Element rightval, string comp)`

set the assembly code of the selected equality or inequality operation between leftval and rightval

```
@param Element leftval
@param Element rightval
@param string comp
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionEqual(ifccParser::ExpressionEqualContext *ctx)`

set the operation equal (==) between leftval and rightval

```
@param ifccParser::ExpressionEqualContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionNotEqual(ifccParser::ExpressionNotEqualContext *ctx)`

set the operation not equal (!=) between leftval and rightval

```
@param ifccParser::ExpressionNotEqualContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionGreater(ifccParser::ExpressionGreaterContext *ctx)`

set the operation greater (>) between leftval and rightval

```
@param ifccParser::ExpressionGreaterContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionLess(ifccParser::ExpressionLessContext *ctx)`

set the operation less (<) between leftval and rightval

```
@param ifccParser::ExpressionLessContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionGreaterEqual(ifccParser::ExpressionGreaterEqualContext *ctx)`

set the operation greater or equal (>=) between leftval and rightval

```
@param ifccParser::ExpressionGreaterEqualContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionLessEqual(ifccParser::ExpressionLessEqualContext *ctx)`

set the operation less or equal (<=) between leftval and rightval

```
@param ifccParser::ExpressionLessEqualContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitArgs(ifccParser::ArgsContext *ctx)`

visit all the arguments of the function (expressions) and save them in the arguments registers

```
@param ifccParser::ArgsContext ctx
@return antlrcpp::Any
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionFn(ifccParser::ExpressionFnContext *ctx)`

call a function, and return the register index of the call's result as a temporal variable

```
@param ifccParser::ExpressionFnContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitFnCall(ifccParser::FnCallContext *ctx)`

set a function call with its arguments

```
@param ifccParser::FnCallContext ctx
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionPar(ifccParser::ExpressionParContext *ctx)`

visit a value (var/const) inside parenthesis and return the register index

```
@param ifccParser::ExpressionValueContext ctx
@return Element (variable or const)
```

### `antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx)`

visit a value (var/const) and return it

```
@param ifccParser::ExpressionValueContext ctx
@return Element (variable or const)
```