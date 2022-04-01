#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "generated/ifccBaseVisitor.h"

#include "CodeGenVisitor.h"

using namespace antlr4;
using namespace std;

int main(int argn, const char **argv)
{
  stringstream in;
  if (argn==2)
  {
     ifstream lecture(argv[1]);
     in << lecture.rdbuf();
  }
  else
  {
      cerr << "usage: ifcc path/to/file.c" << endl ;
      exit(1);
  }
  
  ANTLRInputStream input(in.str());

  ifccLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  tokens.fill();

  ifccParser parser(&tokens);
  tree::ParseTree* tree = parser.axiom();

  if(parser.getNumberOfSyntaxErrors() != 0)
  {
      cerr << "error: syntax error during parsing" << endl;
      exit(1);
  }

  
  CodeGenVisitor v;
  v.setError(false);
  v.visit(tree);
  
  map <string,int> :: iterator iterMAP;
  for(iterMAP= v.mapWarnings.begin(); iterMAP != v.mapWarnings.end(); iterMAP++)
  {
    if(iterMAP->second == 0){
      cout << "#WARNING: variable " << iterMAP->first << " is not used." << endl;
    }
  }
  if(v.getError()==true){
    return 1;
  }else{
    return 0;
  }

  

  
}
