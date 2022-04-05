#include "CodeGenVisitor.h"
#include <utility>
#include <vector>
using namespace std;

// general assembly code shortcuts
static const string START_MAC = ".globl	_main\n_main:\n";
static const string START_OTHERS = ".globl	main\nmain:\n";
static const string STACK = "\tendbr64\n \tpushq\t%rbp  # save %rbp on the stack\n\tmovq\t%rsp, %rbp # define %rbp for the current function";
static const string END = "\t# epilogue\n\tpopq\t %rbp  # restore %rbp from the stack\n\tret  # return to the caller (here the shell)\n";
static const string MOVL = "\tmovl ";

// registers (assembly)
static const string AL = "%al";
static const string EAX = "%eax";
static const string ECX = "%ecx";
static const string EDX = "%edx";
static const string RBP = "(%rbp)";
static const string ARG_REGS[6] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"}; // standard argument registers

/**
 * @brief get the value (register or constant int/char) of an element in assembly code
 *
 * @return string
 */

string Element::getValue()
{
	return this->var ? "-" + to_string(this->value) + RBP : "$" + to_string(this->value);
}

// implementation of the << operator, to allow printing of register or const value of
// the Element struct without having to call getValue implicitly.
ostream &
operator<<(ostream &os, Element &element)
{
	return os << element.getValue();
};

/**
 * @brief visit all the functions declarated, starting with the first one
 *
 * @param ifccParser::ProgContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
	//cout << "\t visit Prog" << endl;
	maxOffset = 0;
	visitChildren(ctx);
	return 0;
}

/************************* FUNCTION DECLARATION ****************************/

/**
 * @brief set all the argument declarations of the function and
 * 				save them in the function's stack through assembly code
 *
 * @param ifccParser::ArgsDefContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitArgsDef(ifccParser::ArgsDefContext *ctx)
{
	int counter = 0;
	for (auto varnameContext : ctx->VARNAME())
	{
		string varname = varnameContext->getText();
		string type = ctx->TYPE(counter)->getText();
		//int index = (this->getVars().size() + 1) * 8;
		int index = maxOffset + 8;
		maxOffset = index;
		// if the name of the argument is not repeated, save it in the map
		if (!this->isVarDeclarated(varname))
		{
			this->setVar(varname, index, type);
			// if there's less than 7 arguments, save the variable in the standard registers
			if (counter < 6)
			{
				cout << MOVL << ARG_REGS[counter] << ", -" << to_string(index) << RBP << endl;
			}
		}
		// if the arguments is repeated, set an error
		else
		{
			cout << "# ERROR: same argument " << varname << " declared multiple times" << endl;
			this->setError();
		}
		counter++;
	}
	return 0;
}

/**
 * @brief set the function's name, stack, arguments and body/content
 *
 * @param ifccParser::FnContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitFn(ifccParser::FnContext *ctx)
{
	//cout << "\t#visit FN" << endl;
	string head;
	string fnName = ctx->VARNAME()->getText();
	string fnType = ctx->TYPE()->getText();
	//cout << "fnName" << fnName << endl;
	this->setCurrentFunction(fnName, fnType);
	//cout << " after set current function " << endl;
// if the machine is from apple, use an _ before the name of the function
#ifdef __APPLE__
	head = ".globl	_" + fnName + "\n_" + fnName + ":\n";
#else
	head = ".globl	" + fnName + "\n" + fnName + ":\n";
#endif
	cout << head << STACK << endl;

	// if there's more than one function and the current function is not the first one
	// move the RSP by 16 bytes to make space (standard)
	if (this->functions.size() > 1)
	{
		cout << "\tsubq	$16, %rsp" << endl;
	}

	// visit the arguments of the function, if there are any
	ifccParser::ArgsDefContext *argsDefContext = ctx->argsDef();
	if (argsDefContext)
	{
		cout << "\t# args" << endl;
		visit(argsDefContext);
	}

	// visit the body of the function, if there is one
	ifccParser::ContentContext *contentContext = ctx->content();
	if (contentContext)
	{
		cout << "\t# content" << endl;
		visit(contentContext);
		// if no body, return xorl %eax (good practice from gcc)
	}
	else
	{
		cout << "\txorl	%eax, %eax" << endl;
	}

	// if we moved the RSP at the beggining of the function, move it back
	if (this->functions.size() > 1)
	{
		cout << "\taddq	$16, %rsp" << endl;
	}

	cout << END << endl;
	return 0;
}

/**
 * @brief visit all the statements of the function, starting with the first one
 *
 * @param ifccParser::ContentContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)
{
	ifccParser::ReturnValueContext *returnValueContext = ctx->returnValue();
	if (returnValueContext)
	{
		visit(returnValueContext);
		return 1;
	}
	else
	{
		if (ctx->content())
		{
			return visitChildren(ctx);
		}
		else
		{
			visitChildren(ctx);
			return 0;
		}
	}
}

/**************************** FUNCTION STATEMENTS ****************************/

