#include "CodeGenVisitor.h"
using namespace std;

static const string START_MAC = ".globl	_main\n_main:\n";
static const string START_OTHERS = ".globl	main\nmain:\n";
static const string END = "\tret\n";


antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
<<<<<<< HEAD
	//cout << "prog" << endl;
	string body = __APPLE__ ? START_MAC : START_OTHERS;
	ifccParser::ContentContext * contentContext = ctx->content();
=======

	std::cout << "prog" << std::endl;
	std::string body;
#ifdef __APPLE__
	body = START_MAC;
#else
	body = START_OTHERS;
#endif
		ifccParser::ContentContext *contentContext = ctx->content();

>>>>>>> 83a76847f5174ab46de8574321f6bc3d75a6eee9
	if (contentContext) {
		cout << "#->content" << endl;
		string content = visit(contentContext).as<string>();
		body += "\t" + content + "\n";
	}
	string returnValue = visit(ctx->returnValue()).as<string>();
	cout << body << "\t" << returnValue << "\n" << END;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx) 
{
	cout << "#content" << endl;
	string init = visit(ctx->init()).as<string>();
	cout<<"#";
	cout << init << endl;
	ifccParser::ContentContext * contentContext = ctx->content();
	if (!contentContext) {
		return init;
	} else {
		string content = visit(contentContext).as<string>();
		return init + content;
	}
}

antlrcpp::Any CodeGenVisitor::visitInit(ifccParser::InitContext *ctx) 
{
	cout << "#init" << endl;
	string type = ctx->TYPE()->getText();
	cout<< "#";
	cout << type << endl;
	string varname = ctx->VARNAME()->getText();
	string constval = ctx->CONST()->getText();
	int index = (this->vars.size() + 1) * 8;
	this->vars[varname] = index;
	return "movl $" + constval + ", -" + to_string(index) + "(%rbp)";
}

antlrcpp::Any CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx) 
{
	cout << "#returnValue" << endl;
	string returnval;
	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	if (varnameNode) {
		string varname = varnameNode->getText();
		string index = to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return "movl " + returnval + ", %eax";
}
