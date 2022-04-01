#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
using namespace std;

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
	string getNewTempVariable();
	string operationExpression(string rightval, string leftval, string operation);
	string operationCompExpression(string rightval, string leftval, string comp);
	string getRegister(string);
	string getMove(string);
	virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
	virtual antlrcpp::Any visitContent(ifccParser::ContentContext *ctx) override;
	virtual antlrcpp::Any visitInit(ifccParser::InitContext *ctx) override;
	virtual antlrcpp::Any visitValue(ifccParser::ValueContext *ctx) override;
	virtual antlrcpp::Any visitReturnValue(ifccParser::ReturnValueContext *ctx) override;
	virtual antlrcpp::Any visitExpressionMultDiv(ifccParser::ExpressionMultDivContext *ctx) override;
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
	map<string, Variable> getVars();
	void setVar(string varname, int index, string type);
	void setVarUsed(string varname);
	Variable getVar(string varname);
	bool isVarDeclarated(string varname);

private:
	bool error{false};
	std::string currentFunction;
};