/**
 * @brief set the return value of the function to the register EAX
 * 				and set the return value equal to EAX
 *
 *
 * @param ifccParser::ReturnValueContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx)
{
	// get the value (var/const) from expression
	Element element = visit(ctx->expression());
	// if the element has the same type as the function's return type, return it
	if (element.type == this->getCurrentFunctionType() ||
			(element.type == "int" && this->getCurrentFunctionType() == "char") ||
			(element.type == "char" && this->getCurrentFunctionType() == "int"))
	{
		// save the value in the register EAX
		// set EAX as the return value
		cout << "\t# return " << element << endl;
		cout << "\t"
				 << "movl " << element << ", %eax" << endl;
	}
	// if the element is not the same type as the function's return type, set an error
	else
	{
		cout << "# ERROR: return type mismatch" << endl;
		this->setError();
	}
	return 0;
}

/**
 * @brief set the declaration of a variable and save it in the stack (vars)
 * 				of the current function (currentFunction)
 *
 * @param ifccParser::InitContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitInit(ifccParser::InitContext *ctx)
{
	cout << "\t#visit init" << endl;
	string type = ctx->TYPE()->getText();
	// get all the declaration in the line
	vector<pair<string, ifccParser::ExpressionContext *>> vectorVars = visit(ctx->declaration());
	// set and save each declaration
	for (auto pair : vectorVars)
	{
		string varname = pair.first;
		//int index = (this->getVars().size() + 1) * 8;
		int index = maxOffset + 8;
		maxOffset = index;
		// if varname doesn't exists in the stack (vars), save it
		if (!this->isVarDeclarated(varname))
		{
			// save the variable in the stack of the current function
			this->setVar(varname, index, type);
			// if there's an expression (initial value), visit the expression and set it to the variable
			// if there's no expression, set the variable to the default value
			string value = pair.second ? visit(pair.second).as<Element>().getValue() : "$0";
			cout << "\t# declare " << type << " and assign " << value << endl;
			cout << "\t" + getMove(type) + " " + value + ", " << getRegister(type) << endl;
			cout << "\t" + getMove(type) + " " + getRegister(type) + ", -" + to_string(index) + "(%rbp)" << endl;
		}
		else
		{
			cout << "# ERROR: variable " << varname << " already declared" << endl;
			this->setError();
		}
	}
	return 0;
}

/**
 * @brief get a vector of pairs (varname, expression/nullptr) for all the declarations
 *
 * @param ifccParser::DeclarationContext ctx
 * @return vector<pair<string, ifccParser::ExpressionContext *>>
 */

antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx)
{
	vector<pair<string, ifccParser::ExpressionContext *>> pairsVector;
	// visit all the declarations
	for (auto decContext : ctx->dec())
	{
		// get the pair of the declaration
		pair<string, ifccParser::ExpressionContext *> pair = visit(decContext);
		// push into the vector
		pairsVector.push_back(pair);
	}
	return pairsVector;
}

