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
      mvwprintw(window, i, 1, "                        ");
    }
  }
  else if (size == 3)
  {
    for (int i = first; i <= last; i++)
    {
      mvwprintw(window, i, 1, "                       ");
    }
  }
}

void deleteBlocked(char removeSemaphore) // Deletar da lista de bloqueados o processo que foi liberado
{
  listaBloqueado *aux, *remover = NULL;

  if (bloqueados)
  {
    if (bloqueados->semaforo == removeSemaphore)
    {
      remover = bloqueados;
      bloqueados = remover->proximo;
    }
    else
    {
      aux = bloqueados;
      while (aux->proximo && aux->proximo->semaforo != removeSemaphore)
        aux = aux->proximo;
      if (aux->proximo)
      {
        remover = aux->proximo;
        aux->proximo = remover->proximo;
      }
    }
  }
}

void simpleProcessFinish(WINDOW *processWin) // Deleta o primeiro processo da fila de processamento (sem mostrar na janela)
{
  No *remover = NULL;

  if (cpuQueue)
  {
    remover = cpuQueue;
    cpuQueue = remover->proximo;
  }
  usleep(10000);
  imprimir(processWin);
}

void simpleProcessFinishDisk() // Retirar o primeiro processo fila de processos no disco
{
  No *remover = NULL;

  if (diskQueue)
  {
    remover = diskQueue;
    diskQueue = remover->proximo;
  }
}

void processFinish(WINDOW *listWin) // Deleta o primeiro processo da fila de processamento (mostrando na janela)
{
  No *remover = NULL;

  if (cpuQueue)
  {
    remover = cpuQueue;
    cpuQueue = remover->proximo;
    mvwprintw(listWin, 9, 1, "[Process excluded]                      ");
    wrefresh(listWin);
    sleep(2);
    mvwprintw(listWin, 9, 1, "                                        ");
    wrefresh(listWin);
  }
  else
  {
    mvwprintw(listWin, 9, 1, "[Empty List]                            ");
  }

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
  simulateTime(pcb, op1);
}

void memLoadReq(PCB *pcb, int op1, WINDOW *outputWin, WINDOW *listWin, WINDOW *logWin) // Calcular a memória restante
{
  size -= pcb[op1].programa.segmentSize;
  fflush(stdout);
  mvwprintw(logWin, 13, 1, "[Finished Memory Loading]");
  wrefresh(logWin);
  sleep(3);
  mvwprintw(logWin, 13, 1, "                          ");
  wrefresh(logWin);
}

int existenciaSemaforo(char verificarSemaforo) // Verificar se o semaforo dado ja existe na lista de semaforos
{
  for (int i = 0; i < 5; i++)
  {
    if (semaforos[i].existe == 1)
      if (semaforos[i].nome == verificarSemaforo)
        return 1;
  }
  for (int i = 0; i < 5; i++)
    if (semaforos[i].existe == 0)
    {
      semaforos[i].P = 0;
      semaforos[i].existe = 1;
      semaforos[i].nome = verificarSemaforo;
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

  // Percorre a cpuQueue e imprime os elementos
  No *aux = cpuQueue;
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

void printDisklist(WINDOW *diskWin)
{
  clearlines(diskWin, 2, 5, 3);

  // Percorre a cpuQueue e imprime os elementos
  No *aux = diskQueue;
  int linha = 3;

  while (aux != NULL)
  {
    if (linha == 3)
    {
      mvwprintw(diskWin, linha, 1, "%s [Running]", aux->processo.filename);
      wrefresh(diskWin);
    }
    else
    {
      mvwprintw(diskWin, linha, 1, "%s", aux->processo.filename);
      wrefresh(diskWin);
    }
    usleep(50000);
    linha++;
    aux = aux->proximo;
  }
  clearlines(diskWin, linha, 10, 3);

  wrefresh(diskWin); // Atualiza a janela processWin
}

void printBlocked(WINDOW *listWin)
{
  clearlines(listWin, 6, 12, 0);

  // Percorre a cpuQueue e imprime os elementos
  listaBloqueado *aux = bloqueados;

  int linha = 6;
  while (aux != NULL)
  {
    mvwprintw(listWin, linha, 1, "%s", aux->processo.filename);
    wrefresh(listWin);

    usleep(50000);
    linha++;
    aux = aux->proximo;
  }
  clearlines(listWin, linha, 10, 0);

  wrefresh(listWin); // Atualiza a janela processWin
}

void printRequest(WINDOW *diskWin)
{
  int i = 0;
  int rnd = 0;
  int timerprnt = cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].time;
  while (strcmp(cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].tipo, "print") == 0)
  {
    while (i <= cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].time)
    {
      mvwprintw(diskWin, 10, 1, "[Printing...] ");
      mvwprintw(diskWin, 7, 1, "[%s] -> [%d]", cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].tipo, i);
      wrefresh(diskWin);
      rnd = rand() % timerprnt / 2;
      i = i + rnd;
      if (timerprnt < 3)
      {
        usleep(5000);
        i = cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].time;
      }
      usleep(150000);
    }
    cpuQueue->processo.readpointer++;
  }
  printFinish(diskWin);
}

