#include "CodeGenVisitor.h"
#include <utility>
#include <vector>
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

	ifccParser::ContentContext *contentContext = ctx->content();
	if (contentContext)
	{
		cout << "\t# content" << endl;
		visit(contentContext);
	}
	string value = visit(ctx->value()).as<string>();
	cout << "\t"
		 << "movl " << value << ", %eax" << endl
		 << END;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx)
{
	visitChildren(ctx);
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
		int index = (this->vars.size() + 1) * 8;
		// if varname already exists in vars, then it's an error
		if (vars.find(varname) == vars.end())
		{
			this->vars[varname] = index;
			// look for the value and cout ASSEMBLY code
			if (paire.second != "")
			{
				string value = paire.second;
				cout << "\tmovl " + value + ", " << EAX << endl;
				cout << "\tmovl " + EAX + ", -" + to_string(index) + "(%rbp)" << endl;

			}
		}
		else
		{
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

antlrcpp::Any CodeGenVisitor::visitValue(ifccParser::ValueContext *ctx)
{
	string returnval;
	antlr4::tree::TerminalNode *varnameNode = ctx->VARNAME();
	if (varnameNode)
	{
		string varname = varnameNode->getText();
		string index = to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	}
	else
	{
		returnval = "$" + ctx->CONST()->getText();
	}
	return returnval;
}

string CodeGenVisitor::getNewTempVariable()
{
	int index = (this->vars.size() + 1) * 8;
	string indexString = to_string(index);
	string varname = "temp" + indexString;
	this->vars[varname] = index;
	return "-" + indexString + "(%rbp)";
}

antlrcpp::Any CodeGenVisitor::visitExpressionPar(ifccParser::ExpressionParContext *ctx)
{
	return visit(ctx->expression()).as<string>();
}

string CodeGenVisitor::operationExpression(string leftval, string rightval, string operation)
{
	string regval = getNewTempVariable();
	cout << "\tmovl " << leftval << ", " << EAX << endl;
	cout << "\t" << operation << " " << rightval << ", " << EAX << endl;
	cout << "\tmovl " << EAX << ", " << regval << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionMult(ifccParser::ExpressionMultContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " * " << rightval << endl;
	return operationExpression(leftval, rightval, "imull");
}

antlrcpp::Any CodeGenVisitor::visitExpressionDiv(ifccParser::ExpressionDivContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	string regval = getNewTempVariable();
	cout << "\tmovl " << leftval << ", " << EAX << endl;
	cout << "\tcltd" << endl;
	cout << "\tmovl\t" << rightval << ", " << ECX << endl;
	cout << "\tidivl\t" << ECX << endl;
	cout << "\tmovl\t" << EAX << ", " << regval << "\n"
		 << endl;
	return regval;
}

antlrcpp::Any CodeGenVisitor::visitExpressionAdd(ifccParser::ExpressionAddContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " + " << rightval << endl;
	return operationExpression(leftval, rightval, "add ");
}

antlrcpp::Any CodeGenVisitor::visitExpressionSub(ifccParser::ExpressionSubContext *ctx)
{
	string leftval = visit(ctx->expression(0)).as<string>();
	string rightval = visit(ctx->expression(1)).as<string>();
	cout << "\t# do " << leftval << " - " << rightval << endl;
	return operationExpression(leftval, rightval, "sub ");
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

string CodeGenVisitor::operationCompExpression(string leftval, string rightval, string comp)
{
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

bool CodeGenVisitor::getWarning()
{
	return this->warning;
}
bool CodeGenVisitor::getError()
{
	return this->error;
}

void CodeGenVisitor::setWarning(bool val)
{
	this->warning = val;
}

void CodeGenVisitor::setError(bool val)
{
	this->error = val;
}