/**
 * @brief get the pair (varname, expression/nullptr) for the current declaration
 *
 * @param ifccParser::DecContext ctx
 * @return pair<string, ifccParser::ExpressionContext *>
 */

antlrcpp::Any CodeGenVisitor::visitDec(ifccParser::DecContext *ctx)
{
	// cout << "\t# visit Dec" << endl;
	// declarate the variable with no register
	string varname = ctx->VARNAME()->getText();
	pair<string, ifccParser::ExpressionContext *> pair;
	pair.first = varname;
	pair.second = ctx->expression();

	return pair;
}

/**
 * @brief set a new value to a variable of the current function
 *
 * @param ifccParser::AffectationExprContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitAffectationExpr(ifccParser::AffectationExprContext *ctx)
{
	// get the variable name
	string varname = ctx->VARNAME()->getText();
	// if the variable is already declarated, assign the new value
	if (this->isVarDeclarated(varname))
	{
		// get the variable/const by using the expression visitor
		Element element = visit(ctx->expression());
		// update the usages of the variable, to cancel the 'var no used' warning
		this->setVarUsed(varname);
		// set the direct assignment
		Variable var = this->getVar(varname);
		cout << "\t# assigning " << element << " to " << varname << endl;
		cout << "\t" + this->getMove(var.type) << " " << element << ", " << this->getRegister(var.type) << endl;
		cout << "\t" + this->getMove(var.type) << " " << this->getRegister(var.type) << ", -" << to_string(var.index) << RBP << endl;
	}
	else
	{
		cout << "# ERROR: variable " << varname << " not declared" << endl;
		this->setError();
	}
	return 0;
}



antlrcpp::Any CodeGenVisitor::visitArrayDeclaration(ifccParser::ArrayDeclarationContext *ctx)
{
	
	// we assume that type is INT for now
	string type = ctx->TYPE()->getText();
	// getting the name of the array, CONST is a non-terminal symbol, so no visit
	string tabName = ctx->VARNAME()->getText();
	// getting its length, which is in CONST()[0]
	int length = stoi(ctx->CONST(0)->getText()); 
	//cout << "length" << length << endl;
	// getting the number of declared values
	int nbrValues = ctx->CONST().size() - 1;

	// array of values
	//vector<int> ArrayVal;
	// check that lengths are coherent
	if (length >= nbrValues)
	{
		
		// pushing tabName in vars, it points the last case of the stack so far (=first elt of the array)
		int index = maxOffset + length * 8;
		maxOffset = index;
		//this->vars[tabName] = index;
		this->setVar(tabName, index, type);
		// pusing tabName in the tab of arrays
		tabOfArrays.push_back(tabName);

		for (int i=1; i<=nbrValues; i++){
			//arrayVal.push_back(atoi(ctx->CONST(i)->getText()));
			string value = ctx->CONST(i)->getText();
			// moving the values to the stack
			cout << "\t# Moving $ " << value << " to its location in the stack" << endl;
			cout << "\tmovl $" + value + ", -" + to_string( index - (i-1)*8 ) + "(%rbp)" << endl;
		}
	}
	else 
	{
		error = true;
	}
	return 0;
}


antlrcpp::Any CodeGenVisitor::visitAffectationArray(ifccParser::AffectationArrayContext *ctx)
{
	//cout << "array affect" << endl;
	// getting the Array's variable name
	string tabName = ctx->VARNAME()->getText();
	// getting the variable/const by using the Value visitor
	//string value = visit(ctx->value()).as<string>();
	//value = value*8;
	//cout << "value = " << value << endl;
	Element value = visit(ctx->expression(0));
	Element expr = visit(ctx->expression(1));

	//cout << "expr = " << expr << endl;
	// create temp var
	Element temp = getNewTempVariable();

	//check if the table has already been declared
	
	if ( find(tabOfArrays.begin(), tabOfArrays.end(), tabName) != tabOfArrays.end() )
	{
		//cout << "1" << endl;
		// get the destination index of the array
		Variable var = this->getVar(tabName);
		string index = to_string(var.index);
		//cout << "index " << index << endl;
		// TODO : Check if value > size of array, if its the case -> then its an error
		//mult
		cout << "\t# affect expression to lvalue (case of array)" << endl;
		cout << "\tmovq $8 , %rax" << endl;
		cout << "\tmovl " << value << " ,%ebx" << endl;
		cout << "\timulq %rbx, %rax"<< endl;
		cout << "\taddq $-" + index + ", %rax" << endl;
		cout << "\tmovq %rax, " << temp << endl;
		cout << "\tmovq %rbp, %rax" << endl;
		cout << "\taddq " << temp << " , %rax" << endl;
		cout << "\tmovq %rax, " << temp << endl;
		cout << "\tmovq " << temp << " , %rax" << endl;
		cout << "\tmovq " << expr << ", %r10" << endl;
		cout << "\tmovq %r10, (%rax)" << endl;
		
	}
	else
	{
   		error = true;
	}
	
	


	return 0;

}


/**
 * @brief return the register or constant ($) of the selected value
 *				it can be a varname, or a number/char respectively
 *
 * @param ifccParser::ValueContext ctx
 * @return antlrcpp::Any
 */





antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx)
{
	string returnval;
	antlr4::tree::TerminalNode *varnameNode = ctx->VARNAME();
	ifccParser::ExpressionContext *expressionContext = ctx->expression();
	antlr4::tree::TerminalNode *charNode = ctx->CHAR();
	antlr4::tree::TerminalNode *constNode = ctx->CONST();
	// if the value is a varname, get the register index from
	// the stack of the current function
	// and return it in the assembly format
	if (varnameNode)
	{
		string varname = varnameNode->getText();
		Element temp = getNewTempVariable();
		// if variable is declared, return it
		if (this->isVarDeclarated(varname))
		{
			if (expressionContext){
				Element index = Element(this->getVar(varname).index, "int", true);
				Element eight = Element(8, "int", false);
				Element exprString= visit(expressionContext);
				cout << "\t# access the case and return its value" << endl;
				Element mult = this->operationExpression(exprString, eight, "imull");
				//Element add = this->operationExpression(index, mult, "add ");
				//cout << "\tmovq %rbp, %rax" << endl;
				cout << "\taddq " << mult << ", %rbp" << endl;
				cout << "\tmovl -" << index.value << "(%rbp), %eax" << endl;
				cout << "\tsubq " << mult << ", %rbp" << endl;
				cout << "\tmovl %eax, " << temp << endl;
				
				return temp;

			} else {
				return Element(this->getVar(varname).index, this->getVar(varname).type, true);
			}
			
		}
		// if variable is not declared, throw an error
		else
		{
			cout << "# ERROR: variable " << varname << " not declared" << endl;
			this->setError();
			return 1;
		}
		// if the value is a char, convert the ascii value
		// and return it in the assembly format
	}
	else if (charNode)
	{
		string character = charNode->getText();
		return Element(int(character[1]), "char", false);
	}
	// if the value is a number, convert the number as string
	// and return it in the assembly format
	string text = constNode->getText();
	int number = stoi(text);
	return Element(number, "int", false);
}

