# Compiler features

These features are related to the code the developer is writing but rather how the compiler can compile better code, or assist the developer in finding the errors in their code.

## Verification of the declaration of an used variable.

If a variable is not declared, the visitAffectExpr method returns the cout error message : "# ERROR: variable ... is not declared"
  
## Verification that a declared variable is used at least once.

If the variable is used, we put the variables name in the parameters of setVarUsed method (which is in visitAffectExpr), which saves all the used variables.
Thereby, when we look if a variable is used, if its name isn't in the used variables, a warning is activated.
  
## Verification that a variable isn't declared several times.

When we declare a new variable, we write its name in a map. Moreover we verify that the variable is not already existing in the map thanks to an if.
This allow us to avoid multiple declaration of a variable.
If a variable is declared more than once, we get this error message : "# ERROR: variable ... is not declared"

## Optimisation des constantes
Les opérations de constantes sont optimisées. Exemple: `3+2` sera transformée en constante `5` au lieu de faire une opération dans l'assembleur.