void printFinish(WINDOW *diskWin)
{
  mvwprintw(diskWin, 10, 1, "                          ");
  mvwprintw(diskWin, 7, 1, "                          ");
  wrefresh(diskWin);
}

int diskRequest(WINDOW *processWin, WINDOW *diskWin, sem_t *diskrun, int *initSession)
{
  // Copiar o elemento do começo da cpuqueue para a diskqueue e retirar ela da cpuqueue e colocar novamente (com novo ponteiro)

  No *auxi, *new = malloc(sizeof(No));
  if (new)
  {
    new->processo.estado = cpuQueue->processo.estado;
    new->processo.abort = cpuQueue->processo.abort;
    new->processo.continuar = cpuQueue->processo.continuar;
    new->processo.readpointer = cpuQueue->processo.readpointer;
    new->processo.execTime = cpuQueue->processo.execTime;
    new->processo.execIndex = cpuQueue->processo.execIndex;
    strcpy(new->processo.filename, cpuQueue->processo.filename);

    new->processo.programa.ID = cpuQueue->processo.programa.ID;
    new->processo.programa.PO = cpuQueue->processo.programa.ID;
    new->processo.programa.segmentSize = cpuQueue->processo.programa.ID;
    new->processo.programa.rt = cpuQueue->processo.programa.ID;
    strcpy(new->processo.programa.pname, cpuQueue->processo.programa.pname);

    for (int i = 0; i < cpuQueue->processo.execIndex; i++)
    {
      new->processo.vetExec[i].time = cpuQueue->processo.vetExec[i].time;
      strcpy(new->processo.vetExec[i].tipo, cpuQueue->processo.vetExec[i].tipo);
    }

    new->processo.nextThreadWait = cpuQueue->processo.nextThreadWait;
    new->processo.arquivo = cpuQueue->processo.arquivo;

    simpleProcessFinish(processWin); // Remove da cpuQueue
    if (cpuQueue == NULL)
    {
      *initSession = 0;
    }
    new->proximo = NULL;
    auxi = diskQueue;

    if (diskQueue == NULL)
    {
      diskQueue = new;
    }
    else
    {
      while (auxi->proximo)
        auxi = auxi->proximo;
      auxi->proximo = new;
    }
  }

  // Liberar semáforo na thread disk permitindo rodar a cpuQueue
  if (diskQueue->proximo == NULL)
  {
    sem_post(diskrun);
  }
  printDisklist(diskWin);
  // Agora a disk thread faz o trabalho de processar o read/write desse elemento
}

