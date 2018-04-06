/*============================================================================*/
/* FILTRAGEM ESPACIAL                                                         */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/
/** Tipos e funções para filtragem espacial. */	
/*============================================================================*/

#ifndef __FILTROS2D_H
#define __FILTROS2D_H

/*============================================================================*/

#include "imagem.h"

/*============================================================================*/

void blur (Imagem* in, Imagem* out, int altura, int largura, Imagem* buffer_blur);
void filtroGaussiano (Imagem* in, Imagem* out, float sigmax, float sigmay, Imagem* buffer);
void filtro1D (Imagem* in, Imagem* out, float* coef, int n, int vertical);

void unsharpMasking (Imagem* in, Imagem* out, float sigma, float threshold, float mult, Imagem* buffer);

void filtroMediana8bpp (Imagem* in, Imagem* out, int altura, int largura);
void maxLocal (Imagem* in, Imagem* out, int altura, int largura, Imagem* buffer);
void minLocal (Imagem* in, Imagem* out, int altura, int largura, Imagem* buffer);

/*============================================================================*/
#endif /* __FILTROS2D_H */
