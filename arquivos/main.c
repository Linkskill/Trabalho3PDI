#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pdi.h"

#define GT2         "GT2.BMP"
#define MASCARAGT2  "11GT2mascara.bmp"
#define BORRADAGT2  "12GT2borrada.bmp"
#define BLOOMGT2    "13GT2bloom.bmp"
#define WW          "Wind Waker GC.bmp"
#define MASCARAWW   "21WWmascara.bmp"
#define BORRADAWW   "22WWborrada.bmp"
#define BLOOMWW     "23WWbloom.bmp"

int main() {
    //Variáveis necessárias.
    Imagem *imagem, *mascara, *buffer;
    //Desativa o buffer para forçar o printf.
    setbuf(stdout, NULL);

    //Inicialzia imagens necessárias.
    imagem = abreImagem(GT2, 3);

    printf("Carregando imagem [ %s ]...", GT2);
    if(!imagem) {
        printf("\n\x1b[31mERRO:\x1b[0m A imagem não pode ser aberta.\n");
        exit(1);
    }
    printf("\t\t[\x1b[32m OK \x1b[0m]\n");

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
    printf("\t\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //2 - Borra a máscara diversas vezes.
    printf("Borrando máscara... ");
    printf("1 ");
    filtroGaussiano(mascara, buffer, 10, 10, NULL);
    printf("2 ");
    filtroGaussiano(buffer, buffer,  20, 20, NULL);
    printf("3 ");
    filtroGaussiano(buffer, buffer,  40, 40, NULL);
    printf("4 ");
    filtroGaussiano(buffer, mascara, 80, 80, NULL);        
    salvaImagem(mascara, BORRADAGT2);
    printf("\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //3 - Soma o resultado da máscara borrada com a imagem original, causando o efeito bloom.
    printf("Somando imagens...");
    soma(imagem, mascara, 1, 0.5, imagem);
    salvaImagem(imagem, BLOOMGT2);
    salvaImagem(imagem, "GT22.bmp");
    printf("\t\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //Libera a memória previamente alocada.
    free(imagem);
    free(mascara);
    free(buffer);

    //Inicialzia imagens necessárias.
    imagem = abreImagem(WW, 3);

    printf("Carregando imagem [ %s ]...", WW);
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
    salvaImagem(mascara, MASCARAWW);
    printf("\t\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //2 - Borra a máscara diversas vezes.
    printf("Borrando máscara... ");
    printf("1 ");
    blur(mascara, buffer, 31,  31, NULL);
    printf("2 ");
    blur(buffer, buffer,  61,  61, NULL);
    printf("3 ");
    blur(buffer, buffer,  121, 121, NULL);
    printf("4 ");
    blur(buffer, mascara, 241, 241, NULL);        
    salvaImagem(mascara, BORRADAWW);
    printf("\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //3 - Soma o resultado da máscara borrada com a imagem original, causando o efeito bloom.
    printf("Somando imagens...");
    soma(imagem, mascara, 1, 0.5, imagem);
    salvaImagem(imagem, "Wind Waker GC2.bmp");
    salvaImagem(imagem, BLOOMWW);
    printf("\t\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //Libera a memória previamente alocada.
    free(imagem);
    free(mascara);
    free(buffer);

    return 0;
}
