#include "CodeGenVisitor.h"
using namespace std;

static const string START_MAC = ".globl	_main\n_main:\n";
static const string START_OTHERS = ".globl	main\nmain:\n";
static const string END = "\t# epilogue\n\tpopq\t %rbp  # restore %rbp from the stack\n\tret  # return to the caller (here the shell)\n";

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
	this->vars[varname] = index;
	cout << "\tmovl " + value + ", -" + to_string(index) + "(%rbp)" << endl;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx) 
{
	string returnval;
	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	if (varnameNode) {
		std::string varname = varnameNode->getText();
		std::string index = std::to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return returnval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionMult(ifccParser::ExpressionMultContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	cout << "\tmovl " << leftval << ", %eax" << endl;
	string rightval = visit(ctx->expression(1)).as<string>();
  cout << "\timull " << rightval << ", %eax" << endl;
	return string("%eax");
}

antlrcpp::Any CodeGenVisitor::visitExpressionAdd(ifccParser::ExpressionAddContext *ctx) 
{
	string leftval = visit(ctx->expression(0)).as<string>();
	cout << "\tmovl " << leftval << ", %eax" << endl;
	string rightval = visit(ctx->expression(1)).as<string>();
  cout << "\tiaddl " << rightval << ", %eax" << endl;
	return string("%eax");
}

antlrcpp::Any CodeGenVisitor::visitExpressionPar(ifccParser::ExpressionParContext *ctx)
{
	//visitChildren();
}



antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx) 
{
	return visit(ctx->value()).as<string>();
}

antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx)
{
	string type = ctx->TYPE()->getText();
	for (auto varname: ctx->VARNAME()) {
		string name = varname->getText();
		int index = (this->vars.size() + 1) * 8; //int
		this->vars[name] = index;
	}	
	return 0;
}





