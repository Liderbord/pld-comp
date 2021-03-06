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
  if (argn == 2)
  {
    ifstream lecture(argv[1]);
    in << lecture.rdbuf();
  }
  else
  {
    cerr << "usage: ifcc path/to/file.c" << endl;
    exit(1);
  }

  ANTLRInputStream input(in.str());

  ifccLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  tokens.fill();

  ifccParser parser(&tokens);
  tree::ParseTree *tree = parser.axiom();

  if (parser.getNumberOfSyntaxErrors() != 0)
  {
    cerr << "ERROR: syntax error during parsing" << endl;
    exit(1);
  }

  // visit the grammar tree and generate the assembly code
  CodeGenVisitor v;
  v.visit(tree);

  // check if there is a warning (unused variable)
  map<string, Variable>::iterator var;
  map<string, Function>::iterator fn;
  map<string, Function> functions = v.getFunctions();
  for (fn = functions.begin(); fn != functions.end(); fn++)
  {
    for (var = fn->second.vars.begin(); var != fn->second.vars.end(); var++)
    {
      if (!var->second.used)
      {
        cout << "# WARNING: variable " << var->first << " is not used" << endl;
      }
    }
  }

  // return 0 if there is no error, 1 otherwise
  return v.getError() ? 1 : 0;
}
