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
		// int index = (this->getVars().size() + 1) * 8;
		// int index = maxOffset + 8;
		// maxOffset = index;

		int index = this->getMaxOffset() + 8;
		this->setMaxOffset(index);

		// int index = this->getnewindex

		// if the name of the argument is not repeated, save it in the map
		if (!this->isVarDeclarated(varname))
		{
			this->setVar(varname, index, type);
			// if there's less than 7 arguments, save the variable in the standard registers
			if (counter < 6)
			{
				this->fout << MOVL << ARG_REGS[counter] << ", -" << to_string(index) << RBP << endl;
			}
		}
		// if the arguments is repeated, set an error
		else
		{
			this->fout << "# ERROR: same argument " << varname << " declared multiple times" << endl;
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
	// STEP 1: visit the tree, feed the stack and get the assembly code
	string head;
	string fnName = ctx->VARNAME()->getText();
	string fnType = ctx->TYPE()->getText();
	// set the name of the current function and initialize it's data
	this->setCurrentFunction(fnName, fnType);

// if the machine is from apple, use an _ before the name of the function
#ifdef __APPLE__
	head = ".globl	_" + fnName + "\n_" + fnName + ":\n";
#else
	head = ".globl	" + fnName + "\n" + fnName + ":\n";
#endif

	// visit the arguments of the function, if there are any
	ifccParser::ArgsDefContext *argsDefContext = ctx->argsDef();
	if (argsDefContext)
	{
		this->fout << "\t# args" << endl;
		visit(argsDefContext);
	}

	// visit the body of the function, if there is one
	ifccParser::ContentContext *contentContext = ctx->content();
	if (contentContext)
	{
		this->fout << "\t# content" << endl;
		visit(contentContext);
	}
	// if no body, return xorl %eax (good practice from gcc)
	else
	{
		this->fout << "\txorl	%eax, %eax" << endl;
	}

	// define the amount of stack data used by the function (vars including arrays)
	int stackSize = (this->getVars().size() + 1) * 8;

	// STEP 2: print the assembly code of the current function
	// print the standard head of the function
	cout << head << STACK << endl;

	// we move the RSP to protect the stack of the current function
	cout << "\tsubq	$" << stackSize << ", %rsp" << endl;

	// print the assembly code of the function's body (statements)
	cout << this->fout.str() << endl;

	// we move back the RSP at the beggining of the function
	cout << "\taddq	$" << stackSize << ", %rsp" << endl;

	// print the standard footer of the function
	cout << END << endl;
	return 0;
}