/**
 * @brief set a if/else structure using jumps in assembly code
 *
 * @param ifccParser::IfElseContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitIfElse(ifccParser::IfElseContext *ctx)
{
	// get the condition of the if
	Element expval = visit(ctx->expression());
	// get the pointer to the possible else content
	ifccParser::ContentContext *elseContentContext = ctx->content(1);
	// get a new jump label to finish the if
	string jumpEndIf = this->getJumpLabel();
	// set the condition in assembly code
	cout << MOVL << expval << ", " << EAX << endl;
	cout << "\tcmpl $0, " << EAX << endl; // we're comparing the result with 0
	// if there's a else's content
	if (elseContentContext)
	{
		// get a new jump label to omit the if content and pass directly to else
		string jumpElse = this->getJumpLabel();
		// set the jump condition in assembly code (if condition is 0, jump to else)
		cout << "\tje " << jumpElse << endl;
		// set the if's content
		ifccParser::ContentContext *contentContext = ctx->content(0);
		if (contentContext)
		{
			visit(contentContext);
		}
		// set the jump to the end of the if
		cout << "\tjmp " << jumpEndIf << endl;
		// set the jump label of the else
		cout << jumpElse << ":" << endl;
		// set the else's content
		visit(elseContentContext);
		// set the jump to the end of the if
		cout << "\tjmp " << jumpEndIf << endl;
	}
	// if there isn't else's content
	else
	{
		// set the jump condition in assembly code (if condition is 0, jump to the end)
		cout << "\tje " << jumpEndIf << endl;
		// set the if content in assembly code
		ifccParser::ContentContext *contentContext = ctx->content(0);
		if (contentContext)
		{
			visit(ctx->content(0));
		}
		// set the jump to the end of the if
		cout << "\tjmp " << jumpEndIf << endl;
	}
	// set the jump label of the if's end
	cout << jumpEndIf << ":" << endl;
	return 0;
}

/**
 * @brief set a while structure using jumps in assembly code
 *
 * @param ifccParser::IfElseContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitWhileDo(ifccParser::WhileDoContext *ctx)
{
	// get a new jump label to come back to the while condition
	string jumpCondition = this->getJumpLabel();
	// get a new jump label to finish the while
	string jumpEnd = this->getJumpLabel();
	// set the jump label of the condition
	cout << jumpCondition << ":" << endl;
	// get the condition (var/const) of the while
	Element element = visit(ctx->expression());
	// set the condition in assembly code
	// if the condition is a var, compare it directly
	if (element.var)
	{
		cout << "\tcmpl $0, " << element << endl; // we're comparing the result with 0
	}
	// if the condition is a const, put it on EAX and compare it
	else
	{
		cout << MOVL << element << ", " << EAX << endl;
		cout << "\tcmpl $0, " << EAX << endl; // we're comparing the result with 0
	}
	// set the jump condition in assembly code (if condition is 0, jump to the end)
	cout << "\tje " << jumpEnd << endl;
	// set the while's content
	int returned = visit(ctx->content());
	cout << "# RETURNED " << returned << endl;
	// if the content doesn't contain a return statement, set the jump to the condition
	if (returned == 0)
	{
		// set the jump to the condition
		cout << "\tjmp " << jumpCondition << endl;
	}
	// set the jump label of the while's end
	cout << jumpEnd << ":" << endl;
	return 0;
}

/************************* EXPRESSIONS ****************************/

/**
 * @brief generate a new temporal variable and return its register index
 *
 * @return Element (variable or const)
 */

Element CodeGenVisitor::getNewTempVariable()
{
	//int index = (this->getVars().size() + 1) * 8;
	int index = maxOffset + 8;
	maxOffset = index;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->setVar(varname, index, "int");
	this->setVarUsed(varname);
	return Element(index, "int", true);
}

/**
 * @brief set the assembly code of the selected operation between leftval and rightval
 *
 * @param Element leftval
 * @param Element rightval
 * @param string operation
 * @return Element (variable or const)
 */

