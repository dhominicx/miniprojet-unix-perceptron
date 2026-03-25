#include "config.h"
#include "reseau.h"

/* MATH */
void softmax(const double in[], double out[], int nb) {
    double max_softmax = in[0];
    for (int l = 1; l < nb; l++) {
        if (in[l] > max_softmax) max_softmax = in[l];
    }
    double somme = 0.0;
    for (int l = 0; l < nb; l++) {
        out[l] = exp(in[l] - max_softmax);
        somme += out[l];
    }
    for (int l = 0; l < nb; l++) {
        out[l] /= somme;
    }
}

double act_tanh(double x) { return tanh(x); }