int diskFinish(sem_t *emptysession, sem_t *waitreinsert, sem_t *initProgram, int *initSession, sem_t *waitprocessing)
{
  // Copiar o elemento do comeco da cpuqueue para a diskqueue e retirar ela da cpuQueue e depois colocar ela de novo (com o novo ponteiro)
  No *aux, *novo = malloc(sizeof(No));

  if (novo)
  {
    novo->processo.estado = diskQueue->processo.estado;
    novo->processo.abort = diskQueue->processo.abort;
    novo->processo.continuar = diskQueue->processo.continuar;
    novo->processo.readpointer = diskQueue->processo.readpointer;
    novo->processo.execTime = diskQueue->processo.execTime;
    novo->processo.execIndex = diskQueue->processo.execIndex;
    strcpy(novo->processo.filename, diskQueue->processo.filename);

    novo->processo.programa.ID = diskQueue->processo.programa.ID;
    novo->processo.programa.PO = diskQueue->processo.programa.ID;
    novo->processo.programa.segmentSize = diskQueue->processo.programa.ID;
    novo->processo.programa.rt = diskQueue->processo.programa.ID;
    strcpy(novo->processo.programa.pname, diskQueue->processo.programa.pname);

    for (int i = 0; i < diskQueue->processo.execIndex; i++)
    {
      novo->processo.vetExec[i].time = diskQueue->processo.vetExec[i].time;
      strcpy(novo->processo.vetExec[i].tipo, diskQueue->processo.vetExec[i].tipo);
    }

    novo->processo.nextThreadWait = diskQueue->processo.nextThreadWait;
    novo->processo.arquivo = diskQueue->processo.arquivo;

    simpleProcessFinishDisk();

    novo->proximo = NULL;

    reinsert = 1; // Está reinserindo na cpuQueue, não deixar inserir um novo pelo processCreate

    if (startedprocessing == 1)
    { // Está inserindo na cpuQueue pelo diskFinish
      sem_wait(waitprocessing);
      startedprocessing = 0;
    }

    usleep(100000);

    aux = cpuQueue;
    if (cpuQueue == NULL)
    {
      cpuQueue = malloc(sizeof(No));
      novo->processo.estado = 1;
      cpuQueue = novo;
      finishedLoad = 0;
      sem_post(emptysession);
      usleep(100000);
      reinsert = 0;
      if (*initSession != 1)
      {
        sem_post(initProgram);
      }
      sem_post(waitreinsert);
      return 1;
    }
    else
    {
      if ((cpuQueue)->processo.execTime > novo->processo.execTime) //
      {
        cpuQueue->processo.abort = 1;
        novo->processo.estado = 1;
        novo->proximo = cpuQueue;
        cpuQueue = novo;
        finishedLoad = 0;
        usleep(100000);
        reinsert = 0;
        sem_post(waitreinsert);
        return 1; // Esse processo é o que possui mais prioridade.
      }
      else
      {
        novo->processo.estado = 0;
        while ((aux->proximo != NULL) && aux->proximo->processo.execTime < novo->processo.execTime)
          aux = aux->proximo;
        novo->proximo = aux->proximo;
        aux->proximo = novo;
        finishedLoad = 0;
        usleep(100000);
        reinsert = 0;
        sem_post(waitreinsert);
        return 0; // Processo entrou na cpuQueue
      }
    }
  }
}

