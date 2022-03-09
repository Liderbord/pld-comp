#include "CodeGenVisitor.h"

static const std::string START_MAC = ".globl	_main\n_main:\n";
static const std::string START_OTHERS = ".globl	main\nmain:\n";
static const std::string END = "\tret\n";

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
	std::cout << "prog" << std::endl;
	std::string body = __APPLE__ ? START_MAC : START_OTHERS;
	ifccParser::ContentContext * contentContext = ctx->content();
	if (contentContext) {
		std::cout << "->content" << std::endl;
		std::string content = visit(contentContext).as<std::string>();
		body += "\t" + content + "\n";
	}
	std::string returnValue = visit(ctx->returnValue()).as<std::string>();
	std::cout << body << "\t" << returnValue << "\n" << END;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx) 
{
	std::cout << "content" << std::endl;
	std::string init = visit(ctx->init()).as<std::string>();
	std::cout << init << std::endl;
	ifccParser::ContentContext * contentContext = ctx->content();
	if (!contentContext) {
		return init;
	} else {
		std::string content = visit(contentContext).as<std::string>();
		return init + content;
	}
}

antlrcpp::Any CodeGenVisitor::visitInit(ifccParser::InitContext *ctx) 
{
	std::cout << "init" << std::endl;
	std::string type = ctx->TYPE()->getText();
	std::cout << type << std::endl;
	std::string varname = ctx->VARNAME()->getText();
	std::string constval = ctx->CONST()->getText();
	int index = (this->vars.size() + 1) * 8;
	this->vars[varname] = index;
	return "movl $" + constval + ", -" + std::to_string(index) + "(%rbp)";
}

antlrcpp::Any CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx) 
{
	std::cout << "returnValue" << std::endl;
	std::string returnval;
	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	if (varnameNode) {
		std::string varname = varnameNode->getText();
		std::string index = std::to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return "movl " + returnval + ", %eax";
}
