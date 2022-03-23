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
		string getNewTempVariable();
		string operationExpression(string rightval, string leftval, string operation);
		string operationCompExpression(string rightval, string leftval, string comp);
		virtual antlrcpp::Any visitExpressionMult(ifccParser::ExpressionMultContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionDiv(ifccParser::ExpressionDivContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionAdd(ifccParser::ExpressionAddContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionSub(ifccParser::ExpressionSubContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionAnd(ifccParser::ExpressionAndContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionOr(ifccParser::ExpressionOrContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionXor(ifccParser::ExpressionXorContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionEqual(ifccParser::ExpressionEqualContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionNotEqual(ifccParser::ExpressionNotEqualContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionGreater(ifccParser::ExpressionGreaterContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionLess(ifccParser::ExpressionLessContext *ctx) override ;
		virtual antlrcpp::Any visitExpressionValue(ifccParser::ExpressionValueContext *ctx) override ;
		virtual antlrcpp::Any visitIfElse(ifccParser::IfElseContext *ctx) override ;
		virtual antlrcpp::Any visitWhileDo(ifccParser::WhileDoContext *ctx) override ;


    std::map<std::string, int> vars;
		int jumps;
		void setError(bool val);
		bool getError();
		void setWarning(bool val);
		bool getWarning();
		private:
			bool warning;
			bool error;
	};





