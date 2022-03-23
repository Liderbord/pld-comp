//ce test ne marche pas car, lors de la lecture de b=a, on n'a pas encore reconnu la variable a; 
// car on fait remonter toute la ligne quand on croise un ";"
// et ce pour prendre en compte le type INT commun à toutes les variables 


int main() {
    int a=1, b=a;
    return b;
}
