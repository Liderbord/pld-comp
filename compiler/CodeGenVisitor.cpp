#include "CodeGenVisitor.h"
using namespace std;

static const string START_MAC = ".globl	_main\n_main:\n";
static const string START_OTHERS = ".globl	main\nmain:\n";
static const string END = "\t# epilogue\n\tpopq\t %rbp  # restore %rbp from the stack\n\tret  # return to the caller (here the shell)\n";
static const string EAX = "%eax";
static const string ECX = "%ecx";

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
	string body;
	#ifdef __APPLE__
		body = START_MAC;
	#else
		body = START_OTHERS;
	#endif
	body += "\tendbr64\n \tpushq\t%rbp  # save %rbp on the stack\n\tmovq\t%rsp, %rbp # define %rbp for the current function";
	cout << body << endl;

	ifccParser::ContentContext * contentContext = ctx->content();
	if (contentContext) {
		cout << "\t# content" << endl;
		visit(contentContext);
	}
	string value = visit(ctx->value()).as<string>();
	cout << "\t" << "movl " << value << ", %eax" << endl << END;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)
{
	visit(ctx->init());
	ifccParser::ContentContext * contentContext = ctx->content();
	if (contentContext) {
		visit(contentContext);
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitInit(ifccParser::InitContext *ctx) 
{
	string type = ctx->TYPE()->getText();
	string varname = ctx->VARNAME()->getText();
	string value = visit(ctx->expression()).as<string>();
	int index = (this->vars.size() + 1) * 8;
	if (vars.find(varname) == vars.end()) {
		this->vars[varname] = index;
		cout << "\tmovl " + value + ", " << EAX << endl;
	  cout << "\tmovl " + EAX + ", -" + to_string(index) + "(%rbp)" << endl;
	} else {
		error = true;
	}
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx) 
{
	string returnval;
	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	if (varnameNode) {
		string varname = varnameNode->getText();
		string index = to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return returnval;
}

string CodeGenVisitor::operationExpression(string leftval, string rightval, string operation) {
	int index = (this->vars.size() + 1) * 8;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->vars[varname] = index;
	std::string regval = "-" + indexString + "(%rbp)";
	cout << "\tmovl " << leftval << ", " << EAX << endl;
  cout << "\t" << operation << " " << rightval << ", " << EAX << endl;
	cout << "\tmovl " << EAX << ", " << regval << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionMult(ifccParser::ExpressionMultContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationExpression(leftval, rightval, "imull");
}

antlrcpp::Any CodeGenVisitor::visitExpressionDiv(ifccParser::ExpressionDivContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	int index = (this->vars.size() + 1) * 8;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->vars[varname] = index;
	std::string regval = "-" + indexString + "(%rbp)";
	cout << "\tmovl " << leftval << ", " << EAX << endl;
	cout << "\tcltd" << endl;
  cout << "\tmovl " << rightval << ", " << ECX << endl;
	cout << "\tidivl " << ECX << endl;
	cout << "\tmovl " << EAX << ", " << regval << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionAdd(ifccParser::ExpressionAddContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationExpression(leftval, rightval, "add");
}

antlrcpp::Any CodeGenVisitor::visitExpressionSub(ifccParser::ExpressionSubContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	return operationExpression(leftval, rightval, "sub");
}

antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx) 
{
	return visit(ctx->value()).as<string>();
}



bool CodeGenVisitor::getWarning(){
	return this->warning;
}
bool CodeGenVisitor::getError(){
	return this->error;
}

void CodeGenVisitor::setWarning(bool val){
	this->warning=val;
}

void CodeGenVisitor::setError(bool val){
	this->error=val;
}