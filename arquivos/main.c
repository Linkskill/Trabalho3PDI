#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pdi.h"

#define GT2         "GT2.BMP"
#define WW          "Wind Waker GC.bmp"
#define MASCARAGT2  "GT2mascara.bmp"
#define MASCARAWW   "WWmascara.bmp"

int main() {
    Imagem *imagem, *mascara, *buffer;

    imagem = abreImagem(GT2, 3);

    if(!imagem) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }

    mascara = criaImagem(imagem->largura, imagem->altura, imagem->n_canais);

    if(!mascara) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }

    buffer = criaImagem(imagem->largura, imagem->altura, imagem->n_canais);

    if(!buffer) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }

    binariza(imagem, mascara, 0.725f);
    blur(mascara, buffer, 31, 31, NULL);
    unsharpMasking(buffer, mascara, 1, 0.01f, 2, NULL);
    soma(imagem, mascara, 1, 0.5, mascara);

    salvaImagem(mascara, MASCARAGT2);

    destroiImagem(imagem);
    destroiImagem(mascara);
    destroiImagem(buffer);

    imagem = abreImagem(WW, 3);

    if(!imagem) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }

    mascara = criaImagem(imagem->largura, imagem->altura, imagem->n_canais);

    if(!mascara) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }

    buffer = criaImagem(imagem->largura, imagem->altura, imagem->n_canais);

    if(!buffer) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }

    binariza(imagem, mascara, 0.725f);
    blur(mascara, buffer, 31, 31, NULL);
    unsharpMasking(buffer, mascara, 1, 0.01f, 2, NULL);
    soma(imagem, mascara, 1, 0.5, mascara);

    salvaImagem(mascara, MASCARAWW);

    destroiImagem(imagem);
    destroiImagem(mascara);
    destroiImagem(buffer);

    return 0;
}