int processCreate(FILE *instr, PCB *pcb, int op1, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin, char **filenames, sem_t *insertLower, WINDOW *logWin, int *initSession, sem_t *emptysession, sem_t *waitreinsert, sem_t *waitprocessing)
{
  char verificarSemaforo;
  int tempo;
  char linha[100];
  No *novo = malloc(sizeof(No));

  if (novo)
  {
    novo->processo.arquivo = instr;
    rewind(instr);
    novo->processo.execIndex = 0;
    novo->processo.execTime = 0;
    novo->processo.estado = 1;
    novo->processo.readpointer = 0;

    rewind(instr);

    fgets(novo->processo.programa.pname, sizeof(novo->processo.programa.pname), instr);
    fscanf(instr, "%d", &novo->processo.programa.ID);
    fscanf(instr, "%d", &novo->processo.programa.PO);
    fscanf(instr, "%d", &novo->processo.programa.segmentSize);

    strcpy(novo->processo.filename, filenames[op1]);

    memLoadReq(pcb, op1, outputWin, listWin, logWin);

    mvwprintw(logWin, 13, 1, "[Sucessful Loaded Process]");
    wrefresh(logWin);
    sleep(3);
    mvwprintw(logWin, 13, 1, "                          ");
    wrefresh(logWin);
    fseek(instr, 1, SEEK_CUR);

    char c;
    int f = 0;
    while (c != '\n')
    {
      fscanf(instr, "%c", &verificarSemaforo);
      semaforos[f].nome = verificarSemaforo;
      semaforos[f].existe = 1;
      semaforos[f].P = 0;
      c = fgetc(instr);
      f++;
    }

    // Ler os comandos "exec" e seus tempos correspondentes
    char tipo[10];
    int time = 0;
    int tempototal = 0;
    while (!feof(instr))
    {
      fscanf(instr, "%s %d", tipo, &time);

      mvwprintw(listWin, 2, 1, "Reading: [%s] %d      ", tipo, time);
      mvwprintw(listWin, 3, 1, "Total Time: [%d]                 ", tempototal);
      wrefresh(listWin); // ...
      usleep(200000);
      mvwprintw(listWin, 2, 1, "                                      ");
      mvwprintw(listWin, 3, 1, "                                      ");
      wrefresh(listWin);
      usleep(10000);
      if (strcmp(tipo, "P(s)") == 0)
      {
        time = 200;
      }
      else if (strcmp(tipo, "P(t)") == 0)
      {
        time = 200;
      }
      else if ((strcmp(tipo, "V(t)") == 0))
      {
        time = 0;
      }
      else if (strcmp(tipo, "V(s)") == 0)
      {
        time = 0;
      }

      strcpy(novo->processo.vetExec[novo->processo.execIndex].tipo, tipo);

      novo->processo.vetExec[novo->processo.execIndex].time = time;
      tempototal += time;
      novo->processo.execIndex++;
    }
    time = 0;
    novo->processo.execTime = tempototal;
    tempototal = 0;
    novo->processo.abort = 0;

    mvwprintw(logWin, 5, 1, "[Completed Reading.]");
    mvwprintw(logWin, 6, 1, "Starting Process...");
    wrefresh(logWin);
    sleep(1);
    mvwprintw(logWin, 5, 1, "                     ");
    mvwprintw(logWin, 6, 1, "                     ");
    wrefresh(logWin);

    novo->proximo = NULL;

    if (reinsert == 1)
    { // Está inserindo na cpuQueue pelo diskFinish
      sem_wait(waitreinsert);
      reinsert = 0;
    }

    startedprocessing = 1;

    No *aux = cpuQueue;

    if (cpuQueue == NULL)
    {
      cpuQueue = malloc(sizeof(No));
      novo->processo.estado = 1;
      cpuQueue = novo;
      fclose(novo->processo.arquivo);
      finishedLoad = 0;
      sem_post(emptysession);
      startedprocessing = 0;
      sem_post(waitprocessing);
      return 1;
    }
    else
    {
      if ((cpuQueue)->processo.execTime > novo->processo.execTime)
      {
        cpuQueue->processo.abort = 1;

        *initSession = 0;
        sem_wait(insertLower);
        usleep(10000);
        novo->processo.estado = 1;
        novo->proximo = cpuQueue;
        cpuQueue = novo;
        fclose(novo->processo.arquivo);
        finishedLoad = 0;
        startedprocessing = 0;
        sem_post(waitprocessing);
        return 1; // Esse processo é o que possui mais prioridade.
      }
      else
      {
        novo->processo.estado = 0; // 1 = Em execução; 0 = Em espera
        while ((aux->proximo != NULL) && aux->proximo->processo.execTime < novo->processo.execTime)
          aux = aux->proximo;
        novo->proximo = aux->proximo;
        aux->proximo = novo;
        imprimir(processWin);
        finishedLoad = 0;
        startedprocessing = 0;
        sem_post(waitprocessing);
        return 0; // Processo entrou na cpuQueue
      }
    }
  }
}

