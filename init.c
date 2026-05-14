#include "hoc.h"
#include "y.tab.h" 
#include <stdio.h> 

/* Importamos los envoltorios vectoriales */
extern Vector *Sin(), *Cos(), *Atan(), *Log(), *Log10(), *Exp(), *Sqrt(), *Integer(), *Abs();

/* Arreglo de constantes matemáticas */
static struct {
    char *name;
    double cval;
} consts[] = {
    "PI",    3.14159265358979323846,
    "E",     2.71828182845904523536,
    "GAMMA", 0.57721566490153286060,  
    "DEG",   57.29577951308232087680, 
    "PHI",   1.61803398874989484820,  
    0,       0
};

/* Diccionario de funciones integradas (built-ins) */
static struct {
    char *name;
    Vector *(*func)(Vector *); 
} builtins[] = {
    "sin",   Sin,
    "cos",   Cos,
    "atan",  Atan,
    "log",   Log,
    "log10", Log10,
    "exp",   Exp,
    "sqrt",  Sqrt,
    "int",   Integer,
    "abs",   Abs,
    0,       0
};

/* --- NUEVO: Diccionario de Palabras Reservadas (Keywords) --- */
static struct {
    char *name;
    int kval; /* Guardará el tipo de token (WHILE, IF, etc.) */
} keywords[] = {
    "if",    IF,
    "else",  ELSE,
    "while", WHILE,
    "for",   FOR,
    "print", PRINT,
    0,       0
};

/* Función principal de inicialización */
void init() {
    int i;
    Symbol *s;
    Vector *v;

    /* 1. Instalación de Constantes */
    for (i = 0; consts[i].name; i++) {
        v = creaVector(1);
        v->vec[0] = consts[i].cval;
        install(consts[i].name, VAR, v);
    }
    
    /* 2. Instalación de Funciones Predefinidas */
    for (i = 0; builtins[i].name; i++) {
        s = install(builtins[i].name, BLTIN, NULL);
        s->u.ptr = builtins[i].func;
    }

    /* 3. --- NUEVO: Instalación de Palabras Reservadas --- */
    for (i = 0; keywords[i].name; i++) {
        /* Se instalan sin ningún vector (NULL) ya que solo son etiquetas de control */
        install(keywords[i].name, keywords[i].kval, NULL);
    }
}
