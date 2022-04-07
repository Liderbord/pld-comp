#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
using namespace std;

/**
 * @brief Element is used to store the information about the variables and constants (int/char).
 * 			  It's goal is to pass information between the different expressions, so they can optimize
 * 				the compilation by executing operations between constants and checking the types of the
 * 				variables used to avoid wrong results.
 *
 */

struct Element
{
	bool var{false}; // if the variable is a variable or not
	string type;		 // int/char
	int value;			 // register index (RBP) if it's a variable or a constant value ($N) if it's a constant
	Element(int value, string type, bool var) : value(value), type(type), var(var) {}
	string getValue(); // returns the value of the element on assembly code
};

/**
 * @brief Variable is used to store the information of a variable (int/char).
 * 				It stores the register index (RBP), the type (int/char) and if the variable was used or not.
 *
 */

struct Variable
{
	int index;				// register index (RBP)
	string type;			// int/char
	bool used{false}; // if the variable was used or not
};

/**
 * @brief Function is used to store the information of a function (int/char/void).
 * 				It stores the type (int/char/void) and the variables used in the function (stack)
 * 				using a mapping between the name of the variable and it's information (Variable).
 *
 */

struct Function
{
	int maxOffset = 0;
	string type;								// int/char/void
	map<string, Variable> vars; // stack (variable name -> variable information)
};

class CodeGenVisitor : public ifccBaseVisitor
{
public:
	// Functions descriptions on implementation
	// Grammar visitors
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
	virtual antlrcpp::Any visitExpressionNegation(ifccParser::ExpressionNegationContext *ctx) override;
	virtual antlrcpp::Any visitExpressionNegative(ifccParser::ExpressionNegativeContext *ctx) override;
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
	virtual antlrcpp::Any visitArrayDeclaration(ifccParser::ArrayDeclarationContext *ctx) override;
	virtual antlrcpp::Any visitAffectationArray(ifccParser::AffectationArrayContext *ctx) override;
	// Helpers
	Element getNewTempVariable();
	Element operationExpression(Element rightval, Element leftval, string operation);
	Element operationCompExpression(Element rightval, Element leftval, string comp);
	string getRegister(string);
	string getMove(string);
	void setCurrentFunction(string name, string type);
	string getCurrentFunctionType();
	int getMaxOffset();
	void setMaxOffset(int value);
	map<string, Function> getFunctions();
	map<string, Variable> getVars();
	vector<string> tabOfArrays;
	void setVar(string varname, int index, string type);
	Variable getVar(string varname);
	void setVarUsed(string varname);
	bool isVarDeclarated(string varname);
	string getJumpLabel();
	// Errors and warning
	void setError();
	bool getError();
	void setWarning(bool val);
	bool getWarning();
	string content;		 // temporal string that holds the assembly code of a function's body
	stringstream fout; // stream to add assembly code to the content of the function

private:
	int jumps;											 // counter of jumps on assembly code (used to generate unique labels)
	bool error{false};							 // if there was an error during the compilation, used to return 1 or 0 (compilation)
	string currentFunction;					 // name of the current function, used to access the current stack (next field)
	map<string, Function> functions; // mapping of functions names and their information (type, variables (stack))
};