Element CodeGenVisitor::operationExpression(Element leftval, Element rightval, string operation)
{
	// if leftval and rightval are constants, execute the operation on compiler
	if (!leftval.var && !rightval.var)
	{
		int result;
		if (operation == "imull")
		{
			result = leftval.value * rightval.value;
		}
		else if (operation == "idiv")
		{
			result = leftval.value / rightval.value;
		}
		else if (operation == "mod")
		{
			result = leftval.value % rightval.value;
		}
		else if (operation == "add")
		{
			result = leftval.value + rightval.value;
		}
		else if (operation == "sub")
		{
			result = leftval.value - rightval.value;
		}
		else if (operation == "and")
		{
			result = leftval.value && rightval.value;
		}
		else if (operation == "or")
		{
			result = leftval.value || rightval.value;
		}
		else
		{
			result = leftval.value ^ rightval.value;
		}
		cout << "\t# result: " << result << endl;
		return Element(result, "int", false);
	}
	// if one element is a variable, execute the operation on assembly
	else
	{
		Element temporalVariable = getNewTempVariable();
		cout << MOVL << leftval << ", " << EAX << endl;
		if (operation == "idiv")
		{
			cout << "\tcltd" << endl;
			cout << "\tmovl " << rightval << ", " << ECX << endl;
			cout << "\tidivl " << ECX << endl;
		}
		else if (operation == "mod")
		{
			cout << "\tcltd" << endl;
			if (rightval.var)
			{
				cout << "\tidivl " << rightval << endl;
			}
			else
			{
				cout << "\tmovl " << rightval << ", " << ECX << endl;
				cout << "\tidivl " << ECX << endl;
			}
			cout << "\tmovl " << EDX << ", " << EAX << endl;
		}
		else
		{
			cout << "\t" << operation << " " << rightval << ", " << EAX << endl;
		}
		cout << MOVL << EAX << ", " << temporalVariable << endl;
		return temporalVariable;
	}
}

/**
 * @brief set an multiplication or division operation between leftval and rightval
 * 				according to the symbol of MULTDIV
 *
 * @param ifccParser::ExpressionMultDivContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitExpressionMultDivMod(ifccParser::ExpressionMultDivModContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	string operation = ctx->MULTDIVMOD()->getText();
	if (operation == "*")
	{
		cout << "\t# do " << leftval << " * " << rightval << endl;
		return operationExpression(leftval, rightval, "imull");
	}
	else if (operation == "/")
	{
		cout << "\t# do " << leftval << " / " << rightval << endl;
		return operationExpression(leftval, rightval, "idiv");
	}
	else
	{
		cout << "\t# do " << leftval << " % " << rightval << endl;
		return operationExpression(leftval, rightval, "mod");
	}
}

/**
 * @brief set an addition or substraction operation between leftval and rightval
 * 				according to the symbol of ADDSUB
 *
 * @param ifccParser::ExpressionAddSubContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitExpressionAddSub(ifccParser::ExpressionAddSubContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	string operation = ctx->ADDSUB()->getText();
	if (operation == "+")
	{
		cout << "\t# do " << leftval << " + " << rightval << endl;
		return operationExpression(leftval, rightval, "add");
	}
	else
	{
		cout << "\t# do " << leftval << " - " << rightval << endl;
		return operationExpression(leftval, rightval, "sub");
	}
}

/**
 * @brief set the operation AND between leftval and rightval
 *   		  the symbol can be && or &=
 *
 * @param ifccParser::ExpressionAndContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionAnd(ifccParser::ExpressionAndContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " - " << rightval << endl;
	return operationExpression(leftval, rightval, "and");
}

/**
 * @brief set the operation OR between leftval and rightval
 * 				the symbol can be |& or |=
 *
 * @param ifccParser::ExpressionOrContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionOr(ifccParser::ExpressionOrContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " | " << rightval << endl;
	return operationExpression(leftval, rightval, "or");
}

/**
 * @brief set the operation XOR between leftval and rightval
 *
 * @param ifccParser::ExpressionXorContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionXor(ifccParser::ExpressionXorContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " ^ " << rightval << endl;
	return operationExpression(leftval, rightval, "xor");
}

/**
 * @brief set the assembly code of the selected equality or inequality operation
 * 				between leftval and rightval
 *
 * @param Element leftval
 * @param Element rightval
 * @param string comp
 * @return Element (variable or const)
 */

