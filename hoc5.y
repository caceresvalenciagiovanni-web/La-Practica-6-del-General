%{
#include "hoc.h"
#include <math.h>

#define code2(c1, c2)     code(c1); code(c2);
#define code3(c1, c2, c3) code(c1); code(c2); code(c3);

void yyerror(char *s);
int yylex();
void warning(char *s, char *t);
void execerror(char *s, char *t);
void fpecatch(int e);

extern void init();
extern void initcode();
extern void execute(Inst *p);
extern Inst *code(Inst f);
extern Vector *creaVector(int n);
%}

%union {
    Symbol *sym;
    Inst *inst;
    long narg; /* Usado para contar elementos de un vector */
}

/* Tokens combinados: Variables, Funciones y Control de Flujo */
%token <sym> NUMBER VAR BLTIN INDEF WHILE IF ELSE FOR PRINT
%type <inst> stmt asgn expr stmtlist cond while if end for forexpr
%type <narg> vector_elements

/* Precedencia de operaciones (De menor a mayor prioridad) */
%right '='
%left OR
%left AND
%left GT GE LT LE EQ NE
%left '+' '-'
%left '*' '/' '@' 'X'
%left UNARYMINUS NOT
%right '^'

%% /* REGLAS GRAMATICALES */

list:   
        | list '\n'
        | list asgn '\n'  { code2((Inst)pop1, STOP); return 1; }
        | list stmt '\n'  { code(STOP); return 1; }
        | list expr '\n'  { code2(print, STOP); return 1; }
        | list error '\n' { yyerrok; }
    ;

asgn:   VAR '=' expr { $$ = $3; code3(varpush, (Inst)$1, assign); }
    ;

/* Bloques de sentencias para ciclos y condiciones */
stmt:     expr                { code(pop1); }
        | PRINT expr          { code(prexpr); $$ = $2; }
        | while cond stmt end {
            ($1)[1] = (Inst)$3; /* Cuerpo del bucle */
            ($1)[2] = (Inst)$4; /* Salto para salir del bucle */
        }
        | for '(' forexpr ';' forexpr ';' forexpr ')' stmt end {
            ($1)[1] = (Inst)$3;   /* Puntero 0: Inicialización */
            ($1)[2] = (Inst)$5;   /* Puntero 1: Condición */
            ($1)[3] = (Inst)$7;   /* Puntero 2: Incremento */
            ($1)[4] = (Inst)$9;   /* Puntero 3: Cuerpo del bucle */
            ($1)[5] = (Inst)$10;  /* Puntero 4: Dirección de salida */
        }
        | if cond stmt end {    
            ($1)[1] = (Inst)$3; /* Parte THEN */
            ($1)[3] = (Inst)$4; /* Salto si es falso */
        }
        | if cond stmt end ELSE stmt end {  
            ($1)[1] = (Inst)$3; /* Parte THEN */
            ($1)[2] = (Inst)$6; /* Parte ELSE */
            ($1)[3] = (Inst)$7; /* Salto de salida */
        }
        | '{' stmtlist '}'    { $$ = $2; }
    ;

cond:   '(' expr ')'          { code(STOP); $$ = $2; }
    ;

forexpr:  expr { code(STOP); $$ = $1; }
    |     asgn { code(STOP); $$ = $1; }
    ;

for:    FOR { 
            $$ = code(forcode); 
            code(STOP); code(STOP); code(STOP); code(STOP); code(STOP); 
        }
    ;

while:  WHILE                 { $$ = code3(whilecode, STOP, STOP); }
    ;

if:     IF                    { $$ = code(ifcode); code3(STOP, STOP, STOP); }
    ;

end:                          { code(STOP); $$ = progp; }
    ;

stmtlist:                     { $$ = progp; }
        | stmtlist '\n'
        | stmtlist stmt
    ;

