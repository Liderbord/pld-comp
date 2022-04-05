
Feature 1: Vérification qu’une variable utilisée dans une expression a été déclarée.

  Si la variable n'a pas été déclarée, la méthode  visitAffectExpr retournera un cout d'error: "# ERROR: variable ... is not declared"
  
Feature 2: Vérification qu’une variable déclarée est utilisée au moins une fois.

  Si la variable est utilisée, on met le nom de la variable en paramètre de la méthode setVarUsed (qui se trouve dans visitAffectExpr)  qui sauvegarde toutes les variables utilisées.
  Ainsi lorsque l'on regardera si une variable est utilisée, si son nom n'est pas dans les variables utilisées alors un warning sera activé.
  
Feature 3: Vérification qu’une variable n’est pas déclarée plusieurs fois

  Lorsque l'on déclare une nouvelle variable, on inscrit son nom dans une map. De plus on vérifie que la variable n'existe pas déjà dans cette map à l'aide d'un if.
  Cela permet ainsi de nous empêcher de déclarer plusieurs fois une variable.
  Si jamais une variable est déclarée plusieurs fois, alors on a un code d'erreur: "# ERROR: variable ... is not declared"
