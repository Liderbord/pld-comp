#pragma once


#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
using namespace std;


class  CodeGenVisitor : public ifccBaseVisitor {
	public:
		virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
		virtual antlrcpp::Any visitContent(ifccParser::ContentContext *ctx) override ;
		virtual antlrcpp::Any visitInit(ifccParser::InitContext *ctx) override ;
		virtual antlrcpp::Any visitValue(ifccParser::ValueContext *ctx) override ;
		string operationExpression(string rightval, string leftval, string operation) ;
		virtual antlrcpp::Any visitExpressionMult(ifccParser::ExpressionMultContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionDiv(ifccParser::ExpressionDivContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionAdd(ifccParser::ExpressionAddContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionSub(ifccParser::ExpressionSubContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionValue(ifccParser::ExpressionValueContext *ctx) override ;

    std::map<std::string, int> vars;
	void setError(bool val);
	bool getError();
	void setWarning(bool val);
	bool getWarning();
	private:
		bool warning;
		bool error;
};





