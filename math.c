#include <math.h>
#include <errno.h>
#include "hoc.h"

extern int errno;
void execerror(char *s, char *t);

/* Se evalúan los errores de dominio y rango de la librería matemática de C. */
double errcheck(double d, char *s) {
    if (errno == EDOM) {
        errno = 0;
        execerror(s, "argument out of domain");
    } else if (errno == ERANGE) {
        errno = 0;
        execerror(s, "result out of range");
    }
    return d;
}

/* Envoltorio genérico de broadcasting: Se aplica la función trigonométrica a cada componente del vector. */
Vector *Sin(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(sin(x->vec[i]), "sin");
    return res;
}

Vector *Cos(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(cos(x->vec[i]), "cos");
    return res;
}

Vector *Atan(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(atan(x->vec[i]), "atan");
    return res;
}

Vector *Log(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(log(x->vec[i]), "log");
    return res;
}

Vector *Log10(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(log10(x->vec[i]), "log10");
    return res;
}

Vector *Exp(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(exp(x->vec[i]), "exp");
    return res;
}

Vector *Sqrt(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = errcheck(sqrt(x->vec[i]), "sqrt");
    return res;
}

Vector *Integer(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = (double)(long)(x->vec[i]);
    return res;
}

Vector *Abs(Vector *x) {
    int i;
    Vector *res = creaVector(x->n);
    for(i = 0; i < x->n; i++) res->vec[i] = fabs(x->vec[i]);
    return res;
}
