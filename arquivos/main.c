#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pdi.h"

#define GT2         "GT2.BMP"
#define MASCARAGT2  "1GT2mascara.bmp"
#define BORRADAGT2  "2GT2borrada.bmp"
#define BLOOMGT2    "3GT2bloom.bmp"
#define WW          "Wind Waker GC.bmp"
#define MASCARAWW   "1WWmascara.bmp"
#define BORRADAWW   "2WWborrada.bmp"
#define BLOOMWW     "3WWbloom.bmp"

int main() {

    Imagem *imagem, *mascara, *buffer;

    //Inicialzia imagens necessárias.
    imagem = abreImagem(GT2, 3);

    printf("Carregando imagem [ %s ]...", GT2);
    if(!imagem) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }
    printf("\t[\x1b[32m OK \x1b[0m]\n");

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

    //1 - Cria a máscara, separando as regiões mais claras da imagem.
    printf("Criando máscara...");
    binariza(imagem, mascara, 0.75f);
    salvaImagem(mascara, MASCARAGT2);
    printf("\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //2 - Borra a máscara diversas vezes.
    printf("Borrando máscara... ");
    printf("1 ");
    filtroGaussiano(mascara, buffer, 10, 10, NULL);
    printf("2 ");
    filtroGaussiano(buffer, buffer, 20, 20, NULL);
    printf("3 ");
    filtroGaussiano(buffer, buffer, 40, 40, NULL);
    printf("4 ");
    filtroGaussiano(buffer, mascara, 80, 80, NULL);        
    salvaImagem(mascara, BORRADAGT2);
    printf("\t\t[\x1b[32m OK \x1b[0m]\n");

    //3 - Soma o resultado da máscara borrada com a imagem original, causando o efeito bloom.
    printf("Somando imagens...");
    soma(imagem, mascara, 1, 0.5, imagem);
    salvaImagem(imagem, BLOOMGT2);
    printf("\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //Libera a memória previamente alocada.
    free(imagem);
    free(mascara);
    free(buffer);

    //Inicialzia imagens necessárias.
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

    //1 - Cria a máscara, separando as regiões mais claras da imagem.
    binariza(imagem, mascara, 0.75f);
    salvaImagem(mascara, MASCARAWW);

    //2 - Borra a máscara diversas vezes.
    filtroGaussiano(mascara, buffer, 10, 10, NULL);
    filtroGaussiano(buffer, buffer, 20, 20, NULL);
    filtroGaussiano(buffer, buffer, 40, 40, NULL);
    filtroGaussiano(buffer, mascara, 80, 80, NULL);        
    salvaImagem(mascara, BORRADAWW);

    //3 - Soma o resultado da máscara borrada com a imagem original, causando o efeito bloom.
    soma(imagem, mascara, 1, 0.5, imagem);
    salvaImagem(imagem, BLOOMWW);

    //Libera a memória previamente alocada.
    free(imagem);
    free(mascara);
    free(buffer);

    return 0;
}