void semaphoreP(WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin)
{
  char stringaux[10];
  /// Verifica se o semáforo está ocupado ou não para inciar
  for (int t = 0; t < 5; t++) // Verifica se o semáforo está sendo utilizado, percorrendo o vetor de semáforos
  {
    stringaux[0] = cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].tipo[2];
    if ((stringaux[0] == semaforos[t].nome) && semaforos[t].P == 0) // Se verdade, significa que encontrou o semáforo e ele não está em uso
    {
      semaforos[t].P = 1;                                                                     // Semaforo está em uso
      semaforos[t].nome = cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].tipo[2]; // Colocar o semáforo na lista de semáforos
      usleep(100000);
      mvwprintw(outputWin, 2, 1, "                                             ");
      wattron(outputWin, COLOR_PAIR(1));
      mvwprintw(outputWin, 2, 1, "Semaphore [%c]: Enabled                       ", semaforos[t].nome);
      wattroff(outputWin, COLOR_PAIR(1));
      wrefresh(outputWin);

      sleep(3);
      break;
    }
    else if ((stringaux[0] == semaforos[t].nome) && semaforos[t].P == 1) // Encontrou o semáforo e ele está em uso, o processo será bloqueado
    {
      listaBloqueado *aux3, *novo3 = malloc(sizeof(listaBloqueado));

      if (novo3) // Verificar se foi alocado corretamente
      {
        // Copiar o processo atual para o novo nó
        novo3->processo.estado = cpuQueue->processo.estado;
        novo3->processo.abort = cpuQueue->processo.abort;
        novo3->processo.continuar = cpuQueue->processo.continuar;
        novo3->processo.readpointer = cpuQueue->processo.readpointer;
        novo3->processo.execTime = cpuQueue->processo.execTime;
        novo3->processo.execIndex = cpuQueue->processo.execIndex;
        strcpy(novo3->processo.filename, cpuQueue->processo.filename);
        novo3->processo.programa.ID = cpuQueue->processo.programa.ID;
        novo3->processo.programa.PO = cpuQueue->processo.programa.ID;
        novo3->processo.programa.segmentSize = cpuQueue->processo.programa.ID;
        novo3->processo.programa.rt = cpuQueue->processo.programa.ID;
        strcpy(novo3->processo.programa.pname, cpuQueue->processo.programa.pname);

        for (int q = 0; q < cpuQueue->processo.execIndex; q++)
        {
          novo3->processo.vetExec[q].time = cpuQueue->processo.vetExec[q].time;
          strcpy(novo3->processo.vetExec[q].tipo, cpuQueue->processo.vetExec[q].tipo);
        }

        novo3->processo.nextThreadWait = cpuQueue->processo.nextThreadWait;
        novo3->processo.arquivo = cpuQueue->processo.arquivo;
        novo3->semaforo = semaforos[t].nome;

        processFinish(listWin); // Remover elemento da cpuQueue
        imprimir(processWin);
        // Adicionar processo na lista de bloqueados
        novo3->proximo = NULL;
        aux3 = bloqueados;

        if (bloqueados == NULL)
        {
          bloqueados = novo3;
        }
        else
        {
          while (aux3->proximo)
            aux3 = aux3->proximo;
          aux3->proximo = novo3;
        }
        printBlocked(listWin);
      }
      return;
    }
  }
}