/* Expresiones (Aritmética, Vectores y Lógica) */
expr:     NUMBER              { $$ = code2(constpush, (Inst)$1); }
        | VAR                 { $$ = code3(varpush, (Inst)$1, eval); }
        | asgn
        | BLTIN '(' expr ')'  { $$ = $3; code2(bltin, (Inst)$1->u.ptr); }
        | '(' expr ')'        { $$ = $2; }
        | '|' expr '|'        { $$ = $2; code(magop); }
        | '[' vector_elements ']' { $$ = code2(buildvec, (Inst)$2); }
        | expr '+' expr       { code(add); }
        | expr '-' expr       { code(sub); }
        | expr '*' expr       { code(mul); }
        | expr '/' expr       { code(divop); }
        | expr '@' expr       { code(dotop); }
        | expr 'X' expr       { code(crossop); }
        | expr '^' expr       { code(power); }
        | '-' expr %prec UNARYMINUS { $$ = $2; code(negate); }
        | expr GT expr        { code(gt); }
        | expr GE expr        { code(ge); }
        | expr LT expr        { code(lt); }
        | expr LE expr        { code(le); }
        | expr EQ expr        { code(eq); }
        | expr NE expr        { code(ne); }
        | expr AND expr       { code(and); }
        | expr OR expr        { code(or); }
        | NOT expr            { $$ = $2; code(not); }
    ;

vector_elements:
          expr { $$ = 1; }
        | vector_elements ',' expr { $$ = $1 + 1; }
        ;
%%

/* =======================================================================
   ANALIZADOR LÉXICO Y FUNCIÓN MAIN
   ======================================================================= */
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

jmp_buf begin;
char *progname;
int lineno = 1;

extern void init();
extern void initcode();
extern void execute(Inst *p);
int follow(int expect, int ifyes, int ifno);

int main(int argc, char **argv) {
    progname = argv[0];
    printf("--------------------------------------------------------------------------------------------------\n");
    printf("                                        CALCULADORA VECTORIAL HOC5\n");
    printf("--------------------------------------------------------------------------------------------------\n");
    printf("Suma(+) Resta(-) Magnitud(| |) Prod.Cruz(X) Prod.Punto(@) Escalar(*)\n");
    printf("Uso de if, for y while: if (cond) { ... } else { ... } | while (cond) { ... } | for (cond) { ... }\n");
    printf("Escribe un vector, por ejemplo: variable = [PI, E, sin(PI/2)]\n");
    printf("--------------------------------------------------------------------------------------------------\n");

    init();
    setjmp(begin);
    signal(SIGFPE, fpecatch);
    for(initcode(); yyparse(); initcode())
        execute(prog);
    return 0;
}

int yylex() {
    int c;
    while ((c = getchar()) == ' ' || c == '\t');
    if (c == EOF) return 0;

    /* Extraer números y empaquetarlos en vectores 1D */
    if (c == '.' || isdigit(c)) {
        double d;
        Vector *v;
        ungetc(c, stdin);
        scanf("%lf", &d);
        
        v = creaVector(1);
        v->vec[0] = d;
        yylval.sym = install("", NUMBER, v);
        return NUMBER;
    }

    /* Extraer variables, funciones o palabras reservadas */
    if (isalpha(c)) {
        Symbol *s;
        char sbuf[200], *p = sbuf;
        do {
            if (p >= sbuf + sizeof(sbuf) - 1) {
                *p = '\0';
                execerror("nombre muy largo", sbuf);
            }
            *p++ = c;
        } while ((c = getchar()) != EOF && isalnum(c));
        ungetc(c, stdin);
        *p = '\0';
        
        /* Excepción del operador cruz */
        if (strcmp(sbuf, "X") == 0) return 'X';

        if ((s = lookup(sbuf)) == 0)
            s = install(sbuf, INDEF, NULL);
        yylval.sym = s;
        return s->type == INDEF ? VAR : s->type;
    }

    /* Lectura de operadores lógicos de dos caracteres (==, <=, ||) */
    switch (c) {
        case '>': return follow('=', GE, GT);
        case '<': return follow('=', LE, LT);
        case '=': return follow('=', EQ, '=');
        case '!': return follow('=', NE, NOT);
        case '|': return follow('|', OR, '|');
        case '&': return follow('&', AND, '&');
        case '\n': lineno++; return '\n';
        default: return c;
    }
}

/* Función para emparejar operadores dobles (ej. revisa si a '=' le sigue otro '=') */
int follow(int expect, int ifyes, int ifno) {
    int c = getchar();
    if (c == expect) return ifyes;
    ungetc(c, stdin);
    return ifno;
}

void yyerror(char *s) {
    warning(s, (char *)0);
}

void execerror(char *s, char *t) {
    warning(s, t);
    longjmp(begin, 0);
}

void fpecatch(int e) {
    execerror("excepcion de punto flotante", (char *)0);
}

void warning(char *s, char *t) {
    fprintf(stderr, "%s: %s", progname, s);
    if(t) fprintf(stderr, " %s", t);
    fprintf(stderr, " cerca de la linea %d\n", lineno);
}
