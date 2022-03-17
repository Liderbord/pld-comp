#pragma once


#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"


class  CodeGenVisitor : public ifccBaseVisitor {
	public:
		virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
		virtual antlrcpp::Any visitContent(ifccParser::ContentContext *ctx) override ;
		virtual antlrcpp::Any visitInit(ifccParser::InitContext *ctx) override ;
		virtual antlrcpp::Any visitReturnValue(ifccParser::ReturnValueContext *ctx) override ;
		//virtual antlrcpp::Any visitTestVar(ifccParser::ReturnValueContext *ctx) override ;
    std::map<std::string, int> vars;
	void setError(bool val);
	bool getError();
	void setWarning(bool val);
	bool getWarning();
	private:
		bool warning;
		bool error;
};