void semaphoreV(WINDOW *menuWin, WINDOW *outputWin, sem_t *waitreinsert, WINDOW *processWin, WINDOW *listWin)
{
  char stringaux[10];
  /// Procurar e desocupar o semáforo ocupado correspondente
  for (int t = 0; t < 5; t++)
  {
    stringaux[0] = cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].tipo[2];
    if ((stringaux[0] == semaforos[t].nome) && semaforos[t].P == 1)
    {
      semaforos[t].P = 0;
      usleep(100000);
      mvwprintw(outputWin, 2, 1, "                                             ");
      wattron(outputWin, COLOR_PAIR(2));
      mvwprintw(outputWin, 2, 1, "Semaphore [%c]: Disabled                     ", semaforos[t].nome);
      wattroff(outputWin, COLOR_PAIR(2));

      wrefresh(outputWin);
      sleep(3);
      listaBloqueado *auxx, *no = NULL;
      auxx = bloqueados;
      while (auxx && auxx->semaforo != cpuQueue->processo.vetExec[cpuQueue->processo.readpointer].tipo[2])
        auxx = auxx->proximo;
      if (auxx)
      {
        no = auxx;
        No *aux2, *novo2 = malloc(sizeof(No));
        if (novo2)
        {
          novo2->processo.estado = no->processo.estado;
          novo2->processo.abort = no->processo.abort;
          novo2->processo.continuar = no->processo.continuar;
          novo2->processo.readpointer = no->processo.readpointer - 1;
          novo2->processo.execTime = no->processo.execTime;
          novo2->processo.execIndex = no->processo.execIndex;
          strcpy(novo2->processo.filename, no->processo.filename);
          novo2->processo.programa.ID = no->processo.programa.ID;
          novo2->processo.programa.PO = no->processo.programa.ID;
          novo2->processo.programa.segmentSize = no->processo.programa.ID;
          novo2->processo.programa.rt = no->processo.programa.ID;
          strcpy(novo2->processo.programa.pname, no->processo.programa.pname);

          for (int i = 0; i < no->processo.execIndex; i++)
          {
            novo2->processo.vetExec[i].time = no->processo.vetExec[i].time;
            strcpy(novo2->processo.vetExec[i].tipo, no->processo.vetExec[i].tipo);
          }

          novo2->processo.nextThreadWait = no->processo.nextThreadWait;
          novo2->processo.arquivo = no->processo.arquivo;
          // Deletar processo da lista de bloqueados
          deleteBlocked(semaforos[t].nome);
          printBlocked(listWin);
          // Reinserir o processo bloqueado na lista de processos
          novo2->proximo = NULL;
          aux2 = cpuQueue;
          reinsert = 1;

          if (cpuQueue == NULL)
          {
            cpuQueue = malloc(sizeof(No));
            novo2->processo.estado = 1;
            cpuQueue = novo2;
            imprimir(processWin);
            finishedLoad = 0;
            usleep(100000);
            reinsert = 0;
            sem_post(waitreinsert);
          }
          else
          {
            if ((cpuQueue)->processo.execTime > novo2->processo.execTime) //
            {
              cpuQueue->processo.abort = 1;
              usleep(100000);
              cpuQueue->processo.readpointer++;
              novo2->processo.estado = 1;
              novo2->proximo = cpuQueue;
              cpuQueue = novo2;
              imprimir(processWin);
              finishedLoad = 0;
              usleep(100000);
              reinsert = 0;
              sem_post(waitreinsert);
            }
            else
            {
              novo2->processo.estado = 0; // 1 = Em execução; 0 = Em espera
              while ((aux2->proximo != NULL) && aux2->proximo->processo.execTime < novo2->processo.execTime)
                aux2 = aux2->proximo;
              novo2->proximo = aux2->proximo;
              aux2->proximo = novo2;
              imprimir(processWin);
              finishedLoad = 0;
              usleep(100000);
              reinsert = 0;
              sem_post(waitreinsert);
            }
          }
        }
      }
      break;
    }
  }
}

void loadingBar(WINDOW *outputWin, int sizebar, int iteration)
{
  int barWidth = 50;
  int i = 0;
  int filledWidth = (iteration * barWidth) / sizebar;

  mvwprintw(outputWin, 13, 1, "Progress [");
  for (i = 0; i < filledWidth; i++)
  {
    mvwprintw(outputWin, 13, i + 11, "=");
  }
  if (sizebar - 1 != iteration)
  {
    mvwprintw(outputWin, 13, barWidth + 2, "] %d%%", i * 100 / barWidth);
  }
  else
  {
    mvwprintw(outputWin, 13, barWidth + 2, "] 100%%");
  }
  wrefresh(outputWin);
}

