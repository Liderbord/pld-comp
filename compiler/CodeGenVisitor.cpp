#include "CodeGenVisitor.h"
using namespace std;


static const string START_MAC = ".globl	_main\n_main:\n";
static const std::string START_OTHERS = ".globl	main\nmain:\n";
static const std::string END = "\tret\n";



antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{



	cout << "#prog" << std::endl;
	string body;
#ifdef __APPLE__
	body = START_MAC;
#else
	body = START_OTHERS;
#endif
		ifccParser::ContentContext *contentContext = ctx->content();



	if (contentContext) {
		cout << "#->content" << endl;
		string content = visit(contentContext).as<string>();

		body += "\t" + content + "\n";
	}
	string returnValue = visit(ctx->returnValue()).as<string>();
	cout << body << "\t" << returnValue << "\n" << END;
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitContent(ifccParser::ContentContext *ctx) 
{

	cout << "#content" << endl;
	string init = visit(ctx->init()).as<string>();
	cout<<"#";
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

	cout << "#init" << endl;
	string type = ctx->TYPE()->getText();
	cout<< "#";
	cout << type << endl;
	string varname = ctx->VARNAME()->getText();
	string constval = ctx->CONST()->getText();

	int index = (this->vars.size() + 1) * 8;
	if( vars.find(varname)==vars.end()){
		this->vars[varname]=index;
		return "movl $" + constval + ", -" + to_string(index) + "(%rbp)";
	}else{
		error=true;
		return NULL;
	}
	
	
}

antlrcpp::Any CodeGenVisitor::visitReturnValue(ifccParser::ReturnValueContext *ctx) 
{

	cout << "#returnValue" << endl;
	string returnval;

	antlr4::tree::TerminalNode * varnameNode = ctx->VARNAME();
	if (varnameNode) {
		string varname = varnameNode->getText();
		string index = to_string(this->vars[varname]);
		returnval = "-" + index + "(%rbp)";
	} else {
		returnval = "$" + ctx->CONST()->getText();
	}
	return "movl " + returnval + ", %eax";
}



bool CodeGenVisitor::getWarning(){
	return this->warning;
}
bool CodeGenVisitor::getError(){
	return this->error;
}

void CodeGenVisitor::setWarning(bool val){
	this->warning=val;
}

void CodeGenVisitor::setError(bool val){
	this->error=val;
}