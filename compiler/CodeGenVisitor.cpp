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

// registers
static const string AL = "%al";
static const string EAX = "%eax";
static const string ECX = "%ecx";
static const string EDX = "%edx";
static const string RBP = "(%rbp)";
static const string ARG_REGS[6] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"}; // standard argument registers

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
		int index = (this->getVars().size() + 1) * 8;
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
	string head;
	string fnName = ctx->VARNAME()->getText();
	string fnType = ctx->TYPE()->getText();
	this->setCurrentFunction(fnName, fnType);
// if the machine is from apple, use an _ before the name of the function
#ifdef __APPLE__
	head = ".globl	_" + fnName + "\n_" + fnName + ":\n";
#else
	head = ".globl	" + fnName + "\n" + fnName + ":\n";
#endif
	cout << head << STACK << endl;

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
	visitChildren(ctx);
	return 0;
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
	string value = visit(ctx->expression()).as<string>();
	// save the value in the register EAX
	cout << "\t# return " << value << endl;
	// set EAX as the return value
	cout << "\t"
			 << "movl " << value << ", %eax" << endl;
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
	// we assume that type is INT TODO: add char
	string type = ctx->TYPE()->getText();
	// get all the declaration in the line
	vector<pair<string, ifccParser::ExpressionContext *>> vectorVars = visit(ctx->declaration());
	// set and save each declaration
	for (auto pair : vectorVars)
	{
		string varname = pair.first;
		int index = (this->getVars().size() + 1) * 8;
		// if varname doesn't exists in the stack (vars), save it
		if (!this->isVarDeclarated(varname))
		{
			// save the variable in the stack of the current function
			this->setVar(varname, index, type);
			// if there's an expression (initial value), visit the expression and set it to the variable
			// if there's no expression, set the variable to the default value:
			// if the type is INT, set it to 0
			// TODO: if the type is CHAR, set it to ?
			string value = pair.second ? visit(pair.second).as<string>() : "$0";
			cout << "\t# declare " << type << " and assign " << value << endl;
			cout << MOVL + value + ", " << EAX << endl;
			cout << MOVL + EAX + ", -" + to_string(index) + RBP << endl;
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
		string value = visit(ctx->expression()).as<string>();
		// update the usages of the variable, to cancel the 'var no used' warning
		this->setVarUsed(varname);
		// set the direct assignment
		Variable var = this->getVar(varname);
		cout << "\t# assigning " << value << " to " << varname << endl;
		cout << "\t" + this->getMove(var.type) + " " + value + ", " << EAX << endl;
		cout << "\t" + this->getMove(var.type) + " " + EAX + ", -" + to_string(var.index) + "(%rbp)" << endl;
	}
	else
	{
		cout << "# ERROR: variable " << varname << " not declared" << endl;
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
	// if the value is a varname, get the register index from
	// the stack of the current function
	// and return it in the assembly format
	if (varnameNode)
	{
		string varname = varnameNode->getText();
		// if variable is declared, return it
		if (this->isVarDeclarated(varname))
		{
			return "-" + to_string(this->getVar(varname).index) + RBP;
		}
		// if variable is not declared, throw an error
		else
		{
			cout << "# ERROR: variable " << varname << " not declared" << endl;
			this->setError();
			return 1;
		}
	}
	antlr4::tree::TerminalNode *charNodes = ctx->CHAR();
	// if the value is a char, convert the ascii value
	// and return it in the assembly format
	if (charNodes)
	{
		string character = charNodes->getText();
		return "$" + to_string(int(character[1]));
	}
	// if the value is a number, convert the number as string
	// and return it in the assembly format
	string constant = ctx->CONST()->getText();
	return "$" + constant;
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
	string expval = visit(ctx->expression()).as<string>();
	// get the pointer to the possible else content
	ifccParser::ContentContext *elseContentContext = ctx->content(1);
	// get a new jump label to finish the if
	this->jumps++;
	string jumpEndIf = "LBB0_" + to_string(this->jumps);
	// set the condition in assembly code
	cout << MOVL << expval << ", " << EAX << endl;
	cout << "\tcmpl $0, " << EAX << endl; // we're comparing the result with 0
	// if there's a else's content
	if (elseContentContext)
	{
		// get a new jump label to omit the if content and pass directly to else
		this->jumps++;
		string jumpElse = "LBB0_" + to_string(this->jumps);
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
	this->jumps++;
	string jumpCondition = "LBB0_" + to_string(this->jumps);
	// get a new jump label to finish the while
	this->jumps++;
	string jumpEnd = "LBB0_" + to_string(this->jumps);
	// set the jump label of the condition
	cout << jumpCondition << ":" << endl;
	// get the condition (var/const) of the while
	string expval = visit(ctx->expression()).as<string>();
	// set the condition in assembly code
	cout << "\tcmpl $0, " << expval << endl; // we're comparing the result with 0
	// set the jump condition in assembly code (if condition is 0, jump to the end)
	cout << "\tje " << jumpEnd << endl;
	// set the while's content
	visit(ctx->content());
	// set the jump to the condition
	cout << "\tjmp " << jumpCondition << endl;
	// set the jump label of the while's end
	cout << jumpEnd << ":" << endl;
	return 0;
}

/************************* EXPRESSIONS ****************************/

/**
 * @brief generate a new temporal variable and return its register index
 *
 * @return string (register index)
 */

string CodeGenVisitor::getNewTempVariable()
{
	int index = (this->getVars().size() + 1) * 8;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->setVar(varname, index, "int");
	this->setVarUsed(varname);
	return "-" + indexString + RBP;
}

/**
 * @brief set the assembly code of the selected operation between leftval and rightval
 *
 * @param string leftval
 * @param string rightval
 * @param string operation
 * @return string (register index)
 */

string CodeGenVisitor::operationExpression(string leftval, string rightval, string operation)
{
	string regval = getNewTempVariable();
	cout << MOVL << leftval << ", " << EAX << endl;
	cout << "\t" << operation << " " << rightval << ", " << EAX << endl;
	cout << MOVL << EAX << ", " << regval << endl;
	return regval;
}

/**
 * @brief set an multiplication or division operation between leftval and rightval
 * 				according to the symbol of MULTDIV
 *
 * @param ifccParser::ExpressionMultDivContext ctx
 * @return antlrcpp::Any
 */

antlrcpp::Any CodeGenVisitor::visitExpressionMultDiv(ifccParser::ExpressionMultDivContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	string operation = ctx->MULTDIV()->getText();
	if (operation == "*")
	{
		cout << "\t# do " << leftval << " * " << rightval << endl;
		return operationExpression(leftval, rightval, "imull");
	}
	else
	{
		cout << "\t# do " << leftval << " / " << rightval << endl;
		string regval = getNewTempVariable();
		cout << MOVL << leftval << ", " << EAX << endl;
		cout << "\tcltd" << endl;
		cout << "\tmovl\t" << rightval << ", " << ECX << endl;
		cout << "\tidivl\t" << ECX << endl;
		cout << "\tmovl\t" << EAX << ", " << regval << endl;
		return regval;
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
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	string operation = ctx->ADDSUB()->getText();
	if (operation == "+")
	{
		cout << "\t# do " << leftval << " + " << rightval << endl;
		return operationExpression(leftval, rightval, "add ");
	}
	else
	{
		cout << "\t# do " << leftval << " - " << rightval << endl;
		return operationExpression(leftval, rightval, "sub ");
	}
}

/**
 * @brief set the operation AND between leftval and rightval
 *   		  the symbol can be && or &=
 *
 * @param ifccParser::ExpressionAndContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionAnd(ifccParser::ExpressionAndContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " - " << rightval << endl;
	return operationExpression(leftval, rightval, "and");
}

/**
 * @brief set the operation OR between leftval and rightval
 * 				the symbol can be |& or |=
 *
 * @param ifccParser::ExpressionOrContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionOr(ifccParser::ExpressionOrContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " | " << rightval << endl;
	return operationExpression(leftval, rightval, "or");
}

/**
 * @brief set the operation XOR between leftval and rightval
 *
 * @param ifccParser::ExpressionXorContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionXor(ifccParser::ExpressionXorContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " ^ " << rightval << endl;
	return operationExpression(leftval, rightval, "xor");
}

/**
 * @brief set the assembly code of the selected equality or inequality operation
 * 				between leftval and rightval
 *
 * @param string leftval
 * @param string rightval
 * @param string comp
 * @return string (register index)
 */

string CodeGenVisitor::operationCompExpression(string leftval, string rightval, string comp)
{
	string regval = getNewTempVariable();
	cout << MOVL << leftval << ", " << EAX << endl;
	cout << "\tcmpl " << rightval << ", " << EAX << endl;
	cout << "\tset" << comp << " %al" << endl;
	cout << "\tandb	$1, %al" << endl;
	cout << "\tmovzbl	%al, %eax" << endl;
	cout << "\tmovl	%eax, " << regval << endl;
	return regval;
}

/**
 * @brief set the operation equal (==) between leftval and rightval
 *
 * @param ifccParser::ExpressionEqualContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionEqual(ifccParser::ExpressionEqualContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "e");
}

/**
 * @brief set the operation not equal (!=) between leftval and rightval
 *
 * @param ifccParser::ExpressionNotEqualContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionNotEqual(ifccParser::ExpressionNotEqualContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "ne");
}

/**
 * @brief set the operation greater (>) between leftval and rightval
 *
 * @param ifccParser::ExpressionGreaterContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionGreater(ifccParser::ExpressionGreaterContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "g");
}

/**
 * @brief set the operation less (<) between leftval and rightval
 *
 * @param ifccParser::ExpressionLessContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionLess(ifccParser::ExpressionLessContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "l");
}

/**
 * @brief set the operation greater or equal (>=) between leftval and rightval
 *
 * @param ifccParser::ExpressionGreaterEqualContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionGreaterEqual(ifccParser::ExpressionGreaterEqualContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "ge");
}

/**
 * @brief set the operation less or equal (<=) between leftval and rightval
 *
 * @param ifccParser::ExpressionLessEqualContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionLessEqual(ifccParser::ExpressionLessEqualContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
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
	for (auto fn : ctx->expression())
	{
		string regval = visit(fn);
		// if there's less than 7 arguments, save the variable in the standard registers
		if (counter < 6)
		{
			cout << MOVL << regval << ", " << ARG_REGS[counter] << endl;
			// if there's more than 6 arguments, save the variable in the RSP
		}
		else
		{
			string reg = (counter == 6 ? to_string((counter - 6) * 8) : "") + "(%rsp)";
			cout << MOVL << regval << ", " << reg << endl;
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
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionFn(ifccParser::ExpressionFnContext *ctx)
{
	visit(ctx->fnCall());
	string regval = getNewTempVariable();
	cout << MOVL << EAX << ", " << regval << endl;
	return regval;
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
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionPar(ifccParser::ExpressionParContext *ctx)
{
	return visit(ctx->expression()).as<string>();
}

/**
 * @brief visit a value (var/const) and return the register index
 *
 * @param ifccParser::ExpressionValueContext ctx
 * @return string (register index)
 */

antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx)
{
	return visit(ctx->value()).as<string>();
}

/************************* HELPERS ****************************/

// TODO: add def

string CodeGenVisitor::getRegister(string type)
{
	if (type == "char")
	{
		return AL;
	}
	return EAX;
}

string CodeGenVisitor::getMove(string type)
{
	if (type == "char")
	{
		return "movb";
	}
	return "movl";
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
	this->functions[name] = function;
	this->currentFunction = name;
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
