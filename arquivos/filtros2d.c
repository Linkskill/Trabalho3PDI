/*============================================================================*/
/* FILTRAGEM ESPACIAL                                                         */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/
/** Tipos e fun��es para filtragem espacial. */
/*============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "base.h"
#include "filtros2d.h"

/*============================================================================*/
/* FILTRAGEM LINEAR GEN�RICA                                                  */
/*============================================================================*/
/** Filtragem espacial na horizontal/vertical, usando um vetor de coeficientes.
 * Use para aplicar filtros 1D ou uma componente de um filtro separ�vel.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             float* coef: vetor de coeficientes.
 *             int n: n�mero de valores no vetor de coeficientes.
 *             int vertical: se != 0, aplica o filtro na vertical.
 *
 * Valor de retorno: nenhum. */

void filtro1D (Imagem* in, Imagem* out, float* coef, int n, int vertical)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais)
    {
        printf ("ERRO: filtro1D: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    if ((!vertical && n >= in->largura*2) || (vertical && n >= in->altura*2))
    {
        printf ("ERRO: filtro1D: vetor de coeficientes grande demais!\n");
        exit (1);
    }

    int channel, row, col, i, pos;
    float soma;
    int centro = n/2;

    // Para cada centro poss�vel...
    for (channel = 0; channel < in->n_canais; channel++)
    {
        for (row = 0; row < in->altura; row++)
        {
            for (col = 0; col < in->largura; col++)
            {
                soma = 0;

                // Percorre o vetor de coeficientes.
                for (i = -centro; i <= centro; i++)
                {
                    // Tratamento de margens com imagem espelhada.
                    pos = ((vertical)? row : col) + i;

                    if (pos < 0)
                        pos = -pos;
                    else if (!vertical && pos >= in->largura)
                        pos = in->largura*2 - pos - 2;
                    else if (vertical && pos >= in->altura)
                        pos = in->altura*2 - pos - 2;

                    if (vertical)
                        soma += in->dados [channel][pos][col] * coef [centro + i];
                    else
                        soma += in->dados [channel][row][pos] * coef [centro + i];
                }

                out->dados [channel][row][col] = soma;
            }
        }
    }
}

/*============================================================================*/
/* FILTRO DA M�DIA                                                            */
/*============================================================================*/
/** Implementa��o de box blur usando uma imagem integral.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             int altura: altura da janela.
 *             int largura: largura da janela.
 *             Imagem* buffer: uma imagem com o mesmo tamanho da imagem de
 *               entrada. Pode ser usada quando se quer evitar a aloca��o do
 *               buffer interno. Use NULL se quiser usar o buffer interno.
 *
 * Valor de retorno: nenhum (usa a imagem de sa�da). */

void blur (Imagem* in, Imagem* out, int altura, int largura, Imagem* buffer)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais ||
        (buffer && (in->largura != buffer->largura || in->altura != buffer->altura || in->n_canais != buffer->n_canais)))
    {
        printf ("ERRO: blurIntegral: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    if (altura % 2 == 0 || largura % 2 == 0)
    {
        printf ("ERRO: blurIntegral: a janela deve ter largura e altura impares.\n");
        exit (1);
    }

    if (altura == 1 && largura == 1)
    {
        copiaConteudo (in, out);
        return;
    }

    // Primeiro calcula a imagem integral.
    Imagem* integral = (buffer)? buffer : criaImagem (in->largura, in->altura, in->n_canais);

    int channel, row, col;
    for (channel = 0; channel < in->n_canais; channel++)
    {
        // Soma em linhas.
        for (row = 0; row < in->altura; row++)
        {
            integral->dados [channel][row][0] = in->dados [channel][row][0];

            for (col = 1; col < in->largura; col++)
                integral->dados [channel][row][col] = in->dados [channel][row][col] + integral->dados [channel][row][col-1];
        }

        // Agora soma na vertical.
        for (row = 1; row < in->altura; row++)
            for (col = 0; col < in->largura; col++)
                integral->dados [channel][row][col] += integral->dados [channel][row-1][col];
    }

    // Agora calcula as m�dias.
    int top, left, bottom, right;
    float soma, area;
    for (channel = 0; channel < in->n_canais; channel++)
        for (row = 0; row < in->altura; row++)
            for (col = 0; col < in->largura; col++)
            {
                top = MAX (-1, row-altura/2-1);
                left = MAX (-1, col-largura/2-1);
                bottom = MIN (in->altura-1, row+altura/2);
                right = MIN (in->largura-1, col+largura/2);

                soma = ((top >= 0 && left >= 0)? integral->dados [channel][top][left] : 0) +
                        integral->dados [channel][bottom][right] -
                        ((left >= 0)? integral->dados [channel][bottom][left] : 0) -
                        ((top >= 0)? integral->dados [channel][top][right] : 0);

                area = (right-left)*(bottom-top);
                out->dados [channel][row][col] = soma/area;
            }

    if (!buffer)
        destroiImagem (integral);
}

/*============================================================================*/
/* FILTRO GAUSSIANO                                                           */
/*============================================================================*/
/** Implementa��o de filtro Gaussiano. Como a filtragem Gaussiana � separ�vel,
 * fazemos o processo primeiro na horizontal e depois na vertical. Os
 * coeficientes s�o calculados com base no sigma, que deve ser positivo,
 * exceto em 3 casos especiais: -3, -5 e -7. Nestes casos, o valor n�o �
 * interpretado como o sigma, e s�o usados os coeficientes cl�ssicos para a
 * aproxima��o 3x3 com sigma=0.8; 5x5 com sigma=1.1; e 7x7 com sigma=1.4,
 * respectivamente.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             float sigmax: desvio padr�o na horizontal. Deve ser positivo,
 *               ou um dos 3 valores especiais.
 *             float sigmay: desvio padr�o na vertical. Deve ser positivo,
 *               ou um dos 3 valores especiais.
 *             Imagem* buffer: uma imagem com o mesmo tamanho da imagem de
 *               entrada. Pode ser usada quando se quer evitar a aloca��o do
 *               buffer interno. Use NULL se quiser usar o buffer interno.
 *
 * Valor de retorno: nenhum (usa a imagem de sa�da). */

// Fun��o auxiliar usada para calcular os coeficientes horizontais ou verticais seguindo uma fun��o Gaussiana.
void _filtroGaussianoCalculaCoef (int largura, float sigma, float* coef)
{
    // Valores especiais.
    if (sigma == -3)
    {
        coef [0] = coef [2] = 0.25f;
        coef [1] = 0.5f;
		return;
    }

    if (sigma == -5)
    {
        coef [0] = coef [4] = 0.0625f;
        coef [1] = coef [3] = 0.25f;
        coef [2] = 0.375f;
		return;
    }

    if (sigma == -7)
    {
        coef [0] = coef [6] = 0.03125f;
        coef [1] = coef [5] = 0.109375f;
        coef [2] = coef [4] = 0.21875f;
        coef [3] = 0.28125f;
		return;
    }

    int i;
    float x, computado, total;

    int centro = largura/2;

    // Calcula.
    coef [centro] = total = 1;
    for (i = 1; i <= centro; i++)
    {
        x = i;
        computado = exp (-(x*x)/(2*sigma*sigma));
        coef [centro-i] = computado;
        coef [centro+i] = computado;
        total += computado*2;
    }

    // Normaliza.
    total = 1.0f/total;
    for (i = 0; i < largura; i++)
        coef [i] *= total;
}

// Micro-fun��o que retorna o n�mero de coeficientes para um dado sigma.
int _filtroGaussianoNCoef (float sigma)
{
    // Primeiro, os valores especiais.
    if (sigma == -3)
        return (3);

    if (sigma == -5)
        return (5);

    if (sigma == -7)
        return (7);

    int n = (int) (sigma*4.0f + 0.5f);
    n |= 1; // Precisa ser �mpar.
    return (n);
}

void filtroGaussiano (Imagem* in, Imagem* out, float sigmax, float sigmay, Imagem* buffer)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais ||
        (buffer && (in->largura != buffer->largura || in->altura != buffer->altura || in->n_canais != buffer->n_canais)))
    {
        printf ("ERRO: filtroGaussiano: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    Imagem* img_aux = (buffer)? buffer : criaImagem (in->largura, in->altura, in->n_canais);

    // Calcula o tamanho da matriz de coeficientes.
    int largura = _filtroGaussianoNCoef (sigmax);
    int altura = _filtroGaussianoNCoef (sigmay);

    float* coef = malloc (sizeof (float) * MAX (largura, altura));

    // Filtra na horizontal.
    _filtroGaussianoCalculaCoef (largura, sigmax, coef);
    filtro1D (in, img_aux, coef, largura, 0);

    // Agora na vertical.
    if (sigmax != sigmay)
        _filtroGaussianoCalculaCoef (altura, sigmay, coef);
    filtro1D (img_aux, out, coef, altura, 1);

    free (coef);
    if (!buffer)
        destroiImagem (img_aux);
}

/*============================================================================*/
/* UNSHARP MASKING                                                            */
/*============================================================================*/
/** Realce de bordas usando unsharp masking. Borra a imagem, verifica a
 * diferen�a da original para a borrada, e soma a diferen�a em lugares onde
 * ela for grande.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             float sigma: par�metro para a suaviza��o Gaussiana.
 *             float threshold: altera apenas regi�es onde a diferen�a � grande.
 *             float mult: multiplica as diferen�as por este valor. Valores
 *               mais altos implicam em bordas mais destacadas.
 *             Imagem* buffer: uma imagem com o mesmo tamanho da imagem de
 *               entrada. Pode ser usada quando se quer evitar a aloca��o do
 *               buffer interno. Use NULL se quiser usar o buffer interno.
 *
 * Valor de retorno: nenhum. */

void unsharpMasking (Imagem* in, Imagem* out, float sigma, float threshold, float mult, Imagem* buffer)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais ||
        (buffer && (in->largura != buffer->largura || in->altura != buffer->altura || in->n_canais != buffer->n_canais)))
    {
        printf ("ERRO: unsharpMasking: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    // Come�a borrando a imagem...
    filtroGaussiano (in, out, sigma, sigma, buffer);

    // Verifica a diferen�a da imagem original para a borrada.
    soma (in, out, 1, -1, out);

    // Agora percorre a imagem e procura lugares com diferen�a grande. Real�a a imagem nestes locais.
    int channel, row, col;
    for (channel = 0; channel < in->n_canais; channel++)
        for (row = 0; row < in->altura; row++)
            for (col = 0; col < in->largura; col++)
                if (out->dados [channel][row][col] > threshold)
                    out->dados [channel][row][col] = in->dados [channel][row][col] + mult*out->dados [channel][row][col];
                else
                    out->dados [channel][row][col] = in->dados [channel][row][col];
}

/*============================================================================*/
/* FILTRO DA MEDIANA                                                          */
/*============================================================================*/
/** Implementa��o cl�ssica do filtro da mediana aproximado. Mant�m um
 * histograma que � atualizado quando a janela desliza. Usamos um histograma
 * de 256 faixas, supondo que os pixels est�o no intervalo [0,1]. Isso nos d�
 * resultados exatos para entradas e sa�das com 8bpp. Este n�o � o algoritmo
 * mais r�pido poss�vel, mas � simples de implementar.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             int altura: altura da janela.
 *             int largura: largura da janela.
 *
 * Valor de retorno: nenhum (usa a imagem de sa�da). */

// Fun��o auxiliar, chamada pela filtroMediana8bpp para obter a mediana a partir de um histograma de 256 faixas.
int _medianaHistograma8bpp (int* hist, int total)
{
    int i, soma = 0;

    total /= 2; // Para quando encontrar a metade dos valores.

    for (i = 0; i < 256; i++)
    {
        soma += hist [i];
        if (soma >= total)
            return (i);
    }

    return (255);
}

void filtroMediana8bpp (Imagem* in, Imagem* out, int altura, int largura)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais)
    {
        printf ("ERRO: filtroMediana8bpp: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    if (altura % 2 == 0 || largura % 2 == 0)
    {
        printf ("ERRO: filtroMediana8bpp: a janela deve ter largura e altura impares.\n");
        exit (1);
    }

    int channel, row, col, i, j;
    int w = largura/2;
    int h = altura/2;
    int saiu = -1, entrou = -1; // Para verificar se o histograma mudou.

    // Este � o histograma. Para trabalhar nele, teremos que reconverter a imagem para 8bpp.
    int histograma [256];
    int n_histograma = 0;

    unsigned char** in8bpp;
    in8bpp = malloc (sizeof (unsigned char*) * in->altura);
    for (i = 0; i < in->altura; i++)
        in8bpp [i] = malloc (sizeof (unsigned char) * in->largura);

    // Para cada canal...
    for (channel = 0; channel < in->n_canais; channel++)
    {
        // Converte para 8bpp.
        for (row = 0; row < in->altura; row++)
            for (col = 0; col < in->largura; col++)
                in8bpp [row][col] = float2uchar (in->dados [channel][row][col]);

        // Para cada linha...
        for (row = 0; row < in->altura; row++)
        {
            // Inicializa o histograma.
            for (i = 0; i < 256; i++)
                histograma [i] = 0;

            // Obt�m o primeiro histograma para esta linha.
            for (i = MAX (0, row-h); i <= MIN (in->altura-1, row+h); i++)
                for (j = 0; j <= w; j++)
                    histograma [in8bpp [i][j]]++;
            n_histograma = (row+h-MAX(0,row-h)+1)*(w+1); // N�mero de valores na primeira janela.

            // Obt�m a mediana para o primeiro pixel.
            out->dados [channel][row][0] = _medianaHistograma8bpp (histograma, n_histograma) / 255.0f;

            // Agora vai para as colunas seguintes.
            for (col = 1; col < in->largura; col++)
            {
                 // Remove a coluna que sai, adiciona a que entra.
                for (i = MAX (0, row-h); i <= MIN (in->altura-1, row+h); i++)
                {
                    j = col-w-1;
                    if (j >= 0)
                    {
                        saiu = in8bpp [i][j];
                        histograma [saiu]--;
                        n_histograma--;
                    }
                    else
                        saiu = -1;

                    j = col+w;
                    if (j < in->largura)
                    {
                        entrou = in8bpp [i][j];
                        histograma [entrou]++;
                        n_histograma++;
                    }
                    else
                        entrou = -1;
                }

                // Obt�m a mediana para esta posi��o.
                if (saiu == entrou)
                    out->dados [channel][row][col] = out->dados [channel][row][col-1];
                else
                    out->dados [channel][row][col] = _medianaHistograma8bpp (histograma, n_histograma) / 255.0f;
            }
        }
    }

    for (i = 0; i < in->altura; i++)
        free (in8bpp [i]);
    free (in8bpp);
}

/*============================================================================*/
/* M�XIMOS E M�NIMOS LOCAIS                                                   */
/*============================================================================*/
/** Localiza o m�ximo local em uma vizinhan�a da imagem. Consideramos que o
 * m�ximo local � um filtro espacial n�o-linear e separ�vel.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             int altura: altura da janela.
 *             int largura: largura da janela.
 *             Imagem* buffer: uma imagem com o mesmo tamanho da imagem de
 *               entrada. Pode ser usada quando se quer evitar a aloca��o do
 *               buffer interno. Use NULL se quiser usar o buffer interno.
 *
 * Valor de retorno: nenhum (usa a imagem de sa�da). */

int _maxLocalMaxLinha (Imagem* in, int channel, int row, int inicio, int fim) // Fun��o auxiliar para a maxLocal.
{
    int col;

    if (inicio < 0) inicio = 0;
    if (fim >= in->largura) fim = in->largura-1;

    int pos_max = inicio;
    for (col = inicio+1; col <= fim; col++)
        if (in->dados [channel][row][col] >= in->dados [channel][row][pos_max])
            pos_max = col;

    return (pos_max);
}

float _maxLocalMaxColuna (Imagem* in, int channel, int col, int inicio, int fim) // Fun��o auxiliar para a maxLocal.
{
    int row;

    if (inicio < 0) inicio = 0;
    if (fim >= in->altura) fim = in->altura-1;

    int pos_max = inicio;
    for (row = inicio+1; row <= fim; row++)
        if (in->dados [channel][row][col] >= in->dados [channel][pos_max][col])
            pos_max = row;

    return (pos_max);
}

void maxLocal (Imagem* in, Imagem* out, int altura, int largura, Imagem* buffer)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais ||
        (buffer && (in->largura != buffer->largura || in->altura != buffer->altura || in->n_canais != buffer->n_canais)))
    {
        printf ("ERRO: maxLocal: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    if (altura % 2 == 0 || largura % 2 == 0)
    {
        printf ("ERRO: maxLocal: a janela deve ter largura e altura impares.\n");
        exit (1);
    }

    Imagem* img_aux = (buffer)? buffer : criaImagem (in->largura, in->altura, in->n_canais);

    int channel, row, col, pos_max;
    int w = largura/2; // largura = 2w+1
    int h = altura/2; // algtura = 2h+1
    for (channel = 0; channel < in->n_canais; channel++)
    {
        // Primeiro na horizontal.
        for (row = 0; row < in->altura; row++)
        {
            // Acha o maior valor dentro da primeira janela desta linha.
            pos_max = _maxLocalMaxLinha (in, channel, row, 0, w);
            img_aux->dados [channel][row][0] = in->dados [channel][row][pos_max];

            for (col = 1; col < in->largura; col++)
            {
                // Remove a coluna que sai, adiciona a que entra.
                if (pos_max == col-w-1)
                    pos_max = _maxLocalMaxLinha (in, channel, row, col-w, col+w);
                else if (col+w < in->largura && in->dados [channel][row][col+w] >= in->dados [channel][row][pos_max])
                    pos_max = col+w;

                img_aux->dados [channel][row][col] = in->dados [channel][row][pos_max];
            }
        }

        // Agora na vertical.
        for (col = 0; col < in->largura; col++)
        {
             // Acha o maior valor dentro da primeira janela desta coluna.
             pos_max = _maxLocalMaxColuna (img_aux, channel, col, 0, h);
             out->dados [channel][0][col] = img_aux->dados [channel][pos_max][col];

             for (row = 1; row < in->altura; row++)
             {
                if (pos_max == row-h-1)
                    pos_max = _maxLocalMaxColuna (img_aux, channel, col, row-h, row+h);
                else if (row+h < in->altura && img_aux->dados [channel][row+w][col] >= img_aux->dados [channel][pos_max][col])
                    pos_max = row+w;

                out->dados [channel][row][col] = img_aux->dados [channel][pos_max][col];
             }
        }
    }

    if (!buffer)
        destroiImagem (img_aux);
}

/*----------------------------------------------------------------------------*/
/** Localiza o m�nimo local em uma vizinhan�a da imagem. Consideramos que o
 * m�nimo local � um filtro espacial n�o-linear e separ�vel.
 *
 * Par�metros: Imagem* in: imagem de entrada. Se tiver mais que 1 canal,
 *               processa cada canal independentemente.
 *             Imagem* out: imagem de sa�da. Deve ter o mesmo tamanho da
 *               imagem de entrada.
 *             int altura: altura da janela.
 *             int largura: largura da janela.
 *             Imagem* buffer: uma imagem com o mesmo tamanho da imagem de
 *               entrada. Pode ser usada quando se quer evitar a aloca��o do
 *               buffer interno. Use NULL se quiser usar o buffer interno.
 *
 * Valor de retorno: nenhum (usa a imagem de sa�da). */

int _minLocalMinLinha (Imagem* in, int channel, int row, int inicio, int fim) // Fun��o auxiliar para a minLocal.
{
    int col;

    if (inicio < 0) inicio = 0;
    if (fim >= in->largura) fim = in->largura-1;

    int pos_min = inicio;
    for (col = inicio+1; col <= fim; col++)
        if (in->dados [channel][row][col] <= in->dados [channel][row][pos_min])
            pos_min = col;

    return (pos_min);
}

float _minLocalMinColuna (Imagem* in, int channel, int col, int inicio, int fim) // Fun��o auxiliar para a minLocal.
{
    int row;

    if (inicio < 0) inicio = 0;
    if (fim >= in->altura) fim = in->altura-1;

    int pos_min = inicio;
    for (row = inicio+1; row <= fim; row++)
        if (in->dados [channel][row][col] <= in->dados [channel][pos_min][col])
            pos_min = row;

    return (pos_min);
}

void minLocal (Imagem* in, Imagem* out, int altura, int largura, Imagem* buffer)
{
    if (in->largura != out->largura || in->altura != out->altura || in->n_canais != out->n_canais ||
        (buffer && (in->largura != buffer->largura || in->altura != buffer->altura || in->n_canais != buffer->n_canais)))
    {
        printf ("ERRO: minLocal: as imagens precisam ter o mesmo tamanho e numero de canais.\n");
        exit (1);
    }

    if (altura % 2 == 0 || largura % 2 == 0)
    {
        printf ("ERRO: minLocal: a janela deve ter largura e altura impares.\n");
        exit (1);
    }

    Imagem* img_aux = (buffer)? buffer : criaImagem (in->largura, in->altura, in->n_canais);

    int channel, row, col, pos_min;
    int w = largura/2; // largura = 2w+1
    int h = altura/2; // algtura = 2h+1
    for (channel = 0; channel < in->n_canais; channel++)
    {
        // Primeiro na horizontal.
        for (row = 0; row < in->altura; row++)
        {
            // Acha o menor valor dentro da primeira janela desta linha.
            pos_min = _minLocalMinLinha (in, channel, row, 0, w);
            img_aux->dados [channel][row][0] = in->dados [channel][row][pos_min];

            for (col = 1; col < in->largura; col++)
            {
                // Remove a coluna que sai, adiciona a que entra.
                if (pos_min == col-w-1)
                    pos_min = _minLocalMinLinha (in, channel, row, col-w, col+w);
                else if (col+w < in->largura && in->dados [channel][row][col+w] <= in->dados [channel][row][pos_min])
                    pos_min = col+w;

                img_aux->dados [channel][row][col] = in->dados [channel][row][pos_min];
            }
        }

        // Agora na vertical.
        for (col = 0; col < in->largura; col++)
        {
             // Acha o maior valor dentro da primeira janela desta coluna.
             pos_min = _minLocalMinColuna (img_aux, channel, col, 0, h);
             out->dados [channel][0][col] = img_aux->dados [channel][pos_min][col];

             for (row = 1; row < in->altura; row++)
             {
                if (pos_min == row-h-1)
                    pos_min = _minLocalMinColuna (img_aux, channel, col, row-h, row+h);
                else if (row+h < in->altura && img_aux->dados [channel][row+w][col] <= img_aux->dados [channel][pos_min][col])
                    pos_min = row+w;

                out->dados [channel][row][col] = img_aux->dados [channel][pos_min][col];
             }
        }
    }

    if (!buffer)
        destroiImagem (img_aux);
}

/*============================================================================*/
