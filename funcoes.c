#include "funcoes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <semaphore.h>

int size = 125000;

void clearTerminal()
{
#ifdef _WIN32
  system("cls");
#else
  printf("\033[H\033[J");
#endif
}

void clearlines(WINDOW *window, int first, int last, int size)
{
  // size = 0, limpar linhas pequenas
  // size = 1, limpar linhas maiores

  if (size == 1)
  {
    for (int i = first; i <= last; i++)
    {
      mvwprintw(window, i, 1, "                                          ");
    }
  }
  else if (size == 0)
  {
    for (int i = first; i <= last; i++)
    {
      mvwprintw(window, i, 1, "                      ");
    }
  }
}

void processFinish(WINDOW *listWin)
{
  No *remover = NULL;

  if (fila)
  {
    remover = fila;
    fila = remover->proximo;
    mvwprintw(listWin, 8, 1, "[Process excluded]                      ");
    wrefresh(listWin);
  }
  else
    mvwprintw(listWin, 8, 1, "[Empty List]                            ");
  wrefresh(listWin);
}

void simulateTime(PCB *pcb, int op1)
{
  if (pcb[op1].estado == 0)
  {
    for (pcb[op1].programa.rt = 100000; pcb[op1].programa.rt >= 0; pcb[op1].programa.rt--)
    {
      pcb[op1].programa.rt -= 0.1;
      printf("%.1f\n", pcb[op1].programa.rt);
      if (pcb[op1].programa.rt > 10000)
      {
        usleep(50);
      }
      else if (pcb[op1].programa.rt >= 5000 && pcb[op1].programa.rt < 10000)
      {
        usleep(200);
      }
      else if (pcb[op1].programa.rt >= 100 && pcb[op1].programa.rt < 5000)
      {
        usleep(500);
      }
      else if (pcb[op1].programa.rt < 100)
      {
        usleep(10000);
      }
      clearTerminal();
    }
  }
}

void processInterrupt(PCB *pcb, int op1)
{
  simulateTime(pcb, op1); // Testando a estética de diminuir o tempo de execução
}

void memLoadReq(PCB *pcb, int op1, WINDOW *outputWin, WINDOW *listWin) // TIRAR O LISTWIN porque teoricamente é inútil
{
  size -= pcb[op1].programa.segmentSize;
  fflush(stdout);
  mvwprintw(outputWin, 8, 1, "Unused Space: %d kbytes                ", size);
  mvwprintw(outputWin, 9, 1, "[Finished Memory Loading]              ");
  wrefresh(outputWin);
}

int existenciaSemaforo(char verificarSemaforo)
{
  for (int i = 0; i < 5; i++)
  {
    if (semaforos[i].existe == 1)
      if (strcmp(semaforos[i].nome, &verificarSemaforo) == 0)
        return 1;
  }
  for (int i = 0; i < 5; i++)
    if (semaforos[i].existe == 0)
    {
      semaforos[i].P = 0;
      semaforos[i].existe = 1;
      strcpy(semaforos[i].nome, &verificarSemaforo);
    }
  return 0;
}

void memLoadFinish(WINDOW *outputWin, WINDOW *listWin)
{
  mvwprintw(outputWin, 10, 1, "[Final De Carregamento de Memória]        ");
  wrefresh(outputWin);
}



void imprimir(WINDOW *processWin)
{
  clearlines(processWin, 2, 5, 0);

  /* if ((fila)->proximo != NULL)
  {
    mvwprintw(processWin, 7, 1, "%d", (fila)->processo.execTime);
    mvwprintw(processWin, 8, 1, "%d", (fila)->proximo->processo.execTime);
    wrefresh(processWin);
    sleep(5);
  } */

  // Percorre a fila e imprime os elementos
  No *aux = fila;
  int linha = 3;

  while (aux != NULL)
  {
    if (linha == 3)
    {
      mvwprintw(processWin, linha, 1, "%s [Running]", aux->processo.filename);
      wrefresh(processWin);
    }
    else
    {
      mvwprintw(processWin, linha, 1, "%s", aux->processo.filename);
      wrefresh(processWin);
    }

    usleep(50000);
    linha++;
    aux = aux->proximo;
  }

  clearlines(processWin, linha, 10, 0);

  wrefresh(processWin); // Atualiza a janela processWin
}