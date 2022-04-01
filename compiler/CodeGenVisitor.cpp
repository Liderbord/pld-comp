#include "CodeGenVisitor.h"
#include <utility>
#include <vector>
using namespace std;

static const string START_MAC = ".globl	_main\n_main:\n";
static const string START_OTHERS = ".globl	main\nmain:\n";
static const string STACK = "\tendbr64\n \tpushq\t%rbp  # save %rbp on the stack\n\tmovq\t%rsp, %rbp # define %rbp for the current function";
static const string END = "\t# epilogue\n\tpopq\t %rbp  # restore %rbp from the stack\n\tret  # return to the caller (here the shell)\n";
static const string EAX = "%eax";
static const string ECX = "%ecx";
static const string EDX = "%edx";
static const string ARG_REGS[6] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static const string AL = "%al";

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
	for (auto fn : ctx->fn()) {
		visit(fn);
	}
	return 0;
}

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

antlrcpp::Any CodeGenVisitor::visitArgsDef(ifccParser::ArgsDefContext *ctx) 
{
	int counter = 0;
	for (auto varnameContext : ctx->VARNAME()) {
		string varname = varnameContext->getText();
		int index = (this->getVars().size() + 1) * 8;
		if (this->isVarNoDeclarated(varname)) {
			this->setVar(varname, index, "int");
			if (counter < 6) {
				cout << "\tmovl " << ARG_REGS[counter] << ", -" + to_string(index) + "(%rbp)" << endl;
			}
		} else {
			// TODO : print the error
			error = true;
		}
		counter++;	
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitArgs(ifccParser::ArgsContext *ctx) 
{
	int counter = 0;
	for (auto fn : ctx->expression()) {
		string regval = visit(fn);
		if (counter < 6) {
			cout << "\tmovl " << regval << ", " << ARG_REGS[counter] << endl;
		} else {
			string reg = (counter == 6 ? to_string((counter - 6)*8) : "") + "(%rsp)";
			cout << "\tmovl " << regval << ", " << reg << endl;
		}
		counter++;
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitFn(ifccParser::FnContext *ctx) 
{
	string head;
	string fnName = ctx->VARNAME()->getText();
	this->setCurrentFunction(fnName);
	#ifdef __APPLE__
		head = ".globl	_" + fnName + "\n_" + fnName + ":\n";
	#else
		head = ".globl	" + fnName + "\n" + fnName + ":\n";
	#endif
	cout << head << STACK << endl;
	
	ifccParser::ArgsDefContext * argsDefContext = ctx->argsDef();
	if (argsDefContext) {
		cout << "\t# args" << endl;
		visit(argsDefContext);
	}
	
	cout << "\t# content" << endl;
	visit(ctx->content());
	cout << END << endl;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)
{
	visitChildren(ctx);
	return 0;
}

antlrcpp::Any  CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx)
{
	string value = visit(ctx->expression()).as<string>();
	cout << "\t# return " << value << endl;
	cout << "\t" << "movl " << value << ", %eax" << endl;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitInit(ifccParser::InitContext *ctx)
{
	// we assume that type is INT for now
	string type = ctx->TYPE()->getText();
	// ctx->declaration() --> contexte de vector
	//  visit(ctx->declaration()) --> renvoie un vector
	vector<pair<string, string>> vectorVars = visit(ctx->declaration());
	for (auto paire : vectorVars)
	{
		string varname = paire.first;
		// type = INT
		int index = (this->getVars().size() + 1) * 8;
		// if varname already exists in vars, then it's an error
		if (this->isVarNoDeclarated(varname)) {
			this->setVar(varname, index, type);

			// Add assmebly comments for legibility
			cout << "\t# declare " << type << " " << varname;
			mapWarnings[varname] = 0;
			this->varsError[varname] = index;
			// look for the value and cout ASSEMBLY code
			if (paire.second != "")
			{
				string value = paire.second;
				cout << " and assign " + value << endl;
				cout << "\t" + getMove(type) + " " + value + ", " << getRegister(type) << endl;
				cout << "\t" + getMove(type) + " " + getRegister(type) + ", -" + to_string(index) + "(%rbp)" << endl;
				// look for the value and cout ASSEMBLY code
			}
			else
			{
				cout << endl;
			}
		} else {
			// TODO : print the error
			error = true;
		}
	}

	return 0;
}

antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx)
{
	// string type = ctx->TYPE()->getText();
	vector<pair<string, string>> vectorVars;
	for (auto contexte : ctx->dec())
	{
		// get the result of visitDec, ie: pair
		pair<string, string> paire = visit(contexte);
		// push into the vector
		vectorVars.push_back(paire);
	}
	return vectorVars;
}

antlrcpp::Any CodeGenVisitor::visitDec(ifccParser::DecContext *ctx)
{
	string varname = ctx->VARNAME()->getText();
	string value;
	if (ctx->expression())
	{
		value = visit(ctx->expression()).as<string>();
	}
	else
	{
		value = "";
	}

	pair<string, string> paire;
	paire.first = varname;
	paire.second = value;
	return paire;
}

antlrcpp::Any CodeGenVisitor::visitAffectationExpr(ifccParser::AffectationExprContext *ctx)
{
	// getting the variable 
	string varname = ctx->VARNAME()->getText();
	// getting the variable/const by using the expressionValue visitor
	string value = visit(ctx->expression()).as<string>();
	Variable var;
	// check if the variable was already declared
	if (this->isVarNoDeclarated(varname)){
		mapWarnings[varname] = 1;
		var = this->getVar(varname);
		// apply the direct assignment
		cout << "\t# assigning " << value << " to " << varname << endl;
		cout << "\t" + this->getMove(var.type) + " " + value + ", " << EAX << endl;
		cout << "\t" + this->getMove(var.type) + " " + EAX + ", -" + to_string(var.index) + "(%rbp)" << endl;
	} else if (varsError.find(varname) == varsError.end()) {
		// set an error
		error = true;
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx)
{
	string returnval;
	antlr4::tree::TerminalNode *varnameNode = ctx->VARNAME();
	if (varnameNode)
	{
		string varname = varnameNode->getText();
		string index = to_string(this->getVar(varname).index);
		returnval = "-" + index + "(%rbp)";
		return returnval;
	}
	antlr4::tree::TerminalNode *charNodes = ctx->CHAR();
	// evaluate if the constant is a char
	// if its a char convert it to its ascii value
	if (charNodes)
	{
		string character = charNodes->getText();
		return "$" + to_string(int(character[1]));
	}
	// get the constant
	string constant = ctx->CONST()->getText();
	return "$" + constant;
}

string CodeGenVisitor::getNewTempVariable() {
	int index = (this->getVars().size() + 1) * 8;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->setVar(varname, index, "int");
	return "-" + indexString + "(%rbp)";
}

string CodeGenVisitor::operationExpression(string leftval, string rightval, string operation) {
	string regval = getNewTempVariable();
	cout << "\tmovl " << leftval << ", " << EAX << endl;
  cout << "\t" << operation << " " << rightval << ", " << EAX << endl;
	cout << "\tmovl " << EAX << ", " << regval << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionMultDiv(ifccParser::ExpressionMultDivContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	string operation = ctx->MULTDIV()->getText();
	if (operation == "*") {
		cout << "\t# do " << leftval << " * " << rightval << endl;
		return operationExpression(leftval, rightval, "imull");
	} else {
		cout << "\t# do " << leftval << " / " << rightval << endl;
		string regval = getNewTempVariable();
		cout << "\tmovl " << leftval << ", " << EAX << endl;
		cout << "\tcltd" << endl;
		cout << "\tmovl\t" << rightval << ", " << ECX << endl;
		cout << "\tidivl\t" << ECX << endl;
		cout << "\tmovl\t" << EAX << ", " << regval << endl;
		return regval;
	}
}

antlrcpp::Any CodeGenVisitor::visitExpressionAddSub(ifccParser::ExpressionAddSubContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	string operation = ctx->ADDSUB()->getText();
	if (operation == "+") {
		cout << "\t# do " << leftval << " + " << rightval << endl;
		return operationExpression(leftval, rightval, "add ");
	} else {
		cout << "\t# do " << leftval << " - " << rightval << endl;
		return operationExpression(leftval, rightval, "sub ");
	}
}

antlrcpp::Any CodeGenVisitor::visitExpressionAnd(ifccParser::ExpressionAndContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " - " << rightval << endl;
	return operationExpression(leftval, rightval, "and");
}

antlrcpp::Any CodeGenVisitor::visitExpressionOr(ifccParser::ExpressionOrContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " | " << rightval << endl;
	return operationExpression(leftval, rightval, "or");
}

antlrcpp::Any CodeGenVisitor::visitExpressionXor(ifccParser::ExpressionXorContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " ^ " << rightval << endl;
	return operationExpression(leftval, rightval, "xor");
}

string CodeGenVisitor::operationCompExpression(string leftval, string rightval, string comp) {
	string regval = getNewTempVariable();
	cout << "\tmovl " << leftval << ", " << EAX << endl;
	cout << "\tcmpl " << rightval << ", " << EAX << endl;
	cout << "\tset" << comp << " %al" << endl;
	cout << "\tandb	$1, %al" << endl;
	cout << "\tmovzbl	%al, %eax" << endl;
	cout << "\tmovl	%eax, " << regval << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionEqual(ifccParser::ExpressionEqualContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "e");
}

antlrcpp::Any CodeGenVisitor::visitExpressionNotEqual(ifccParser::ExpressionNotEqualContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "ne");
}

antlrcpp::Any CodeGenVisitor::visitExpressionGreater(ifccParser::ExpressionGreaterContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "ge");
}

antlrcpp::Any CodeGenVisitor::visitExpressionLess(ifccParser::ExpressionLessContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationCompExpression(leftval, rightval, "le");
}

antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx) 
{
	return visit(ctx->value()).as<string>();
}

antlrcpp::Any CodeGenVisitor::visitExpressionFn(ifccParser::ExpressionFnContext *ctx) 
{
	string fnName = ctx->VARNAME()->getText();
	visit(ctx->args());
	string call;
	#ifdef __APPLE__
		call = "\tcallq	_" + fnName;
	#else
		call = "\tcallq	_" + fnName;
	#endif
	cout << call << endl;
	string regval = getNewTempVariable();
	cout << "\tmovl " << EAX << ", " << regval << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionPar(ifccParser::ExpressionParContext *ctx)
{
	return visit(ctx->expression()).as<string>();
}

antlrcpp::Any CodeGenVisitor::visitIfElse(ifccParser::IfElseContext *ctx) 
{
	string expval = visit(ctx->expression()).as<string>();
	this->jumps++;
	string jump = "LBB0_" + to_string(this->jumps);
	cout << "\tcmpl $0, " << expval << endl;
	cout << "\tje " << jump << endl;
	visit(ctx->content(0));
	cout << jump << ":" << endl;
	ifccParser::ContentContext * contentContext = ctx->content(1);
	if (contentContext) {
		visit(contentContext);
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitWhileDo(ifccParser::WhileDoContext *ctx) 
{
	this->jumps++;
	string jumpCondition = "LBB0_" + to_string(this->jumps);
	this->jumps++;
	string jumpEnd = "LBB0_" + to_string(this->jumps);
	cout << jumpCondition << ":" << endl;
	string expval = visit(ctx->expression()).as<string>();
	cout << "\tcmpl $0, " << expval << endl;
	cout << "\tje " << jumpEnd << endl;
	visit(ctx->content());
	cout << "\tjmp " << jumpCondition << endl;
	cout << jumpEnd << ":" << endl;
	return 0;
}


bool CodeGenVisitor::getError(){
	return this->error;
}

void CodeGenVisitor::setError(bool val){
	this->error=val;
}

void CodeGenVisitor::setCurrentFunction(string name){
	this->vars[name] = {};
	this->currentFunction = name;
}

string CodeGenVisitor::getCurrentFunction(){
	return this->currentFunction;
}

map<string, Variable> CodeGenVisitor::getVars()
{
	return this->vars[this->currentFunction];
}

Variable CodeGenVisitor::getVar(string varname)
{
	return this->vars[this->currentFunction][varname];
}

void CodeGenVisitor::setVar(string varname, int index, string type)
{
	this->vars[this->currentFunction][varname] = Variable{index, type};
}

bool CodeGenVisitor::isVarNoDeclarated(string varname) {
	return this->vars[this->currentFunction].find(varname) == this->vars[this->currentFunction].end();
}