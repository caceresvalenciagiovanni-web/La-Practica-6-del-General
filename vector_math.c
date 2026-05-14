#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hoc.h"

/* Se declara la función externa de manejo de errores para interrumpir operaciones inválidas. */
extern void execerror(char *s, char *t);

/* Se reserva memoria dinámica tanto para la estructura del vector como para su arreglo de componentes. */
Vector *creaVector(int n) {
    Vector *v = (Vector *)malloc(sizeof(Vector));
    if (v == NULL) execerror("Out of memory", "creaVector");
    
    v->n = n;
    v->vec = (double *)malloc(sizeof(double) * n);
    if (v->vec == NULL) execerror("Out of memory", "creaVector array");
    
    return v;
}

/* Se genera una copia exacta e independiente de un vector existente en un nuevo espacio de memoria. */
Vector *copiaVector(Vector *a) {
    Vector *c = creaVector(a->n);
    int i;
    for(i = 0; i < a->n; i++) {
        c->vec[i] = a->vec[i];
    }
    return c;
}

/* Se imprime el contenido del vector en la salida estándar. 
   Si el vector es de dimensión 1, se imprime como un escalar puro. */
void imprimeVector(Vector *a) {
    int i;
    if (a == NULL) return;
    
    if (a->n == 1) {
        printf("\t%.8g\n", a->vec[0]);
    } else {
        printf("\t[ ");
        for(i = 0; i < a->n; i++) {
            printf("%.8g ", a->vec[i]);
        }
        printf("]\n");
    }
}

/* Se realiza la suma aritmética con soporte para Broadcasting (Escalar + Vector). */
Vector *sumaVector(Vector *a, Vector *b) {
    Vector *c;
    int i;
    
    if (a->n == 1 && b->n > 1) {
        c = creaVector(b->n);
        for(i = 0; i < b->n; i++) c->vec[i] = a->vec[0] + b->vec[i];
        return c;
    }
    else if (a->n > 1 && b->n == 1) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] + b->vec[0];
        return c;
    }
    else if (a->n == b->n) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] + b->vec[i];
        return c;
    }
    else {
        execerror("Dimensiones incompatibles", "en suma");
        return NULL;
    }
}

/* Se realiza la resta aritmética con soporte para Broadcasting (Escalar - Vector). */
Vector *restaVector(Vector *a, Vector *b) {
    Vector *c;
    int i;
    
    if (a->n == 1 && b->n > 1) {
        c = creaVector(b->n);
        for(i = 0; i < b->n; i++) c->vec[i] = a->vec[0] - b->vec[i];
        return c;
    }
    else if (a->n > 1 && b->n == 1) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] - b->vec[0];
        return c;
    }
    else if (a->n == b->n) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] - b->vec[i];
        return c;
    }
    else {
        execerror("Dimensiones incompatibles", "en resta");
        return NULL;
    }
}

/* Se realiza el producto componente a componente o multiplicación por escalar (Broadcasting). */
Vector *multiVector(Vector *a, Vector *b) {
    Vector *c;
    int i;
    
    /* Caso 1: Escalar * Vector (ej. 5 * [1 2 3]) */
    if (a->n == 1 && b->n > 1) {
        c = creaVector(b->n);
        for(i = 0; i < b->n; i++) c->vec[i] = a->vec[0] * b->vec[i];
        return c;
    }
    /* Caso 2: Vector * Escalar (ej. [1 2 3] * 5) */
    else if (a->n > 1 && b->n == 1) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] * b->vec[0];
        return c;
    }
    /* Caso 3: Vector * Vector (componente a componente) */
    else if (a->n == b->n) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] * b->vec[i];
        return c;
    }
    /* Caso 4: Dimensiones totalmente incompatibles (ej. 2D * 3D) */
    else {
        execerror("Dimensiones incompatibles", "en multiplicacion");
        return NULL;
    }
}

/* Se realiza la division componente a componente o division por escalar (Broadcasting). */
Vector *diviVector(Vector *a, Vector *b) {
    Vector *c;
    int i;
    
    /* Caso 1: Escalar / Vector (ej. 5 / [1 2 3]) */
    if (a->n == 1 && b->n > 1) {
        c = creaVector(b->n);
        for(i = 0; i < b->n; i++) {
            if (b->vec[i] == 0.0) execerror("division por cero", "");
            c->vec[i] = a->vec[0] / b->vec[i];
        }
        return c;
    }
    /* Caso 2: Vector / Escalar (ej. [1 2 3] / 5) */
    else if (a->n > 1 && b->n == 1) {
        if (b->vec[0] == 0.0) execerror("division por cero", "");
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) c->vec[i] = a->vec[i] / b->vec[0];
        return c;
    }
    /* Caso 3: Vector / Vector (componente a componente) */
    else if (a->n == b->n) {
        c = creaVector(a->n);
        for(i = 0; i < a->n; i++) {
            if (b->vec[i] == 0.0) execerror("division por cero", "");
            c->vec[i] = a->vec[i] / b->vec[i];
        }
        return c;
    }
    /* Caso 4: Dimensiones totalmente incompatibles */
    else {
        execerror("Dimensiones incompatibles", "en division");
        return NULL;
    }
}

