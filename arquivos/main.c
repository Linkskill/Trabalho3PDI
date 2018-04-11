#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pdi.h"

#define THRESHOLD   0.5f
#define ALTURA      31
#define LARGURA     31
#define SIGMA       10
#define GT2         "GT2.BMP"
#define MASCARAGT2  "11GT2mascara.bmp"
#define BORRADAGT2  "12GT2borrada.bmp"
#define BLOOMGT2    "13GT2bloom.bmp"
#define WW          "Wind Waker GC.bmp"
#define MASCARAWW   "21WWmascara.bmp"
#define BORRADAWW   "22WWborrada.bmp"
#define BLOOMWW     "23WWbloom.bmp"

//Funções novas
void mask(Imagem *in, Imagem *out, float threshold);
void boxbloom(Imagem *mascara, int altura, int largura, int vezes);
void gaussianbloom(Imagem *mascara, int sigma, int vezes);

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
    copiaConteudo(imagem, mascara);

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
    mask(imagem, mascara, THRESHOLD);
    salvaImagem(mascara, MASCARAGT2);

    //2 - Borra a máscara diversas vezes.
    gaussianbloom(mascara, SIGMA, 4);
    salvaImagem(mascara, BORRADAGT2);

    //3 - Soma o resultado da máscara borrada com a imagem original, causando o efeito bloom.
    printf("Somando imagens...");
    soma(imagem, mascara, 0.9, 0.1, imagem);
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
    copiaConteudo(imagem, mascara);

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
    mask(imagem, mascara, THRESHOLD);
    salvaImagem(mascara, MASCARAWW);    

    //2 - Borra a máscara diversas vezes.
    boxbloom(mascara, ALTURA, LARGURA, 4);        
    salvaImagem(mascara, BORRADAWW);    

    //3 - Soma o resultado da máscara borrada com a imagem original, causando o efeito bloom.
    printf("Somando imagens...");
    soma(imagem, mascara, 0.9, 0.1, imagem);
    salvaImagem(imagem, "Wind Waker GC2.bmp");
    salvaImagem(imagem, BLOOMWW);
    printf("\t\t\t\t[\x1b[32m OK \x1b[0m]\n");

    //Libera a memória previamente alocada.
    free(imagem);
    free(mascara);
    free(buffer);

    return 0;
}

//1 - Converte a imagem RGB para HSL.
//2 - Compara com o limiar global apenas a camada da luminância.
//3 - Zera os pixels onde a luminância for menor ou igual ao limiar.
void mask(Imagem *in, Imagem *out, float threshold) {

    printf("Criando máscara...");
    
    Imagem *temp = criaImagem(in->largura, in->altura, in->n_canais);

    RGBParaHSL(in, temp);

    for(int y = 0; y < temp->altura; y += 1) {
        for(int x = 0; x < temp->largura; x += 1) {
            if(temp->dados[2][y][x] <= threshold) {
                out->dados[0][y][x] = 0.0f;
                out->dados[1][y][x] = 0.0f;
                out->dados[2][y][x] = 0.0f;
            }
        }
    }

    free(temp);

    printf("\t\t\t\t[\x1b[32m OK \x1b[0m]\n");
}

//Borra a máscara com o box blur.
void boxbloom(Imagem *mascara, int altura, int largura, int vezes) {

    printf("Borrando máscara... 1 ");

    Imagem *temp = criaImagem(mascara->largura, mascara->altura, mascara->n_canais);

    blur(mascara, mascara, altura, largura, NULL);
    copiaConteudo(mascara, temp);

    for(int i = 1; i < vezes; i += 1) {
        printf("%d ", i + 1);
        blur(temp, temp, altura*2*i + 1, largura*2*i + 1, NULL);
        soma(mascara, temp, 1, 1, mascara);
    }

    free(temp);

    printf("\t\t\t[\x1b[32m OK \x1b[0m]\n");
}

//Borra a máscara com o filtro gaussiano.
void gaussianbloom(Imagem *mascara, int sigma, int vezes) {

    printf("Borrando máscara... 1 ");

    Imagem *temp = criaImagem(mascara->largura, mascara->altura, mascara->n_canais);

    filtroGaussiano(mascara, mascara, sigma, sigma, NULL);
    copiaConteudo(mascara, temp);

    for(int i = 1; i < vezes; i += 1) {
        printf("%d ", i + 1);
        filtroGaussiano(temp, temp, sigma*(i + 1), sigma*(i + 1), NULL);
        soma(mascara, temp, 1, 1, mascara);
    }

    free(temp);

    printf("\t\t\t[\x1b[32m OK \x1b[0m]\n");
}