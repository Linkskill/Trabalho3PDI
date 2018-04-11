all:
	gcc -o programa main.c base.c cores.c desenho.c filtros2d.c geometria.c imagem.c segmenta.c -lm

run:
	gcc -o programa main.c base.c cores.c desenho.c filtros2d.c geometria.c imagem.c segmenta.c -lm
	clear
	./programa

fast:
	gcc -o programa main.c base.c cores.c desenho.c filtros2d.c geometria.c imagem.c segmenta.c -lm -Ofast
	clear
	./programa

debug:
	gcc -g main.c base.c cores.c desenho.c filtros2d.c geometria.c imagem.c segmenta.c -lm -Wall -Wextra
