/* Se define la estructura Vector para que la máquina virtual la reconozca */
struct vector {
    int n;
    double *vec;
};
typedef struct vector Vector;

/* Entrada de la tabla de símbolos adaptada a vectores */
typedef struct Symbol { 
	char   *name;
	short   type;   /* VAR, BLTIN, UNDEF */
	union {
		Vector *vec;	         /* si es VAR, almacena el vector */
		Vector *(*ptr)(Vector*); /* sí es BLTIN, usa función vectorial */
	} u;
	struct Symbol   *next;       /* para ligarse a otro */ 
} Symbol;

/* Modificamos install para recibir un puntero a Vector */
Symbol *install(char *s, int t, Vector *v), *lookup(char *s);

/* Tipo de la pila del intérprete (La pila ahora apilará Vectores) */
typedef union Datum {   
    Vector  *vec;  /* Antes: double val; */
    Symbol  *sym; 
} Datum;

extern Datum pop();
typedef void (*Inst)();  /* instrucción de máquina */ 

#define STOP    (Inst) 0
extern Inst prog[], *progp;
extern void execute(Inst *p);

/* Instrucciones de máquina que actualizaremos más adelante */
extern void eval(), add(), sub(), mul(), divop(), negate(), power();
extern void assign(), bltin(), varpush(), constpush(), print();
extern void buildvec(), dotop(), crossop(), magop();
/* NUEVAS INSTRUCCIONES HOC5: Lógicas, ciclos y control */
extern void gt(), lt(), ge(), le(), eq(), ne(), and(), or(), not();
extern void ifcode(), whilecode(), pop1(), prexpr(), forcode();
extern void execerror(char *s, char *t);
extern Inst *code(Inst f);

/* Prototipos de vector_math.c (el motor matemático) */
extern Vector *creaVector(int n); 
extern Vector *copiaVector(Vector *a);
extern void imprimeVector(Vector *a); 
extern Vector *sumaVector(Vector *a, Vector *b); 
extern Vector *restaVector(Vector *a, Vector *b); 
extern Vector *multiVector(Vector *a, Vector *b); 
extern Vector *diviVector(Vector *a, Vector *b);
extern Vector *productoPunto(Vector *a, Vector *b);
extern Vector *productoCruz(Vector *a, Vector *b);
extern double magnitudVector(Vector *v);
/* Prototipos de operaciones lógicas y relacionales */
extern Vector *mayorQueVector(Vector *a, Vector *b);
extern Vector *menorQueVector(Vector *a, Vector *b);
extern Vector *mayorIgualVector(Vector *a, Vector *b);
extern Vector *menorIgualVector(Vector *a, Vector *b);
extern Vector *igualVector(Vector *a, Vector *b);
extern Vector *diferenteVector(Vector *a, Vector *b);
extern Vector *andVector(Vector *a, Vector *b);
extern Vector *orVector(Vector *a, Vector *b);
extern Vector *notVector(Vector *a);
extern int esVerdadero(Vector *v);
