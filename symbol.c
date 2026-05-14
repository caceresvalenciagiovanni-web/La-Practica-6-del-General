#include "hoc.h" 
#include "y.tab.h"   /* Aseguramos usar el archivo generado por Yacc */
#include <string.h>  /* Para usar strcmp, strlen, strcpy */
#include <stdlib.h>  /* Para usar malloc */

static Symbol *symlist = 0;    /* tabla de simbolos: lista ligada */

/* encontrar s en la tabla de símbolos */
Symbol *lookup(char *s)    
{
    Symbol *sp;
    /* Recorremos la lista ligada buscando una coincidencia */
    for (sp = symlist; sp != (Symbol *)0; sp = sp->next) {
        if (strcmp(sp->name, s) == 0) {
            return sp;
        }
    }
    return 0;      /* 0 ==> no se encontró */ 
}

/* instalar s en la tabla de símbolos, ahora acepta un Vector */
Symbol *install(char *s, int t, Vector *v) 
{
    Symbol *sp;
    char *emalloc();

    /* Reservamos memoria para el nuevo símbolo */
    sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s) + 1) ; /* +1 para '\0' */
    strcpy(sp->name, s);
    
    /* Asignamos el tipo (VAR, BLTIN, etc.) */
    sp->type = t;
    
    /* CAMBIO CLAVE: Guardamos el puntero al vector en la unión u */
    sp->u.vec = v;
    
    /* Ponemos el nuevo símbolo al frente de la lista ligada */
    sp->next = symlist;
    symlist = sp;
    
    return sp;
}

/* revisar el regreso desde malloc para evitar errores de memoria */
char *emalloc(unsigned n)	
{
    char *p;
    p = malloc(n);
    if(p == 0) {
        execerror("out of memory", (char *) 0);
    }
    return p;
}