Element CodeGenVisitor::operationCompExpression(Element leftval, Element rightval, string comp)
{
	// if leftval and rightval are constants, execute the comparation on compiler
	if (!leftval.var && !rightval.var)
	{
		int result;
		if (comp == "e")
		{
			result = leftval.value == rightval.value;
		}
		else if (comp == "ne")
		{
			result = leftval.value != rightval.value;
		}
		else if (comp == "g")
		{
			result = leftval.value > rightval.value;
		}
		else if (comp == "l")
		{
			result = leftval.value < rightval.value;
		}
		else if (comp == "ge")
		{
			result = leftval.value >= rightval.value;
		}
		else
		{
			result = leftval.value <= rightval.value;
		}
		cout << "\t# result: " << result << endl;
		return Element(result, "int", false);
	}
	// if one element is a variable, execute the comparation on assembly
	else
	{
		Element temporalVariable = getNewTempVariable();
		cout << MOVL << leftval << ", " << EAX << endl;
		cout << "\tcmpl " << rightval << ", " << EAX << endl;
		cout << "\tset" << comp << " %al" << endl;
		cout << "\tandb	$1, %al" << endl;
		cout << "\tmovzbl	%al, %eax" << endl;
		cout << "\tmovl	%eax, " << temporalVariable << endl;
		return temporalVariable;
	}
}

/**
 * @brief set the operation equal (==) between leftval and rightval
 *
 * @param ifccParser::ExpressionEqualContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionEqual(ifccParser::ExpressionEqualContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " == " << rightval << endl;
	return operationCompExpression(leftval, rightval, "e");
}

/**
 * @brief set the operation not equal (!=) between leftval and rightval
 *
 * @param ifccParser::ExpressionNotEqualContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionNotEqual(ifccParser::ExpressionNotEqualContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " != " << rightval << endl;
	return operationCompExpression(leftval, rightval, "ne");
}

/**
 * @brief set the operation greater (>) between leftval and rightval
 *
 * @param ifccParser::ExpressionGreaterContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionGreater(ifccParser::ExpressionGreaterContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " > " << rightval << endl;
	return operationCompExpression(leftval, rightval, "g");
}

/**
 * @brief set the operation less (<) between leftval and rightval
 *
 * @param ifccParser::ExpressionLessContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionLess(ifccParser::ExpressionLessContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " < " << rightval << endl;
	return operationCompExpression(leftval, rightval, "l");
}

/**
 * @brief set the operation greater or equal (>=) between leftval and rightval
 *
 * @param ifccParser::ExpressionGreaterEqualContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionGreaterEqual(ifccParser::ExpressionGreaterEqualContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " >= " << rightval << endl;
	return operationCompExpression(leftval, rightval, "ge");
}

/**
 * @brief set the operation less or equal (<=) between leftval and rightval
 *
 * @param ifccParser::ExpressionLessEqualContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionLessEqual(ifccParser::ExpressionLessEqualContext *ctx)
{
	Element leftval = visit(ctx->expression(0));
	Element rightval = visit(ctx->expression(1));
	cout << "\t# do " << leftval << " <= " << rightval << endl;
	return operationCompExpression(leftval, rightval, "le");
}

/**
 * @brief visit all the arguments of the function (expressions)
 * 				and save them in the arguments registers
 *
 * @param ifccParser::ArgsContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitArgs(ifccParser::ArgsContext *ctx)
{
	int counter = 0;
	for (auto arg : ctx->expression())
	{
		Element argument = visit(arg);
		// if there's less than 7 arguments, save the variable in the standard registers
		if (counter < 6)
		{
			cout << MOVL << argument << ", " << ARG_REGS[counter] << endl;
		}
		counter++;
	}
	return 0;
}

/**
 * @brief call a function, and return the register index
 * 				of the call's result as a temporal variable
 *
 * @param ifccParser::ExpressionFnContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionFn(ifccParser::ExpressionFnContext *ctx)
{
	visit(ctx->fnCall());
	Element temporalVariable = getNewTempVariable();
	cout << MOVL << EAX << ", " << temporalVariable << endl;
	return temporalVariable;
}

/**
 * @brief set a function call with its arguments
 *
 * @param ifccParser::FnCallContext ctx
 */

