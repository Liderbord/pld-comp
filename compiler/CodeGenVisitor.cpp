#include "CodeGenVisitor.h"
using namespace std;

static const string START_MAC = ".globl	_main\n_main:\n";
static const std::string START_OTHERS = ".globl	main\nmain:\n";
static const std::string END = "# epilogue\n\tpopq\t %rbp  # restore %rbp from the stack\n\tret  # return to the caller (here the shell)\n";

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
	string body;
#ifdef __APPLE__
	body = START_MAC;
#else
	body = START_OTHERS;
#endif
	ifccParser::ContentContext *contentContext = ctx->content();
	body += "\tendbr64\n \tpushq\t%rbp  # save %rbp on the stack\n\tmovq\t%rsp, %rbp # define %rbp for the current function\n";

	ifccParser::ContentContext * contentContext = ctx->content();
	std::cout << body << std::endl;
	if (contentContext) {
		std::string content = visit(contentContext).as<std::string>();
		std::cout << "\t" << content << std::endl;
	}
	std::string value = visit(ctx->value()).as<std::string>();
	std::cout << "\t" << "movl " << value << ", %eax" << std::endl << END;
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
	std::string type = ctx->TYPE()->getText();
	std::string varname = ctx->VARNAME()->getText();
	std::string value = visit(ctx->expression()).as<std::string>();
	int index = (this->vars.size() + 1) * 8;
	this->vars[varname] = index;
	return "movl " + value + ", -" + std::to_string(index) + "(%rbp)";
}

antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx) 
{
	std::string returnval;
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
	std::string leftval = visit(ctx->expression(0)).as<std::string>();
	std::cout << "\tmovl	" << leftval << ", %eax" << std::endl;
	std::string rightval = visit(ctx->expression(1)).as<std::string>();
  std::cout << "\timull	" << rightval << ", %eax" << std::endl;
	return std::string("%eax");
}

antlrcpp::Any CodeGenVisitor::visitExpressionAdd(ifccParser::ExpressionAddContext *ctx) 
{
	std::string leftval = visit(ctx->expression(0)).as<std::string>();
	std::cout << "\tmovl	" << leftval << ", %eax" << std::endl;
	std::string rightval = visit(ctx->expression(1)).as<std::string>();
  std::cout << "\tiaddl	" << rightval << ", %eax" << std::endl;
	return std::string("%eax");
}

antlrcpp::Any CodeGenVisitor::visitExpressionValue(ifccParser::ExpressionValueContext *ctx) 
{
	return visit(ctx->value()).as<std::string>();
}