/**
 * @brief visit all the statements of the function, starting with the first one
 * 				and return 1 if there's a return inside or 0 otherwise
 *
 * @param ifccParser::ContentContext ctx
 * @return antlrcpp::Any (int)
 */

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)
{
	// if the statement is a return, we don't need to progress further in the recursion
	// and we can just set the assembly code of the return
	// we return 1 to indicate that inside this content there's a return
	ifccParser::ReturnValueContext *returnValueContext = ctx->returnValue();
	if (returnValueContext)
	{
		visit(returnValueContext);
		return 1;
	}
	// if there's no return, we progress further in the recursion
	// we return 0 to indicate that inside this content there's no return
	else
	{
		// if there's more content, we return it's result
		if (ctx->content())
		{
			return visitChildren(ctx);
		}
		// if there is not more content, we return 0 to indicate no return
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
		this->fout << "\t# return " << element << endl;
		this->fout << "\t"
							 << "movl " << element << ", %eax" << endl;
	}
	// if the element is not the same type as the function's return type, set an error
	else
	{
		this->fout << "# ERROR: return type mismatch" << endl;
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
	string type = ctx->TYPE()->getText();
	// get all the declaration in the line
	vector<pair<string, ifccParser::ExpressionContext *>> vectorVars = visit(ctx->declaration());
	// set and save each declaration
	for (auto pair : vectorVars)
	{
		string varname = pair.first;

		// get a index to allocate the variable in the stack
		int index = this->getMaxOffset() + 8;
		this->setMaxOffset(index);

		// if varname doesn't exists in the stack (vars), save it
		if (!this->isVarDeclarated(varname))
		{
			// save the variable in the stack of the current function
			this->setVar(varname, index, type);
			// if there's an expression (initial value), visit the expression and set it to the variable
			// if there's no expression, set the variable to the default value
			string value = pair.second ? visit(pair.second).as<Element>().getValue() : "$0";
			this->fout << "\t# declare " << type << " and assign " << value << endl;
			this->fout << "\t" + getMove(type) + " " + value + ", " << getRegister(type) << endl;
			this->fout << "\t" + getMove(type) + " " + getRegister(type) + ", -" + to_string(index) + "(%rbp)" << endl;
		}
		else
		{
			this->fout << "# ERROR: variable " << varname << " already declared" << endl;
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
		this->fout << "\t# assigning " << element << " to " << varname << endl;
		this->fout << "\t" + this->getMove(var.type) << " " << element << ", " << this->getRegister(var.type) << endl;
		this->fout << "\t" + this->getMove(var.type) << " " << this->getRegister(var.type) << ", -" << to_string(var.index) << RBP << endl;
	}
	else
	{
		this->fout << "# ERROR: variable " << varname << " not declared" << endl;
		this->setError();
	}
	return 0;
}

/**
 * @brief used to declare an array, put it in vars and generate assembly code in case of affect
 *
 * @param ctx ArrayDeclarationContext *ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitArrayDeclaration(ifccParser::ArrayDeclarationContext *ctx)
{

	// we assume that type is INT for now
	string type = ctx->TYPE()->getText();
	// getting the name of the array, CONST is a non-terminal symbol, so no visit
	string tabName = ctx->VARNAME()->getText();
	// getting its length, which is in CONST()[0]
	int length = stoi(ctx->CONST(0)->getText());
	// getting the number of declared values
	int nbrValues = ctx->CONST().size() - 1;

	// check that lengths are coherent
	if (length >= nbrValues)
	{

		// pushing tabName in vars, it points the last case of the stack so far (=first elt of the array)
		int index = this->getMaxOffset() + length * 8;
		this->setMaxOffset(index);
		this->setVar(tabName, index, type);

		for (int i = 1; i <= nbrValues; i++)
		{
			// arrayVal.push_back(atoi(ctx->CONST(i)->getText()));
			string value = ctx->CONST(i)->getText();
			// moving the values to the stack
			this->fout << "\t# moving " << value << " to its location in the stack" << endl;
			this->fout << "\tmovl $" << value << ", -" << to_string(index - (i - 1) * 8) << "(%rbp)" << endl;
		}
	}
	else
	{
		this->fout << "# ERROR: array " << tabName << " has a length of " << length << " but only " << nbrValues << " values are declared" << endl;
		this->setError();
	}
	return 0;
}

/**
 * @brief get the adress of the destination case of the array, and put the value in it
 *
 * @param ctx AffectationArrayContext *ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitAffectationArray(ifccParser::AffectationArrayContext *ctx)
{
	string tabName = ctx->VARNAME()->getText();
	// getting the variable/const by using the Value visitor
	Element value = visit(ctx->expression(0));
	Element expr = visit(ctx->expression(1));

	//  create temp var
	Element temp = getNewTempVariable();

	// check if the table has already been declared

	if (this->isVarDeclarated(tabName))
	{

		// get the destination index of the array
		Variable var = this->getVar(tabName);
		string index = to_string(var.index);
		// set assembly code of the affectation
		this->fout << "\t# affect expression to lvalue (case of array)" << endl;
		this->fout << "\tmovq $8 , %rax" << endl;
		this->fout << "\tmovl " << value << " ,%ebx" << endl;
		this->fout << "\timulq %rbx, %rax" << endl;
		this->fout << "\taddq $-" + index + ", %rax" << endl;
		this->fout << "\tmovq %rax, " << temp << endl;
		this->fout << "\tmovq %rbp, %rax" << endl;
		this->fout << "\taddq " << temp << " , %rax" << endl;
		this->fout << "\tmovq %rax, " << temp << endl;
		this->fout << "\tmovq " << temp << " , %rax" << endl;
		this->fout << "\tmovq " << expr << ", %r10" << endl;
		this->fout << "\tmovq %r10, (%rax)" << endl;
	}
	else
	{
		this->fout << "# ERROR: array " << tabName << " not declared" << endl;
		this->setError();
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
		// if variable is declared, return it
		if (this->isVarDeclarated(varname))
		{
			// if there's an expression, means that we're in the case of an array
			if (expressionContext)
			{
				// get the content of the storage in the index selected of the array
				Element temporalVariable = getNewTempVariable();
				Element array = Element(this->getVar(varname).index, "int", true);
				Element eight = Element(8, "int", false);
				Element index = visit(expressionContext);
				this->fout << "\t# access the case and return its value" << endl;
				Element indexOnBits = this->operationExpression(index, eight, "imull");
				this->fout << "\taddq " << indexOnBits << ", %rbp" << endl;
				this->fout << "\tmovl -" << array.value << "(%rbp), %eax" << endl;
				this->fout << "\tsubq " << indexOnBits << ", %rbp" << endl;
				this->fout << "\tmovl %eax, " << temporalVariable << endl;
				return temporalVariable;
			}
			else
			{
				return Element(this->getVar(varname).index, this->getVar(varname).type, true);
			}
		}
		// if variable is not declared, throw an error
		else
		{
			this->fout << "# ERROR: variable " << varname << " not declared" << endl;
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
	this->fout << MOVL << expval << ", " << EAX << endl;
	this->fout << "\tcmpl $0, " << EAX << endl; // we're comparing the result with 0
	// if there's a else's content
	if (elseContentContext)
	{
		// get a new jump label to omit the if content and pass directly to else
		string jumpElse = this->getJumpLabel();
		// set the jump condition in assembly code (if condition is 0, jump to else)
		this->fout << "\tje " << jumpElse << endl;
		// set the if's content
		ifccParser::ContentContext *contentContext = ctx->content(0);
		if (contentContext)
		{
			visit(contentContext);
		}
		// set the jump to the end of the if
		this->fout << "\tjmp " << jumpEndIf << endl;
		// set the jump label of the else
		this->fout << jumpElse << ":" << endl;
		// set the else's content
		visit(elseContentContext);
		// set the jump to the end of the if
		this->fout << "\tjmp " << jumpEndIf << endl;
	}
	// if there isn't else's content
	else
	{
		// set the jump condition in assembly code (if condition is 0, jump to the end)
		this->fout << "\tje " << jumpEndIf << endl;
		// set the if content in assembly code
		ifccParser::ContentContext *contentContext = ctx->content(0);
		if (contentContext)
		{
			visit(ctx->content(0));
		}
		// set the jump to the end of the if
		this->fout << "\tjmp " << jumpEndIf << endl;
	}
	// set the jump label of the if's end
	this->fout << jumpEndIf << ":" << endl;
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
	this->fout << jumpCondition << ":" << endl;
	// get the condition (var/const) of the while
	Element element = visit(ctx->expression());
	// set the condition in assembly code
	// if the condition is a var, compare it directly
	if (element.var)
	{
		this->fout << "\tcmpl $0, " << element << endl; // we're comparing the result with 0
	}
	// if the condition is a const, put it on EAX and compare it
	else
	{
		this->fout << MOVL << element << ", " << EAX << endl;
		this->fout << "\tcmpl $0, " << EAX << endl; // we're comparing the result with 0
	}
	// set the jump condition in assembly code (if condition is 0, jump to the end)
	this->fout << "\tje " << jumpEnd << endl;
	// set the while's content
	int returned = visit(ctx->content());
	this->fout << "# RETURNED " << returned << endl;
	// if the content doesn't contain a return statement, set the jump to the condition
	if (returned == 0)
	{
		// set the jump to the condition
		this->fout << "\tjmp " << jumpCondition << endl;
	}
	// set the jump label of the while's end
	this->fout << jumpEnd << ":" << endl;
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
	// get a memory address for the new variable
	int index = this->getMaxOffset() + 8;
	// save new index as the top of the stack for the current function
	this->setMaxOffset(index);
	// save the index in the compilator stack of the current function
	string indexString = to_string(index);
	string varname = "temp" + indexString; // name compose of temp + index, ex: temp0
	this->setVar(varname, index, "int");
	this->setVarUsed(varname);
	// return the variable as an Element (variable/const)
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
		this->fout << "\t# result: " << result << endl;
		return Element(result, "int", false);
	}
	// if one element is a variable, execute the operation on assembly
	else
	{
		Element temporalVariable = getNewTempVariable();
		this->fout << MOVL << leftval << ", " << EAX << endl;
		if (operation == "idiv")
		{
			this->fout << "\tcltd" << endl;
			this->fout << "\tmovl " << rightval << ", " << ECX << endl;
			this->fout << "\tidivl " << ECX << endl;
		}
		else if (operation == "mod")
		{
			this->fout << "\tcltd" << endl;
			if (rightval.var)
			{
				this->fout << "\tidivl " << rightval << endl;
			}
			else
			{
				this->fout << "\tmovl " << rightval << ", " << ECX << endl;
				this->fout << "\tidivl " << ECX << endl;
			}
			this->fout << "\tmovl " << EDX << ", " << EAX << endl;
		}
		else
		{
			this->fout << "\t" << operation << " " << rightval << ", " << EAX << endl;
		}
		this->fout << MOVL << EAX << ", " << temporalVariable << endl;
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
		this->fout << "\t# do " << leftval << " * " << rightval << endl;
		return operationExpression(leftval, rightval, "imull");
	}
	else if (operation == "/")
	{
		this->fout << "\t# do " << leftval << " / " << rightval << endl;
		return operationExpression(leftval, rightval, "idiv");
	}
	else
	{
		this->fout << "\t# do " << leftval << " % " << rightval << endl;
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
	string operation = ctx->ADDSUB->getText();
	if (operation == "+")
	{
		this->fout << "\t# do " << leftval << " + " << rightval << endl;
		return operationExpression(leftval, rightval, "add");
	}
	else
	{
		this->fout << "\t# do " << leftval << " - " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " - " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " | " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " ^ " << rightval << endl;
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
		this->fout << "\t# result: " << result << endl;
		return Element(result, "int", false);
	}
	// if one element is a variable, execute the comparation on assembly
	else
	{
		Element temporalVariable = getNewTempVariable();
		this->fout << MOVL << leftval << ", " << EAX << endl;
		this->fout << "\tcmpl " << rightval << ", " << EAX << endl;
		this->fout << "\tset" << comp << " %al" << endl;
		this->fout << "\tandb	$1, %al" << endl;
		this->fout << "\tmovzbl	%al, %eax" << endl;
		this->fout << "\tmovl	%eax, " << temporalVariable << endl;
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
	this->fout << "\t# do " << leftval << " == " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " != " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " > " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " < " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " >= " << rightval << endl;
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
	this->fout << "\t# do " << leftval << " <= " << rightval << endl;
	return operationCompExpression(leftval, rightval, "le");
}

/**
 * @brief set the operation negation (!) of a expression
 *
 * @param ifccParser::ExpressionNegationContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionNegation(ifccParser::ExpressionNegationContext *ctx)
{
	Element element = visit(ctx->expression());
	Element temporalVariable = getNewTempVariable();
	this->fout << "\t# do ! " << element << endl;
	this->fout << MOVL << element << ", " << EAX << endl;
	this->fout << "cmpl $0, " << EAX << endl;
	this->fout << "\tsetne %al" << endl;
	this->fout << "\txorb $-1, %al" << endl;
	this->fout << "\tandb $1, %al" << endl;
	this->fout << "\tmovzbl %al, %eax" << endl;
	this->fout << "\tmovl %eax, " << temporalVariable << endl;
	return temporalVariable;
}

/**
 * @brief set the operation negative (-) of a expression
 *
 * @param ifccParser::ExpressionNegativeContext ctx
 * @return Element (variable or const)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionNegative(ifccParser::ExpressionNegativeContext *ctx)
{
	Element element = visit(ctx->expression());
	Element temporalVariable = getNewTempVariable();
	this->fout << "\t# do - " << element << endl;
	this->fout << "\txorl %eax, %eax" << endl;
	this->fout << "\tsubl " << element << ", %eax" << endl;
	this->fout << MOVL << "%eax, " << temporalVariable << endl;
	return temporalVariable;
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
			this->fout << MOVL << argument << ", " << ARG_REGS[counter] << endl;
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
	this->fout << MOVL << EAX << ", " << temporalVariable << endl;
	return temporalVariable;
}

/**
 * @brief set a function call with its arguments
 *
 * @param ifccParser::FnCallContext ctx
 */

antlrcpp::Any CodeGenVisitor::visitFnCall(ifccParser::FnCallContext *ctx)
{
	// visit all the arguments and assign them to the EDI registers (6 max)
	ifccParser::ArgsContext *argsContext = ctx->args();
	if (argsContext)
	{
		visit(argsContext);
	}
	// call the selected function
	string fnName = ctx->VARNAME()->getText();
	string call;
#ifdef __APPLE__
	call = "\tcallq	_" + fnName;
#else
	call = "\tcallq	" + fnName;
#endif
	this->fout << call << endl;
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
	Function function;
	function.type = type;
	function.vars = {};
	this->content = "";
	this->fout.str("");
	this->functions[name] = function;
	this->currentFunction = name;
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
 * @brief get the current max offset of the function
 *
 * @return int
 */

int CodeGenVisitor::getMaxOffset()
{
	return this->functions[this->currentFunction].maxOffset;
}

/**
 * @brief set the current max offset of the function
 *
 * @param value
 */

void CodeGenVisitor::setMaxOffset(int value)
{
	this->functions[this->currentFunction].maxOffset = value;
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
