#include "funcoes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

int size = 125000;

void clearTerminal()
{
#ifdef _WIN32
  system("cls");
#else
  printf("\033[H\033[J");
#endif
}
void processar(WINDOW *menuWin, WINDOW *outputWin, PCB *pcb, WINDOW *listWin)
{
  char stringaux[10];
  int k;
  int opPaused = -1;

  srand(time(NULL));
  for (int i = 0; i < fila->processo.execIndex; i++)
  { // Percorrer cada comando
    fila->processo.execTime -= fila->processo.vetExec[i].time;

    if (paused == 1)
    {
      while (paused == 1)
      {
        mvwprintw(menuWin, 1, 1, "                                          ");
        mvwprintw(menuWin, 2, 1, "                                          ");
        mvwprintw(menuWin, 3, 1, "                                          ");
        mvwprintw(menuWin, 4, 1, "                                          ");
        mvwprintw(menuWin, 5, 1, "                                          ");
        mvwprintw(menuWin, 6, 1, "                                          ");

        mvwprintw(menuWin, 1, 1, "[Paused Function]                          ");
        mvwprintw(menuWin, 2, 1, "[0] Continue: ");
        wrefresh(menuWin);
        wscanw(menuWin, " %d", &paused);

        wrefresh(menuWin);
        fflush(stdout);
      }
      // Se a função foi despausada, atualiza a mensagem e continua a execução
      mvwprintw(menuWin, 1, 1, "                                          ");
      mvwprintw(menuWin, 2, 1, "                                          ");
      usleep(10000);
      wrefresh(menuWin);
    }

    if (strcmp(fila->processo.vetExec[i].tipo, "exec") == 0 ||
        strcmp(fila->processo.vetExec[i].tipo, "read") == 0 ||
        strcmp(fila->processo.vetExec[i].tipo, "write") == 0 ||
        strcmp(fila->processo.vetExec[i].tipo, "print") == 0)
    {                                            // Simular a redução do tempo (printando no terminal)
                                                 // e reduzir esse tempo no tempo total restante
      while (fila->processo.vetExec[i].time > 0) // Processar um comando do processo
      {
        mvwprintw(outputWin, 2, 1, "                                      ");
        mvwprintw(outputWin, 1, 1, "Execution number [%d]                ", i);
        mvwprintw(outputWin, 2, 1, "Running: [%s] =>  (%.1f)              ", fila->processo.vetExec[i].tipo, fila->processo.vetExec[i].time);

        wrefresh(outputWin); // ......
        if (fila->processo.vetExec[i].time > 10000)
        {
          usleep(5000);
        }
        else if (fila->processo.vetExec[i].time >= 5000 &&
                 fila->processo.vetExec[i].time < 10000)
        {
          usleep(10000);
        }
        else if (fila->processo.vetExec[i].time >= 100 && fila->processo.vetExec[i].time < 5000)
        {
          usleep(50000);
        }
        else if (fila->processo.vetExec[i].time < 100)
        {
          usleep(200000);
        }

        int randn = rand() % 50 + 10;
        fila->processo.vetExec[i].time -= randn;
        wrefresh(outputWin);
      }
    }
    else // É um semáforo
    {
      if (fila->processo.vetExec[i].tipo[0] == 'P') // Encontrou o semáforo P, precisa ocupar
      {
        semaphoreP(i, menuWin, outputWin);
      }
      else if (fila->processo.vetExec[i].tipo[0] == 'V') // Encontrou o semáforo V, precisa desocupar
      {
        semaphoreV(i, menuWin, outputWin);
      }
    }
    mvwprintw(listWin, 5, 1, "Total Remaining Time: [%d]             ", fila->processo.execTime);
    wrefresh(listWin); // ......
    sleep(1);
  }
  // wclear(outputWin);
  mvwprintw(outputWin, 5, 1, "                                                 ");
  mvwprintw(outputWin, 4, 1, "[Executed Processing]                ");
  wrefresh(outputWin);
  sleep(2);
  if (fila->proximo)
  {
    processFinish(listWin);
    fila->proximo->processo.estado = 1;
    processCreate(fila->proximo->processo.arquivo, pcb, 0, menuWin, outputWin, listWin);
  }
  else
  {
    // wclear(outputWin);
    // box(outputWin, 1, 1);
    wattron(outputWin, COLOR_PAIR(1));
    mvwprintw(outputWin, 1, 1, "[All processes have been executed]                        ");
    wattroff(outputWin, COLOR_PAIR(1));
    wrefresh(outputWin);
    sleep(5);
  }
}

void processFinish(WINDOW *listWin)
{
  No *remover = NULL;

  if (fila)
  {
    remover = fila;
    fila = remover->proximo;
    mvwprintw(listWin, 8, 1, "[Process removed]                      ");
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

void memLoadReq(PCB *pcb, int op1, WINDOW *outputWin, WINDOW *listWin)
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

int inserir_com_prioridade(PCB *num, int op1, FILE *arquivo, WINDOW *menuWin)
{
  char copiar;
  No *aux = fila;
  No *novo = malloc(sizeof(No));
  if (novo)
  {
    novo->processo.arquivo = fopen("app.sint", "w+");
    rewind(arquivo);
    while (!feof(arquivo))
    {
      fscanf(arquivo, "%c", &copiar);
      fprintf(novo->processo.arquivo, "%c", copiar);
    }
    fseek(novo->processo.arquivo, -1, SEEK_END); // Posicionar o indicador de posição do arquivo no final - 1
    fputc('\n', novo->processo.arquivo);
    // create
    novo->processo.execIndex = num[op1].execIndex;
    novo->processo.execTime = num[op1].execTime;

    // execucao
    for (int i = 0; i < num[op1].execIndex; i++)
    {
      novo->processo.vetExec[i].time = num[op1].vetExec[i].time;
      strcpy(novo->processo.vetExec[i].tipo, num[op1].vetExec[i].tipo);
    }

    // Programa
    novo->processo.programa.ID = num[op1].programa.ID;
    strcpy(novo->processo.programa.pname, num[op1].programa.pname);
    novo->processo.programa.PO = num[op1].programa.PO;
    novo->processo.programa.rt = num[op1].programa.rt;
    novo->processo.programa.segmentSize = num[op1].programa.segmentSize;
    novo->proximo = NULL;
    if (fila == NULL)
    {
      fila = malloc(sizeof(No));
      novo->processo.estado = 1;
      fila = novo;
      fclose(novo->processo.arquivo);
      return 1;
    }
    else
    {
      if ((fila)->processo.execTime > novo->processo.execTime)
      {
        novo->processo.estado = 0;
        novo->proximo = fila;
        fila = novo;
        fclose(novo->processo.arquivo);

        return 1; // Esse processo é o que possui mais prioridade.
      }
      else
      {
        novo->processo.estado = 0; // 1 = em execucao; 0 = em espera
        while (aux->proximo && aux->proximo->processo.execTime < novo->processo.execTime)
          aux = aux->proximo;
        novo->proximo = aux->proximo;
        aux->proximo = novo;
        fila = novo;
        return 0; // Processo entrou na fila
      }
    }
  }
}