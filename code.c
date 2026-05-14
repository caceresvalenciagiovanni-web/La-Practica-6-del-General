#include "hoc.h"
#include "y.tab.h"
#include <stdio.h>
#include <stdlib.h>

#define NSTACK 256
static Datum stack[NSTACK];  /* La pila de la máquina virtual */
static Datum *stackp;        /* Puntero al siguiente lugar libre en la pila */

#define NPROG 2000
Inst prog[NPROG];            /* Memoria RAM que almacena las instrucciones */
Inst *progp;                 /* Puntero al siguiente lugar libre para generación de código */
Inst *pc;                    /* Contador de programa durante la ejecución */

/* --- INICIALIZACIÓN Y CONTROL DE LA PILA --- */

void initcode() {      
    stackp = stack;
    progp = prog;
}

void push(Datum d) {   
    if (stackp >= &stack[NSTACK])
        execerror("Desbordamiento de la pila (stack overflow)", (char *) 0);
    *stackp++ = d;
}

Datum pop() {          
    if (stackp <= stack)
        execerror("Pila vacía (stack underflow)", (char *) 0);
    return *--stackp;
}

/* --- GESTIÓN DE MEMORIA --- */
void liberaVector(Vector *v) {
    if (v != NULL) {
        if (v->vec != NULL) free(v->vec);
        free(v);
    }
}

/* Sacar y liberar de la pila el elemento del tope (Silencioso, para limpiar) */
void pop1() {
    Datum d;
    d = pop();
    liberaVector(d.vec);
}

/* --- INSTRUCCIONES DE ACCESO A DATOS --- */

void constpush() {     
    Datum d;
    Symbol *s = (Symbol *)*pc++;
    d.vec = copiaVector(s->u.vec); 
    push(d);
}

void varpush() {       
    Datum d;
    d.sym = (Symbol *)(*pc++);
    push(d);
}

void eval() {          
    Datum d;
    d = pop();
    if (d.sym->type == INDEF)
        execerror("Variable no definida", d.sym->name); 
    d.vec = copiaVector(d.sym->u.vec); 
    push(d);
}

/* --- INSTRUCCIONES MATEMÁTICAS --- */

void add() { Datum d1, d2, d3; d2 = pop(); d1 = pop(); d3.vec = sumaVector(d1.vec, d2.vec); liberaVector(d1.vec); liberaVector(d2.vec); push(d3); }
void sub() { Datum d1, d2, d3; d2 = pop(); d1 = pop(); d3.vec = restaVector(d1.vec, d2.vec); liberaVector(d1.vec); liberaVector(d2.vec); push(d3); }
void mul() { Datum d1, d2, d3; d2 = pop(); d1 = pop(); d3.vec = multiVector(d1.vec, d2.vec); liberaVector(d1.vec); liberaVector(d2.vec); push(d3); }
void divop() { Datum d1, d2, d3; d2 = pop(); d1 = pop(); d3.vec = diviVector(d1.vec, d2.vec); liberaVector(d1.vec); liberaVector(d2.vec); push(d3); }
void dotop() { Datum d1, d2, d3; d2 = pop(); d1 = pop(); d3.vec = productoPunto(d1.vec, d2.vec); liberaVector(d1.vec); liberaVector(d2.vec); push(d3); }
void crossop() { Datum d1, d2, d3; d2 = pop(); d1 = pop(); d3.vec = productoCruz(d1.vec, d2.vec); liberaVector(d1.vec); liberaVector(d2.vec); push(d3); }

void magop() {         
    Datum d; double mag; Vector *res;
    d = pop(); 
    mag = magnitudVector(d.vec); 
    liberaVector(d.vec); 
    res = creaVector(1); res->vec[0] = mag; d.vec = res;
    push(d); 
}

void buildvec() {
    int n = (int)(long)*pc++; 
    Datum d; int i, j, idx; int total_n = 0;
    Vector **temp = (Vector **)malloc(n * sizeof(Vector *));
    if (temp == NULL) execerror("Sin memoria en", "buildvec");
    
    for (i = n - 1; i >= 0; i--) temp[i] = pop().vec;
    for (i = 0; i < n; i++) total_n += temp[i]->n;
    
    Vector *res = creaVector(total_n); idx = 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < temp[i]->n; j++) res->vec[idx++] = temp[i]->vec[j];
        liberaVector(temp[i]); 
    }
    free(temp); d.vec = res; push(d);    
}

void negate() {        
    Datum d; Vector *temp; int i;
    d = pop(); temp = creaVector(d.vec->n);
    for(i = 0; i < d.vec->n; i++) temp->vec[i] = -(d.vec->vec[i]);
    liberaVector(d.vec); d.vec = temp; push(d);
}

void power() { execerror("Operacion de potencia aun no soportada", (char *)0); }

/* --- INSTRUCCIONES LÓGICAS Y RELACIONALES (HOC5) --- */

/* Macro para ensamblar dinámicamente las instrucciones booleanas */
#define IMPLEMENTA_OP_LOGICA(nombre_instruccion, funcion_vectorial) \
void nombre_instruccion() { \
    Datum d1, d2, d3; \
    d2 = pop(); \
    d1 = pop(); \
    d3.vec = funcion_vectorial(d1.vec, d2.vec); \
    liberaVector(d1.vec); \
    liberaVector(d2.vec); \
    push(d3); \
}