/* Se calcula el producto punto (escalar) y se encapsula el resultado en un vector unidimensional. */
Vector *productoPunto(Vector *a, Vector *b) {
    Vector *c;
    double suma = 0.0;
    int i;
    
    if (a->n != b->n) execerror("Dimensiones incompatibles", "en producto punto");
    for(i = 0; i < a->n; i++) suma += (a->vec[i] * b->vec[i]);
    
    c = creaVector(1);
    c->vec[0] = suma;
    return c;
}

/* Se calcula el producto cruz geométrico. Esta operación está estrictamente limitada a vectores tridimensionales. */
Vector *productoCruz(Vector *a, Vector *b) {
    Vector *c;
    if (a->n != 3 || b->n != 3) execerror("El producto cruz requiere vectores 3D", "");
    
    c = creaVector(3);
    c->vec[0] = a->vec[1] * b->vec[2] - a->vec[2] * b->vec[1];
    c->vec[1] = a->vec[2] * b->vec[0] - a->vec[0] * b->vec[2];
    c->vec[2] = a->vec[0] * b->vec[1] - a->vec[1] * b->vec[0];
    return c;
}

/* Se calcula la norma euclidiana (longitud o magnitud) de un vector. */
double magnitudVector(Vector *v) {
    double suma = 0.0;
    int i;
    for(i = 0; i < v->n; i++) suma += (v->vec[i] * v->vec[i]);
    return sqrt(suma);
}

/* ========================================================================= */
/* OPERACIONES RELACIONALES Y LÓGICAS                      */
/* ========================================================================= */

/* * MACRO GENERADORA: Escribe automáticamente la lógica de broadcasting para 
 * cualquier operador relacional o lógico, devolviendo 1.0 (Verdadero) o 
 * 0.0 (Falso) componente a componente.
 */
#define GENERA_LOGICA_VECTORIAL(nombre_funcion, operador) \
Vector *nombre_funcion(Vector *a, Vector *b) { \
    Vector *c; \
    int i; \
    if (a->n == 1 && b->n > 1) { \
        c = creaVector(b->n); \
        for(i = 0; i < b->n; i++) c->vec[i] = (double)(a->vec[0] operador b->vec[i]); \
        return c; \
    } \
    else if (a->n > 1 && b->n == 1) { \
        c = creaVector(a->n); \
        for(i = 0; i < a->n; i++) c->vec[i] = (double)(a->vec[i] operador b->vec[0]); \
        return c; \
    } \
    else if (a->n == b->n) { \
        c = creaVector(a->n); \
        for(i = 0; i < a->n; i++) c->vec[i] = (double)(a->vec[i] operador b->vec[i]); \
        return c; \
    } \
    else { \
        execerror("Dimensiones incompatibles", "en operacion logica"); \
        return NULL; \
    } \
}

/* Usamos la macro para generar las 8 funciones matemáticas automáticamente */
GENERA_LOGICA_VECTORIAL(mayorQueVector, >)
GENERA_LOGICA_VECTORIAL(menorQueVector, <)
GENERA_LOGICA_VECTORIAL(mayorIgualVector, >=)
GENERA_LOGICA_VECTORIAL(menorIgualVector, <=)
GENERA_LOGICA_VECTORIAL(igualVector, ==)
GENERA_LOGICA_VECTORIAL(diferenteVector, !=)
GENERA_LOGICA_VECTORIAL(andVector, &&)
GENERA_LOGICA_VECTORIAL(orVector, ||)

/* El operador NOT (!) solo recibe un vector, por lo que lo hacemos manual */
Vector *notVector(Vector *a) {
    Vector *c = creaVector(a->n);
    int i;
    for(i = 0; i < a->n; i++) {
        c->vec[i] = (double)(!a->vec[i]);
    }
    return c;
}

/* * REGLA ESTRICTA DE VERDAD (ALL):
 * Revisa un vector booleano para los ciclos 'while' e 'if'.
 * Retorna 1 (Verdadero) solo si TODOS los elementos son distintos de 0.
 * Retorna 0 (Falso) si encuentra al menos un 0.
 */
int esVerdadero(Vector *v) {
    int i;
    for(i = 0; i < v->n; i++) {
        if (v->vec[i] == 0.0) return 0; /* Falso inmediatamente */
    }
    return 1; /* Verdadero */
}