antlrcpp::Any CodeGenVisitor::visitFnCall(ifccParser::FnCallContext *ctx)
{
	ifccParser::ArgsContext *argsContext = ctx->args();
	if (argsContext)
	{
		visit(argsContext);
	}
	string fnName = ctx->VARNAME()->getText();
	string call;
#ifdef __APPLE__
	call = "\tcallq	_" + fnName;
#else
	call = "\tcallq	_" + fnName;
#endif
	cout << call << endl;
	return 0;
}

/**
 * @brief visit a value (var/const) inside parenthesis and return the register index
 *
 * @param ifccParser::ExpressionValueContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionPar(ifccParser::ExpressionParContext *ctx)
{
	return visit(ctx->expression());
}

/**
 * @brief visit a value (var/const) and return it
 *
 * @param ifccParser::ExpressionValueContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx)
{
	return visit(ctx->value());
}

/************************* HELPERS ****************************/

/**
 * @brief get the respective register (AL, EAX) for the given type (CHAR, INT)
 *
 * @param type
 * @return string
 */

string CodeGenVisitor::getRegister(string type)
{
	return type == "char" ? AL : EAX;
}

/**
 * @brief get the respective assembly Instruction (movl, movb)
 * 				for the given type (CHAR, INT)
 *
 * @param type
 * @return string
 */

string CodeGenVisitor::getMove(string type)
{
	return type == "char" ? "movb" : "movl";
}

/**
 * @brief get a bool indicating if there's an error in the program
 *
 * @return true
 * @return false
 */

bool CodeGenVisitor::getError()
{
	return this->error;
}

/**
 * @brief set error to true to indicate there's an error in the program
 *
 */

void CodeGenVisitor::setError()
{
	this->error = true;
}

/**
 * @brief set the current function name and intialize it's stack
 *
 * @param string name
 */

void CodeGenVisitor::setCurrentFunction(string name, string type)
{
	//cout << "\t#set current function " << endl;
	Function function;
	function.type = type;
	function.vars = {};
	//cout << "1" << endl;
	this->functions[name] = function;
	//cout << "2" << endl;
	this->currentFunction = name;
	//cout << "\t # end of set current function" << endl;
}



/**
 * @brief get the current function type
 *
 * @return string
 */

string CodeGenVisitor::getCurrentFunctionType()
{
	return this->functions[this->currentFunction].type;
}

/**
 * @brief get all the functions and their stack
 *
 * @return map<string, Function>
 */

map<string, Function> CodeGenVisitor::getFunctions()
{
	return this->functions;
}

/**
 * @brief get the stack (vars) from the current function
 *
 * @return map<string, int>
 */

map<string, Variable> CodeGenVisitor::getVars()
{
	return this->functions[this->currentFunction].vars;
}

/**
 * @brief get the register index of a selected variable of the current function
 *
 * @param varname
 * @return Variable (register index)
 */

Variable CodeGenVisitor::getVar(string varname)
{
	return this->functions[this->currentFunction].vars[varname];
}

/**
 * @brief set the register index of a selected variable of the current function
 *
 * @param varname
 * @param index
 * @param type
 */

void CodeGenVisitor::setVar(string varname, int index, string type)
{
	this->functions[this->currentFunction].vars[varname].type = type;
	this->functions[this->currentFunction].vars[varname].index = index;
}

/**
 * @brief set that a ver was already used in the current function to avoid warnings
 *
 * @param varname
 */

void CodeGenVisitor::setVarUsed(string varname)
{
	this->functions[this->currentFunction].vars[varname].used = true;
}

/**
 * @brief check if variable is not in the stack of the current function
 *
 * @param varname
 * @return true
 * @return false
 */

bool CodeGenVisitor::isVarDeclarated(string varname)
{
	return this->functions[this->currentFunction].vars.find(varname) != this->functions[this->currentFunction].vars.end();
}

/**
 * @brief get a new jump label on assembly code, to jump from one part of the code to another
 *
 * @return string
 */

string CodeGenVisitor::getJumpLabel()
{
	this->jumps++;
	return "LBB0_" + to_string(this->jumps);
}
