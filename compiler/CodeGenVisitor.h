#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
using namespace std;

struct Element
{
	bool var{false};
	string type;
	int value;
	Element(int value, string type, bool var) : value(value), type(type), var(var) {}
	string getValue()
	{
		return var ? "-" + to_string(value) + "(%rbp)" : "$" + to_string(value);
	}
};

ostream &
operator<<(ostream &os, Element &element)
{
	return os << element.getValue();
};

struct Variable
{
	int index;
	string type;
	bool used{false};
};

struct Function
{
	string type;
	map<string, Variable> vars;
};

class CodeGenVisitor : public ifccBaseVisitor
{
public:
	Element getNewTempVariable();
	Element operationExpression(Element rightval, Element leftval, string operation);
	Element operationCompExpression(Element rightval, Element leftval, string comp);
	string getRegister(string);
	string getMove(string);
	virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
	virtual antlrcpp::Any visitContent(ifccParser::ContentContext *ctx) override;
	virtual antlrcpp::Any visitInit(ifccParser::InitContext *ctx) override;
	virtual antlrcpp::Any visitValue(ifccParser::ValueContext *ctx) override;
	virtual antlrcpp::Any visitReturnValue(ifccParser::ReturnValueContext *ctx) override;
	virtual antlrcpp::Any visitExpressionMultDivMod(ifccParser::ExpressionMultDivModContext *ctx) override;
	virtual antlrcpp::Any visitExpressionAddSub(ifccParser::ExpressionAddSubContext *ctx) override;
	virtual antlrcpp::Any visitExpressionAnd(ifccParser::ExpressionAndContext *ctx) override;
	virtual antlrcpp::Any visitExpressionOr(ifccParser::ExpressionOrContext *ctx) override;
	virtual antlrcpp::Any visitExpressionXor(ifccParser::ExpressionXorContext *ctx) override;
	virtual antlrcpp::Any visitExpressionEqual(ifccParser::ExpressionEqualContext *ctx) override;
	virtual antlrcpp::Any visitExpressionNotEqual(ifccParser::ExpressionNotEqualContext *ctx) override;
	virtual antlrcpp::Any visitExpressionGreater(ifccParser::ExpressionGreaterContext *ctx) override;
	virtual antlrcpp::Any visitExpressionLess(ifccParser::ExpressionLessContext *ctx) override;
	virtual antlrcpp::Any visitExpressionGreaterEqual(ifccParser::ExpressionGreaterEqualContext *ctx) override;
	virtual antlrcpp::Any visitExpressionLessEqual(ifccParser::ExpressionLessEqualContext *ctx) override;
	virtual antlrcpp::Any visitExpressionValue(ifccParser::ExpressionValueContext *ctx) override;
	virtual antlrcpp::Any visitExpressionFn(ifccParser::ExpressionFnContext *ctx) override;
	virtual antlrcpp::Any visitExpressionPar(ifccParser::ExpressionParContext *ctx) override;
	virtual antlrcpp::Any visitFnCall(ifccParser::FnCallContext *ctx) override;
	virtual antlrcpp::Any visitIfElse(ifccParser::IfElseContext *ctx) override;
	virtual antlrcpp::Any visitWhileDo(ifccParser::WhileDoContext *ctx) override;
	virtual antlrcpp::Any visitFn(ifccParser::FnContext *ctx) override;
	virtual antlrcpp::Any visitArgs(ifccParser::ArgsContext *ctx) override;
	virtual antlrcpp::Any visitArgsDef(ifccParser::ArgsDefContext *ctx) override;
	virtual antlrcpp::Any visitDeclaration(ifccParser::DeclarationContext *ctx) override;
	virtual antlrcpp::Any visitDec(ifccParser::DecContext *ctx) override;
	virtual antlrcpp::Any visitAffectationExpr(ifccParser::AffectationExprContext *ctx) override;
	map<std::string, Function> functions;
	int jumps;
	void setError();
	bool getError();
	void setWarning(bool val);
	bool getWarning();
	void setCurrentFunction(string name, string type);
	string getCurrentFunctionType();
	map<string, Variable> getVars();
	void setVar(string varname, int index, string type);
	void setVarUsed(string varname);
	Variable getVar(string varname);
	bool isVarDeclarated(string varname);

private:
	bool error{false};
	std::string currentFunction;
};