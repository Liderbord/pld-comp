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

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
	maxOffset = 0;
	for (auto fn : ctx->fn()) {
		visit(fn);
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitArgsDef(ifccParser::ArgsDefContext *ctx) 
{
	int counter = 0;
	for (auto varnameContext : ctx->VARNAME()) {
		string varname = varnameContext->getText();
		//int index = (this->getVars().size() + 1) * 8;
		int index = maxOffset + 8;
		if (this->isVarNoDeclarated(varname)) {
			this->setVar(varname, index);
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
		//int index = (this->getVars().size() + 1) * 8;
		int index = maxOffset + 8;
		maxOffset = index;
		// if varname already exists in vars, then it's an error
		if (this->isVarNoDeclarated(varname)) {
			this->setVar(varname, index);
			mapWarnings[varname] = 0;
			this->varsError[varname] = index;
			// look for the value and cout ASSEMBLY code
			if (paire.second != "")
			{
				string value = paire.second;
				cout << "\t# assign " + value + " to " +  to_string(index) + "(%rbp)" << endl;
				cout << "\tmovl " + value + ", " << EAX << endl;
				cout << "\tmovl " + EAX + ", -" + to_string(index) + "(%rbp)" << endl;
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
	string index;
	// check if the variable was already declared
	if (this->isVarNoDeclarated(varname)){
		mapWarnings[varname] = 1;
		index = to_string(this->getVar(varname));
		// apply the direct assignment
		cout << "\t# assigning " << value << " to " << varname << endl;
		cout << "\tmovl " + value + ", " << EAX << endl;
		cout << "\tmovl " + EAX + ", -" + index + "(%rbp)" << endl;
	} else if (varsError.find(varname) == varsError.end()) {
		// set an error
		error = true;
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
	// getting the number of declared values
	int nbrValues = ctx->CONST().size() - 1;
	// array of values
	vector<int> ArrayVal;
	// check that lengths are coherent
	if (length >= nbrValues)
	{
		
		// pushing tabName in vars, it points the last case of the stack so far (=first elt of the array)
		int index = maxOffset + length * 8;
		maxOffset = index;
		//this->vars[tabName] = index;
		this->setVar(tabName, index);
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
	
	// getting the Array's variable name
	string tabName = ctx->VARNAME()->getText();
	// getting the variable/const by using the Value visitor
	string value = visit(ctx->value()).as<string>();
	//value = value*8;

	string expr = visit(ctx->expression()).as<string>();

	// create temp var
	string temp = getNewTempVariable();

	//check if the table has already been declared
	
	if ( find(tabOfArrays.begin(), tabOfArrays.end(), tabName) != tabOfArrays.end() )
	{
		// get the destination index of the array
		string index = to_string(this->getVar(tabName));
		
		// TODO : Check if value > size of array, if its the case -> then its an error
		//mult
		cout << "\t# affect expression to lvalue (case of array)" << endl;
		cout << "\tmovq $8 , %rax" << endl;
		cout << "\tmovl " + value + " ,%ebx" << endl;
		cout << "\timulq %rbx, %rax"<< endl;
		cout << "\taddq $-" + index + ", %rax" << endl;
		cout << "\tmovq %rax, " + temp << endl;
		cout << "\tmovq %rbp, %rax" << endl;
		cout << "\taddq " + temp + " , %rax" << endl;
		cout << "\tmovq %rax, " + temp << endl;
		cout << "\tmovq " + temp + " , %rax" << endl;
		cout << "\tmovq " + expr + ", %r10" << endl;
		cout << "\tmovq %r10, (%rax)" << endl;
		
	}
	else
	{
   		error = true;
	}
	
	


	return 0;

}



antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx) 
{
	string returnval;
	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	ifccParser::ExpressionContext *expressionContext = ctx->expression();
	if (varnameNode) {
		string varname = varnameNode->getText();
		string index = to_string(this->getVar(varname));

		if (expressionContext){
			string exprString= visit(expressionContext);
			cout << "\t# access the case and return its value" << endl;
			string mult = this->operationExpression(exprString, "$8", "imull");
			string add = this->operationExpression("$-"+index, mult, "add ");
			cout << "\tmovq %rbp, %rax" << endl;
			cout << "\taddq " + add + ", %rax" << endl;
			cout << "\tmovq %rax, " + add << endl;
			cout << "\tmovq " + add + ", %rax" << endl;
			returnval = "(%rax)";
		}
		else 
		{
			returnval = "-" + index + "(%rbp)";
		}
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return returnval;
}

string CodeGenVisitor::getNewTempVariable() {
	//int index = (this->getVars().size() + 1) * 8;
	int index = maxOffset + 8;
	maxOffset = index;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->setVar(varname, index);
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

map<string, int> CodeGenVisitor::getVars(){
	return this->vars[this->currentFunction];
}

int CodeGenVisitor::getVar(string varname){
	return this->vars[this->currentFunction][varname];
}

void CodeGenVisitor::setVar(string varname, int index){
	this->vars[this->currentFunction][varname] = index;
}

bool CodeGenVisitor::isVarNoDeclarated(string varname) {
	return this->vars[this->currentFunction].find(varname) == this->vars[this->currentFunction].end();
}