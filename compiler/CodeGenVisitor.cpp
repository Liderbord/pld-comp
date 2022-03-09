#include "CodeGenVisitor.h"

static const std::string START_MAC = ".globl	_main\n_main:";
static const std::string START_OTHERS = ".globl	main\nmain:";
static const std::string END = "\tret\n";

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
	std::string body = __APPLE__ ? START_MAC : START_OTHERS;
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
	std::string init = visit(ctx->init()).as<std::string>();
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