void processar(WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin, char **filenames, void *arg, int *initSession, sem_t *insertLower, WINDOW *diskWin, sem_t *diskrun, sem_t *waitreinsert)
{
  ThreadArgs *args = (ThreadArgs *)arg;

  char stringaux[10];
  int k = 0;
  int opPaused = -1;
  int randn;
  int timer;

  srand(time(NULL));
  mvwprintw(outputWin, 13, 1, "                                                         ");

  wrefresh(menuWin);
  for (int i = cpuQueue->processo.readpointer; i < cpuQueue->processo.execIndex; i++)
  { // Percorrer cada comando
    usleep(10000);

    if (cpuQueue->processo.abort == 1)
    {
      sleep(1);
      mvwprintw(outputWin, 13, 1, "                                                       ");
      cpuQueue->processo.abort = 0;
      cpuQueue->processo.continuar = 1;
      cpuQueue->processo.readpointer = i;
      i = 0;
      sem_post(insertLower);
      return;
    }
    loadingBar(outputWin, cpuQueue->processo.execIndex, i);
    wrefresh(outputWin);

    cpuQueue->processo.execTime -= cpuQueue->processo.vetExec[i].time;

    if (paused == 1)
    {
      clearlines(menuWin, 1, 6, 0);
      mvwprintw(menuWin, 1, 1, "[Paused Function]        ");
      mvwprintw(menuWin, 2, 1, "[0] Continue: ");
      wrefresh(menuWin);
      wscanw(menuWin, " %d", &paused);
      sem_post(args->pauseSemaphore); // Libera semáforo quando é selecionada a opção de continuar

      wrefresh(menuWin);
      // Se a função foi despausada, atualiza a mensagem e continua a execução
      clearlines(menuWin, 1, 2, 0);
      usleep(10000);
      wrefresh(menuWin);
    }

    if (strcmp(cpuQueue->processo.vetExec[i].tipo, "read") == 0 ||
        strcmp(cpuQueue->processo.vetExec[i].tipo, "write") == 0)
    {
      usleep(100000);
      cpuQueue->processo.readpointer = i;
      diskRequest(processWin, diskWin, diskrun, initSession);
      return;
    }
    else if (strcmp(cpuQueue->processo.vetExec[i].tipo, "exec") == 0)
    {
      // Simular a redução do tempo (printando no terminal) e reduzir esse tempo no tempo total restante
      while (cpuQueue->processo.vetExec[i].time > 0) // Processar um comando do processo
      {
        mvwprintw(outputWin, 2, 1, "                                      ");
        mvwprintw(outputWin, 2, 1, "Running: [%s] =>  (%.1f)              ", cpuQueue->processo.vetExec[i].tipo, cpuQueue->processo.vetExec[i].time);
        wrefresh(outputWin); // ......
        timer = cpuQueue->processo.vetExec[i].time;
        if (cpuQueue->processo.vetExec[i].time > 10000)
        {
          usleep(500000);
          randn = rand() % timer / 2;
        }
        else if (cpuQueue->processo.vetExec[i].time >= 5000 && cpuQueue->processo.vetExec[i].time < 10000)
        {
          usleep(200000);
          randn = rand() % timer / 2;
        }
        else if (cpuQueue->processo.vetExec[i].time >= 100 && cpuQueue->processo.vetExec[i].time < 5000)
        {
          usleep(100000);
          randn = rand() % timer / 2;
        }

        if (cpuQueue->processo.vetExec[i].time >= 100)
        {
          cpuQueue->processo.vetExec[i].time -= randn;
        }
        else
        {
          usleep(100000);
          cpuQueue->processo.vetExec[i].time = 0;
        }
      }
    }
    else if (strcmp(cpuQueue->processo.vetExec[i].tipo, "print") == 0)
    {
      cpuQueue->processo.readpointer = i;
      printRequest(diskWin);
    }
    else // É um semáforo
    {
      if (cpuQueue->processo.vetExec[i].tipo[0] == 'P') // Encontrou o semáforo P, precisa ocupar
      {

        cpuQueue->processo.readpointer = i;
        semaphoreP(menuWin, outputWin, listWin, processWin);
      }
      else if (cpuQueue->processo.vetExec[i].tipo[0] == 'V') // Encontrou o semáforo V, precisa desocupar
      {
        cpuQueue->processo.readpointer = i;
        semaphoreV(menuWin, outputWin, waitreinsert, processWin, listWin);

        i = cpuQueue->processo.readpointer;
      }
    }
    if (cpuQueue == NULL)
    {
      return;
    }
    usleep(10000);
    wrefresh(listWin);
  }
  clearlines(outputWin, 5, 5, 1);
  mvwprintw(outputWin, 4, 1, "[Executed Processing]                ");
  wrefresh(outputWin);
  sleep(1);
  if (cpuQueue->proximo) // Há mais elementos na cpuQueue
  {
    cpuQueue->proximo->processo.estado = 1;
    processFinish(listWin); // Retirar processo que finalizou da cpuQueue e apontar para o próximo
    imprimir(processWin);
  }
  else // Não há mais elementos na cpuQueue
  {
    *initSession = 0;
    processFinish(listWin);
    sleep(2);
  }
}

