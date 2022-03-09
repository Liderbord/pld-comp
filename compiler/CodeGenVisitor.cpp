#include "CodeGenVisitor.h"
using namespace std;


static const string START_MAC = ".globl	_main\n_main:\n";
static const std::string START_OTHERS = ".globl	main\nmain:\n";
static const std::string END = "# epilogue\n\tpopq\t %rbp  # restore %rbp from the stack\n\tret  # return to the caller (here the shell)\n";

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{

	cout << "# prog" << std::endl;
	string body;
#ifdef __APPLE__
	body = START_MAC;
#else
	body = START_OTHERS;
#endif
	ifccParser::ContentContext *contentContext = ctx->content();
	body += "\tendbr64\n \tpushq\t%rbp  # save %rbp on the stack\n\tmovq\t%rsp, %rbp # define %rbp for the current function\n";

	if (contentContext)
	{
		cout << "# ->content" << endl;
		string content = visit(contentContext).as<string>();

		body += "\t" + content + "\n";
	}
	string returnValue = visit(ctx->returnValue()).as<string>();
	cout << body << "\t" << returnValue << "\n"
		 << END;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)
{

	cout << "# content" << endl;
	string init = visit(ctx->init()).as<string>();
	cout << "# ";
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

	cout << "# init" << endl;
	string type = ctx->TYPE()->getText();
	cout << "# ";
	cout << type << endl;
	string varname = ctx->VARNAME()->getText();
	string constval = ctx->CONST()->getText();

	int index = (this->vars.size() + 1) * 8;
	this->vars[varname] = index;
	return "movl\t$" + constval + ", -" + to_string(index) + "(%rbp)";
}

antlrcpp::Any CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx) 
{

	cout << "# returnValue" << endl;
	string returnval;

	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	if (varnameNode) {
		string varname = varnameNode->getText();
		string index = to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return "movl\t" + returnval + ", %eax";
}