IMPLEMENTA_OP_LOGICA(gt, mayorQueVector)
IMPLEMENTA_OP_LOGICA(lt, menorQueVector)
IMPLEMENTA_OP_LOGICA(ge, mayorIgualVector)
IMPLEMENTA_OP_LOGICA(le, menorIgualVector)
IMPLEMENTA_OP_LOGICA(eq, igualVector)
IMPLEMENTA_OP_LOGICA(ne, diferenteVector)
IMPLEMENTA_OP_LOGICA(and, andVector)
IMPLEMENTA_OP_LOGICA(or, orVector)

void not() {
    Datum d, res;
    d = pop();
    res.vec = notVector(d.vec);
    liberaVector(d.vec);
    push(res);
}

/* --- CONTROL DE FLUJO: IF Y WHILE CON GESTIÓN DE MEMORIA ESTRICTA --- */

void whilecode() {
    Datum d;
    Inst *savepc = pc;     /* savepc apunta al inicio del bloque del while */
    
    execute(savepc + 2);   /* Ejecuta la condición, la cual deja un vector en la pila */
    d = pop(); 
    
    while (esVerdadero(d.vec)) {
        liberaVector(d.vec);           /* ¡GARBAGE COLLECTION! Liberamos la condición antes de entrar al cuerpo */
        execute(*((Inst **)(savepc))); /* Ejecuta el cuerpo del ciclo */
        
        execute(savepc + 2);           /* Vuelve a evaluar la condición para la siguiente vuelta */
        d = pop(); 
    }
    liberaVector(d.vec);               /* Liberar el último vector de condición (cuando se volvió falsa) */
    
    pc = *((Inst **)(savepc + 1));     /* Salta al código que sigue después del while */
}

void forcode() {
    Datum d;
    Inst *savepc = pc; /* savepc apunta a nuestro bloque de 5 punteros */
    
    /* 1. Ejecutar Inicialización (ej. i = 0) */
    execute(*((Inst **)(savepc))); 
    pop1(); /* ¡VITAL! Limpiamos el resultado de la asignación para no saturar la pila */
    
    /* 2. Evaluar Condición (ej. i < 10) */
    execute(*((Inst **)(savepc + 1))); 
    d = pop();
    
    while (esVerdadero(d.vec)) {
        liberaVector(d.vec); /* Garbage collection de la condición */
        
        /* 3. Ejecutar Cuerpo del ciclo */
        execute(*((Inst **)(savepc + 3))); 
        
        /* 4. Ejecutar Incremento (ej. i = i + 1) */
        execute(*((Inst **)(savepc + 2))); 
        pop1(); /* ¡VITAL! Limpiamos el resultado del incremento de la pila */
        
        /* 5. Volver a evaluar Condición */
        execute(*((Inst **)(savepc + 1))); 
        d = pop();
    }
    liberaVector(d.vec); /* Liberar el último vector falso */
    
    /* 6. Terminar y saltar al exterior del ciclo */
    pc = *((Inst **)(savepc + 4)); 
}

void ifcode() {
    Datum d;
    Inst *savepc = pc; 
    
    execute(savepc + 3); /* Ejecuta la condición */
    d = pop(); 
    int es_verdad = esVerdadero(d.vec); /* Determinamos si todo el vector es verdadero */
    liberaVector(d.vec);                /* ¡GARBAGE COLLECTION INMEDIATO! */
    
    if (es_verdad) {
        execute(*((Inst **)(savepc)));  /* Ejecuta la parte THEN */
    } else if (*((Inst **)(savepc + 1))) {
        execute(*((Inst **)(savepc + 1))); /* Ejecuta la parte ELSE si existe */
    }
    pc = *((Inst **)(savepc + 2));      /* Salta al siguiente bloque de código exterior */
}

/* --- INSTRUCCIONES DE CONTROL Y FUNCIONES --- */

void assign() {        
    Datum d1, d2;
    d1 = pop(); /* Símbolo destino */
    d2 = pop(); /* Vector a asignar */
    
    if (d1.sym->type != VAR && d1.sym->type != INDEF) 
        execerror("Asignacion a no-variable", d1.sym->name);
        
    if (d1.sym->type == VAR) liberaVector(d1.sym->u.vec);
    
    d1.sym->u.vec = copiaVector(d2.vec);
    d1.sym->type = VAR;
    push(d2); 
} 

void print() {         
    Datum d;
    d = pop();
    imprimeVector(d.vec);
    liberaVector(d.vec);
}

void prexpr() {        /* Imprimir expresiones sin consumir salto de línea (usado en loops) */
    Datum d;
    d = pop();
    imprimeVector(d.vec);
    liberaVector(d.vec);
}

void bltin() {         
    Datum d; Vector *res;
    d = pop();
    res = (*(Vector *(*)(Vector *))(*pc++))(d.vec);
    liberaVector(d.vec);
    d.vec = res;
    push(d);
}

/* --- MOTOR DE EJECUCIÓN --- */

Inst *code(Inst f) {   
    Inst *oprogp = progp;
    if (progp >= &prog[NPROG])
        execerror("Programa demasiado grande", (char *) 0);
    *progp++ = f;
    return oprogp;
}

void execute(Inst *p) { 
    for (pc = p; *pc != STOP; ) 
        (*pc++)();
}