void showLoadingSymbol(WINDOW *menuWin, void *arg2)
{
  const char *symbols[] = {"|", "/", "-", "\\"};
  const int numSymbols = sizeof(symbols) / sizeof(symbols[0]);
  int i = 0;

  LoadThread *args2 = (LoadThread *)arg2;

  while (1)
  {
    usleep(100000); // Ajuste o tempo de espera conforme necessário
    mvwprintw(menuWin, 1, 1, "Loading %s", symbols[i]);
    wrefresh(menuWin);
    i = (i + 1) % numSymbols;

    if (finishedLoad == 0)
    {
      sem_post(args2->loadingSymbol);
      return;
    }
  }
}

void *loadingThread(void *arg2)
{ // Apenas para razões estéticas
  LoadThread *args2 = (LoadThread *)arg2;
  WINDOW *menuWin = args2->menuWin;
  showLoadingSymbol(menuWin, args2);
}

void *processCreateThread(void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;
  WINDOW *menuWin = args->menuWin;
  WINDOW *outputWin = args->outputWin;
  WINDOW *listWin = args->listWin;
  WINDOW *processWin = args->processWin;
  WINDOW *diskWin = args->diskWin;
  char **filenames = args->filenames;
  int *initSession = args->initSession;

  // Processamento do programa
  usleep(100000);

  while (1)
  {
    if (cpuQueue)
    {
      usleep(10000);
      *initSession = 1; // Iniciou sessão
      processar(menuWin, outputWin, listWin, processWin, filenames, args, initSession, args->insertLower, diskWin, args->diskrun, args->waitreinsert);
      if (cpuQueue == NULL && bloqueados == NULL && diskQueue == NULL)
      {
        clearlines(outputWin, 12, 12, 1);
        clearlines(processWin, 2, 5, 0);
        wattron(outputWin, COLOR_PAIR(1));
        wattron(processWin, COLOR_PAIR(1));
        mvwprintw(processWin, 13, 1, "[Finished]");
        mvwprintw(outputWin, 12, 1, "[All processes have been executed]");
        wattroff(processWin, COLOR_PAIR(1));
        wattroff(outputWin, COLOR_PAIR(1));
        wrefresh(processWin);
        wrefresh(outputWin);
      }
      if (*initSession != 1) // Se for diferente de 1, é porque acabou a cpuQueue ou a thread começou.
      {
        sem_wait(args->initProgram);
      }
    }
    else
    {
      sem_wait(args->emptysession);
    }

    mvwprintw(processWin, 13, 1, "                   ");
    mvwprintw(outputWin, 12, 1, "                                  ");
    wrefresh(processWin);
    wrefresh(outputWin);
  }
  return NULL;
}

void *diskThread(void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;
  WINDOW *diskWin = args->diskWin;
  WINDOW *processWin = args->processWin;
  int i = 0;

  sem_wait(args->diskrun);
  while (1)
  {
    i = 0;
    if (!diskQueue)
    {
      sem_wait(args->diskrun);
    }
    while (diskQueue)
    {
      mvwprintw(diskWin, 13, 1, "[Moving] Read/Write head");
      while (strcmp(diskQueue->processo.vetExec[diskQueue->processo.readpointer].tipo, "read") == 0 ||
             strcmp(diskQueue->processo.vetExec[diskQueue->processo.readpointer].tipo, "write") == 0)
      {
        while (i <= diskQueue->processo.vetExec[diskQueue->processo.readpointer].time)
        {
          usleep(10000);
          mvwprintw(diskWin, 5, 1, "[%s] -> [%d]", diskQueue->processo.vetExec[diskQueue->processo.readpointer].tipo, i);
          wrefresh(diskWin);
          i++;
          if (i < 50)
          {
            usleep(150000);
          }
          else if (i >= 50 && i < 100)
          {
            usleep(100000);
          }
          else if (i >= 100)
          {
            usleep(30000);
          }
        }
        mvwprintw(diskWin, 13, 1, "                          ");
        mvwprintw(diskWin, 5, 1, "                          ");
        wrefresh(diskWin);
        diskQueue->processo.readpointer++;
      }
      usleep(10000);
      clearlines(diskWin, 2, 13, 0);
      usleep(10000);
      wrefresh(diskWin);
      diskFinish(args->emptysession, args->waitreinsert, args->initProgram, args->initSession, args->waitprocessing);
      imprimir(processWin);
    }
    if (diskQueue)
    {
      diskQueue = diskQueue->proximo;
      printDisklist(diskWin);
    }
  }
  return NULL;
}
