#include "funcoes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <semaphore.h>

#define tamanho 1000

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
  else if (size == 3)
  {
    for (int i = first; i <= last; i++)
    {
      mvwprintw(window, i, 1, "                       ");
    }
  }
}

void deleteBlocked(char removeSemaphore)
{
  listaBloqueado *aux, *remover = NULL;

    if(bloqueados){
        if(bloqueados->semaforo == removeSemaphore){
            remover = bloqueados;
            bloqueados = remover->proximo;
        }
        else{
            aux = bloqueados;
            while(aux->proximo && aux->proximo->semaforo != removeSemaphore)
                aux = aux->proximo;
            if(aux->proximo){
                remover = aux->proximo;
                aux->proximo = remover->proximo;
            }
        }
    }
}

void simpleProcessFinish(WINDOW *processWin)
{
  No *remover = NULL;

  if (fila)
  {
    remover = fila;
    fila = remover->proximo;
  }
  imprimir(processWin);
}

void simpleProcessFinishDisk()
{
  No *remover = NULL;

  if (fila2)
  {
    remover = fila2;
    fila2 = remover->proximo;
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

void memLoadReq(PCB *pcb, int op1, WINDOW *outputWin, WINDOW *listWin, WINDOW *logWin) // TIRAR O LISTWIN porque teoricamente é inútil
{
  size -= pcb[op1].programa.segmentSize;
  fflush(stdout);
  // mvwprintw(logWin, 13, 1, "Unused Space: %d kbytes", size);
  // wattron(menuWin, COLOR_PAIR(2));
  mvwprintw(logWin, 13, 1, "[Finished Memory Loading]");
  wrefresh(logWin);
  // wattroff(menuWin, COLOR_PAIR(2));
  sleep(3);
  mvwprintw(logWin, 13, 1, "                          ");
  // clearlines(logWin, 8, 9, 1);
  wrefresh(logWin);
}

int existenciaSemaforo(char verificarSemaforo)
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
      //strcpy(semaforos[i].nome, &verificarSemaforo);
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

void printDisklist(WINDOW *diskWin)
{
  clearlines(diskWin, 2, 5, 3);

  // Percorre a fila e imprime os elementos
  No *aux = fila2;
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
  clearlines(listWin, 2, 5, 0);

  // Percorre a fila e imprime os elementos
  listaBloqueado *aux = bloqueados;

  int linha = 5;

  while (aux != NULL)
  {
    if (linha == 3)
    {
      mvwprintw(listWin, linha, 1, "%d", aux->processo.filename);
      wrefresh(listWin);
    }
    else
    {
      mvwprintw(listWin, linha, 1, "%s", aux->processo.filename);
      wrefresh(listWin);
    }

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
  while (strcmp(fila->processo.vetExec[fila->processo.ponteiro_leitura].tipo, "print") == 0)
  {
    while (i <= fila->processo.vetExec[fila->processo.ponteiro_leitura].time)
    {
      mvwprintw(diskWin, 10, 1, "[Printing...] ");
      mvwprintw(diskWin, 7, 1, "[%s] -> [%d]", fila->processo.vetExec[fila->processo.ponteiro_leitura].tipo, i);
      wrefresh(diskWin);
      i++;
      usleep(150000);
    }
    // printar devagar
    fila->processo.ponteiro_leitura++;
  }
  printFinish(diskWin);
}

void printFinish(WINDOW *diskWin)
{
  mvwprintw(diskWin, 10, 1, "                          ");
  mvwprintw(diskWin, 7, 1, "                          ");
  wrefresh(diskWin);
  // semaforo pra voltar pro processar?
}

int colocarDisco(WINDOW *processWin, WINDOW *diskWin, sem_t *diskrun)
{
  // copiar o elemento do comeco da fila para a fila2 e retirar ela da fila e depois colocar ela de novo (com o novo ponteiro)

  No *auxi, *new = malloc(sizeof(No));
  if (new)
  {
    // variaveis simples
    new->processo.estado = fila->processo.estado;
    new->processo.abort = fila->processo.abort;
    new->processo.continuar = fila->processo.continuar;
    new->processo.ponteiro_leitura = fila->processo.ponteiro_leitura;
    new->processo.execTime = fila->processo.execTime;
    new->processo.execIndex = fila->processo.execIndex;
    strcpy(new->processo.filename, fila->processo.filename);
    // program
    new->processo.programa.ID = fila->processo.programa.ID;
    new->processo.programa.PO = fila->processo.programa.ID;
    new->processo.programa.segmentSize = fila->processo.programa.ID;
    new->processo.programa.rt = fila->processo.programa.ID;
    strcpy(new->processo.programa.pname, fila->processo.programa.pname);

    // vetExec
    for (int i = 0; i < fila->processo.execIndex; i++)
    {
      new->processo.vetExec[i].time = fila->processo.vetExec[i].time;
      strcpy(new->processo.vetExec[i].tipo, fila->processo.vetExec[i].tipo);
    }

    new->processo.nextThreadWait = fila->processo.nextThreadWait;
    new->processo.arquivo = fila->processo.arquivo;

    simpleProcessFinish(processWin); // REMOVE DA FILA
    new->proximo = NULL;
    auxi = fila2;

    // é o primeiro?
    if (fila2 == NULL)
    {
      fila2 = new;
    }
    else
    {
      // aux = fila2;
      while (auxi->proximo)
        auxi = auxi->proximo;
      auxi->proximo = new;
    }
    // free(new);
    // free(auxi);
  }
  // PROCESSAR ELA NA OUTRA THREAD

  // Liberar semáforo na thread disk permitindo rodar a fila

  if (fila2->proximo == NULL)
  {
    sem_post(diskrun);
  }
  printDisklist(diskWin);
  // Agora a disk thread faz o trabalho de processar o read/write desse elemento
}

int recolocarFila(sem_t *emptysession, sem_t *waitreinsert, sem_t *initProgram, int *initSession)
{
  // copiar o elemento do comeco da fila para a fila2 e retirar ela da fila e depois colocar ela de novo (com o novo ponteiro)
  char copiar;
  No *aux, *novo = malloc(sizeof(No));

  if (novo)
  {
    // variaveis simples
    novo->processo.estado = fila2->processo.estado;
    novo->processo.abort = fila2->processo.abort;
    novo->processo.continuar = fila2->processo.continuar;
    novo->processo.ponteiro_leitura = fila2->processo.ponteiro_leitura;
    novo->processo.execTime = fila2->processo.execTime;
    novo->processo.execIndex = fila2->processo.execIndex;
    strcpy(novo->processo.filename, fila2->processo.filename);
    // program
    novo->processo.programa.ID = fila2->processo.programa.ID;
    novo->processo.programa.PO = fila2->processo.programa.ID;
    novo->processo.programa.segmentSize = fila2->processo.programa.ID;
    novo->processo.programa.rt = fila2->processo.programa.ID;
    strcpy(novo->processo.programa.pname, fila2->processo.programa.pname);

    // vetExec
    for (int i = 0; i < fila2->processo.execIndex; i++)
    {
      novo->processo.vetExec[i].time = fila2->processo.vetExec[i].time;
      strcpy(novo->processo.vetExec[i].tipo, fila2->processo.vetExec[i].tipo);
    }

    novo->processo.nextThreadWait = fila2->processo.nextThreadWait;
    novo->processo.arquivo = fila2->processo.arquivo;

    simpleProcessFinishDisk();

    novo->proximo = NULL;

    reinsert = 1; // Está reinserindo na fila, não deixar inserir um novo pelo processCreate
    usleep(100000);

    aux = fila;
    if (fila == NULL)
    {
      fila = malloc(sizeof(No));
      novo->processo.estado = 1;
      fila = novo;
      // fclose(novo->processo.arquivo);
      finishedLoad = 0;
      sem_post(emptysession);
      usleep(100000);
      reinsert = 0;
      /* if(*initSession != 1) {
        sem_post(initProgram);
      } */ 
      sem_post(waitreinsert);
      return 1;
    }
    else
    {
      if ((fila)->processo.execTime > novo->processo.execTime) //
      {
        fila->processo.abort = 1;
        novo->processo.estado = 1;
        novo->proximo = fila;
        fila = novo;
        // fclose(novo->processo.arquivo);
        finishedLoad = 0;
        usleep(100000);
        reinsert = 0;
        sem_post(waitreinsert);
        return 1; // Esse processo é o que possui mais prioridade.
      }
      else
      {
        novo->processo.estado = 0; // 1 = em execucao; 0 = em espera
        while ((aux->proximo != NULL) && aux->proximo->processo.execTime < novo->processo.execTime)
          aux = aux->proximo;
        novo->proximo = aux->proximo;
        aux->proximo = novo;
        // imprimir(processWin);
        finishedLoad = 0;
        usleep(100000);
        reinsert = 0;
        sem_post(waitreinsert);
        return 0; // Processo entrou na fila
      }
    }
  }
}

int processCreate(FILE *instr, PCB *pcb, int op1, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin, char **filenames, sem_t *insertLower, WINDOW *logWin, int *initSession, sem_t *emptysession, sem_t *waitreinsert)
{
  char verificarSemaforo;
  char execucao[50];
  int tempo;
  int posVetExec = 0;
  char linha[100];
  int foundEmptyLine = 0;

  char copiar;
  No *aux = fila;
  No *novo = malloc(sizeof(No));

  if (novo)
  {
    novo->processo.arquivo = instr;
    rewind(instr);
    novo->processo.execIndex = 0;
    novo->processo.execTime = 0;
    novo->processo.estado = 1;
    novo->processo.ponteiro_leitura = 0;

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
      // strcpy(semaforos[f].nome, &verificarSemaforo);
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
    { // Está inserindo na fila pelo diskFinish
      sem_wait(waitreinsert);
      reinsert = 0;
    }

    if (fila == NULL)
    {
      fila = malloc(sizeof(No));
      novo->processo.estado = 1;
      fila = novo;
      fclose(novo->processo.arquivo);
      finishedLoad = 0;
      sem_post(emptysession);
      return 1;
    }
    else
    {
      if ((fila)->processo.execTime > novo->processo.execTime)
      {
        fila->processo.abort = 1;

        *initSession = 0;
        sem_wait(insertLower);
        usleep(10000);
        novo->processo.estado = 1;
        novo->proximo = fila;
        fila = novo;
        fclose(novo->processo.arquivo);
        finishedLoad = 0;
        return 1; // Esse processo é o que possui mais prioridade.
      }
      else
      {
        novo->processo.estado = 0; // 1 = em execucao; 0 = em espera
        while ((aux->proximo != NULL) && aux->proximo->processo.execTime < novo->processo.execTime)
          aux = aux->proximo;
        novo->proximo = aux->proximo;
        aux->proximo = novo;
        imprimir(processWin);
        finishedLoad = 0;
        return 0; // Processo entrou na fila
      }
    }
  }
}

void semaphoreP(int i, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin)
{
  char stringaux[10];
  /// Verifica se o semáforo está ocupado ou não para inciar
  for (int t = 0; t < 5; t++) // Verifica se o semáforo está sendo utilizado, percorrendo o vetor de semáforos
  {
    stringaux[0] = fila->processo.vetExec[fila->processo.ponteiro_leitura].tipo[2];
    if ((stringaux[0] == semaforos[t].nome) && semaforos[t].P == 0) // Se entrar nesse if, encontrou o semáforo e ele não está em uso
    {
      semaforos[t].P = 1;
      semaforos[t].nome = fila->processo.vetExec[fila->processo.ponteiro_leitura].tipo[2];
      usleep(100000);
      // mvwprintw(outputWin, 1, 1, "Execution number %d                 ", i);
      mvwprintw(outputWin, 2, 1, "                                             ");
      wattron(outputWin, COLOR_PAIR(1));
      mvwprintw(outputWin, 2, 1, "Semaphore [%c]: Enabled                       ", fila->processo.vetExec[i].tipo[2]);
      wattroff(outputWin, COLOR_PAIR(1));
      wrefresh(outputWin);

      sleep(3);
      break; // Sai do for...
    }
    else if ((stringaux[0] == semaforos[t].nome) && semaforos[t].P == 1) // encontrou o semaforo e esta em uso, o processo precisa ser bloqueado
    {
      listaBloqueado *aux3, *novo3 = malloc(sizeof(listaBloqueado));

      if (novo3)
      {
        // variaveis simples
        novo3->processo.estado = fila->processo.estado;
        novo3->processo.abort = fila->processo.abort;
        novo3->processo.continuar = fila->processo.continuar;
        novo3->processo.ponteiro_leitura = fila->processo.ponteiro_leitura;
        novo3->processo.execTime = fila->processo.execTime;
        novo3->processo.execIndex = fila->processo.execIndex;
        strcpy(novo3->processo.filename, fila->processo.filename);
        // program
        novo3->processo.programa.ID = fila->processo.programa.ID;
        novo3->processo.programa.PO = fila->processo.programa.ID;
        novo3->processo.programa.segmentSize = fila->processo.programa.ID;
        novo3->processo.programa.rt = fila->processo.programa.ID;
        strcpy(novo3->processo.programa.pname, fila->processo.programa.pname);

        // vetExec
        for (int q = 0; q < fila->processo.execIndex; q++)
        {
          novo3->processo.vetExec[q].time = fila->processo.vetExec[q].time;
          strcpy(novo3->processo.vetExec[q].tipo, fila->processo.vetExec[q].tipo);
        }

        novo3->processo.nextThreadWait = fila->processo.nextThreadWait;
        novo3->processo.arquivo = fila->processo.arquivo;
        novo3->semaforo = semaforos[t].nome; // guardar qual semaforo ele ta esperando liberar

        processFinish(listWin); // REMOVE DA FILA
        imprimir(processWin);
        novo3->proximo = NULL;
        aux3 = bloqueados;

        // é o primeiro?
        if (bloqueados == NULL)
        {
          bloqueados = novo3;
        }
        else
        {
          // aux = fila2;
          while (aux3->proximo)
            aux3 = aux3->proximo;
          aux3->proximo = novo3;
        }
        printBlocked(listWin);
      }
      return;
      // break;
    }
  }
}

void semaphoreV(int i, WINDOW *menuWin, WINDOW *outputWin, sem_t *waitreinsert, WINDOW *processWin, WINDOW *listWin)
{
  char stringaux[10];
  /// Desocupa o semáforo ocupado correspondente à posição i
  for (int t = 0; t < 5; t++) // Percorre o vetor de semáforos
  {
    stringaux[0] = fila->processo.vetExec[fila->processo.ponteiro_leitura].tipo[2];
    if ((stringaux[0] == semaforos[t].nome) && semaforos[t].P == 1) // Desocupa esse semáforo setando como 0!!
    {
      semaforos[t].P = 0;
      usleep(100000);
      // mvwprintw(outputWin, 1, 1, "Execution number %d                          ", i);
      mvwprintw(outputWin, 2, 1, "                                             ");
      wattron(outputWin, COLOR_PAIR(2));
      mvwprintw(outputWin, 2, 1, "Semaphore [%c]: Disabled                     ", fila->processo.vetExec[i].tipo[2]);
      wattroff(outputWin, COLOR_PAIR(2));

      wrefresh(outputWin);
      sleep(3);
      listaBloqueado *auxx, *no = NULL;
      auxx = bloqueados;
      while (auxx && auxx->semaforo != fila->processo.vetExec[fila->processo.ponteiro_leitura].tipo[2])
        auxx = auxx->proximo;
      if (auxx) // encontrou aquele que precisa ser liberado
      {
        no = auxx;
        No *aux2, *novo2 = malloc(sizeof(No));
        if (novo2)
        {
          // variaveis simples
          novo2->processo.estado = no->processo.estado;
          novo2->processo.abort = no->processo.abort;
          novo2->processo.continuar = no->processo.continuar;
          novo2->processo.ponteiro_leitura = no->processo.ponteiro_leitura - 1;
          novo2->processo.execTime = no->processo.execTime;
          novo2->processo.execIndex = no->processo.execIndex;
          strcpy(novo2->processo.filename, no->processo.filename);
          // program
          novo2->processo.programa.ID = no->processo.programa.ID;
          novo2->processo.programa.PO = no->processo.programa.ID;
          novo2->processo.programa.segmentSize = no->processo.programa.ID;
          novo2->processo.programa.rt = no->processo.programa.ID;
          strcpy(novo2->processo.programa.pname, no->processo.programa.pname);

          // vetExec
          for (int i = 0; i < no->processo.execIndex; i++)
          {
            novo2->processo.vetExec[i].time = no->processo.vetExec[i].time;
            strcpy(novo2->processo.vetExec[i].tipo, no->processo.vetExec[i].tipo);
          }

          novo2->processo.nextThreadWait = no->processo.nextThreadWait;
          novo2->processo.arquivo = no->processo.arquivo;

          deleteBlocked(semaforos[t].nome);
          printBlocked(listWin);
          novo2->proximo = NULL;
          aux2 = fila;
          reinsert = 1;

          if (fila == NULL)
          {
            fila = malloc(sizeof(No));
            novo2->processo.estado = 1;
            fila = novo2;
            // fclose(novo2->processo.arquivo);
            imprimir(processWin);
            finishedLoad = 0;
            usleep(100000);
            reinsert = 0;
            sem_post(waitreinsert);
          }
          else
          {
            if ((fila)->processo.execTime > novo2->processo.execTime) //
            {
              fila->processo.abort = 1;
              usleep(100000);
              // sem_wait(insertLower);
              /* while(global != 1) {
                usleep(10000);
              }*/
              fila->processo.ponteiro_leitura++;
              novo2->processo.estado = 1;
              novo2->proximo = fila;
              fila = novo2;
              // fclose(novo2->processo.arquivo);
              imprimir(processWin);
              finishedLoad = 0;
              usleep(100000);
              reinsert = 0;
              sem_post(waitreinsert);
            }
            else
            {
              novo2->processo.estado = 0; // 1 = em execucao; 0 = em espera
              while ((aux2->proximo != NULL) && aux2->proximo->processo.execTime < novo2->processo.execTime)
                aux2 = aux2->proximo;
              novo2->proximo = aux2->proximo;
              aux2->proximo = novo2;
              // imprimir(processWin);
              imprimir(processWin);
              finishedLoad = 0;
              usleep(100000);
              reinsert = 0;
              sem_post(waitreinsert);
            }
          }
        }
      }
      // free(auxx);
      // free(no);
      break;
    }
  }
}

void loadingBar(WINDOW *outputWin, int sizebar, int iteration)
{
  // Calcula o tamanho da barra de preenchimento
  int barWidth = 50;
  int i = 0;
  // Calcula a quantidade de caracteres preenchidos proporcionalmente ao tamanho da barra
  int filledWidth = (iteration * barWidth) / sizebar;

  // Desenha a barra de carregamento
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

  // Atualiza a janela de saída
  wrefresh(outputWin);
}

void processar(WINDOW *menuWin, WINDOW *outputWin, PCB *pcb, WINDOW *listWin, WINDOW *processWin, char **filenames, void *arg, int *initSession, sem_t *insertLower, WINDOW *diskWin, sem_t *diskrun, sem_t *waitreinsert)
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
  for (int i = fila->processo.ponteiro_leitura; i < fila->processo.execIndex; i++)
  { // Percorrer cada comando
    usleep(10000);

    if (fila->processo.abort == 1)
    {
      sleep(1);
      mvwprintw(outputWin, 13, 1, "                                                       ");
      fila->processo.abort = 0;
      fila->processo.continuar = 1;
      fila->processo.ponteiro_leitura = i;
      i = 0;
      // fflush(stdout);
      sem_post(insertLower);
      return;
    }

    fila->processo.execTime -= fila->processo.vetExec[i].time;

    if (paused == 1)
    {
      clearlines(menuWin, 1, 6, 0);
      mvwprintw(menuWin, 1, 1, "[Paused Function]        ");
      mvwprintw(menuWin, 2, 1, "[0] Continue: ");
      wrefresh(menuWin);
      wscanw(menuWin, " %d", &paused);
      sem_post(args->pauseSemaphore); // Libera semáforo quando é selecionada a opção de continuar

      wrefresh(menuWin);
      // fflush(stdout);
      // Se a função foi despausada, atualiza a mensagem e continua a execução
      clearlines(menuWin, 1, 2, 0);
      usleep(10000);
      wrefresh(menuWin);
    }

    if (strcmp(fila->processo.vetExec[i].tipo, "read") == 0 ||
        strcmp(fila->processo.vetExec[i].tipo, "write") == 0)
    {
      usleep(100000);
      fila->processo.ponteiro_leitura = i;
      colocarDisco(processWin, diskWin, diskrun);
      return;
      //  esperar funcao responde e retirar da fila
      //   colocar na outra thread
    }
    else if (strcmp(fila->processo.vetExec[i].tipo, "exec") == 0)
    {
      // Simular a redução do tempo (printando no terminal) e reduzir esse tempo no tempo total restante
      while (fila->processo.vetExec[i].time > 0) // Processar um comando do processo
      {
        mvwprintw(outputWin, 2, 1, "                                      ");
        ///
        loadingBar(outputWin, fila->processo.execIndex, i);
        ///
        mvwprintw(outputWin, 1, 1, "[Command Window]     ", i);
        mvwprintw(outputWin, 2, 1, "Running: [%s] =>  (%.1f)              ", fila->processo.vetExec[i].tipo, fila->processo.vetExec[i].time);
        wrefresh(outputWin); // ......
        // usleep(10000);
        timer = fila->processo.vetExec[i].time;
        if (fila->processo.vetExec[i].time > 10000)
        {
          usleep(500000);
          randn = rand() % timer / 2;
        }
        else if (fila->processo.vetExec[i].time >= 5000 && fila->processo.vetExec[i].time < 10000)
        {
          usleep(200000);
          randn = rand() % timer / 2;
        }
        else if (fila->processo.vetExec[i].time >= 100 && fila->processo.vetExec[i].time < 5000)
        {
          usleep(100000);
          randn = rand() % timer / 2;
        }

        if (fila->processo.vetExec[i].time >= 100)
        {
          fila->processo.vetExec[i].time -= randn;
        }
        else
        {
          usleep(100000);
          fila->processo.vetExec[i].time = 0;
        }
      }
    }
    else if (strcmp(fila->processo.vetExec[i].tipo, "print") == 0)
    {
      fila->processo.ponteiro_leitura = i;
      printRequest(diskWin);
    }
    else // É um semáforo
    {
      if (fila->processo.vetExec[i].tipo[0] == 'P') // Encontrou o semáforo P, precisa ocupar
      {

        fila->processo.ponteiro_leitura = i;
        semaphoreP(i, menuWin, outputWin, listWin, processWin);
        // mvwprintw(outputWin, 12, 0, "%d", fila->processo.ponteiro_leitura);
        // i = fila->processo.ponteiro_leitura;
      }
      else if (fila->processo.vetExec[i].tipo[0] == 'V') // Encontrou o semáforo V, precisa desocupar
      {
        fila->processo.ponteiro_leitura = i;
        semaphoreV(i, menuWin, outputWin, waitreinsert, processWin, listWin);
        i = fila->processo.ponteiro_leitura;
        // mvwprintw(outputWin, 11, 1, "Pos arq. block %d", fila->processo.ponteiro_leitura);
        // wrefresh(outputWin);
        // sleep(3);
        // mvwprintw(outputWin, 12, 0, "%d", fila->processo.ponteiro_leitura);
        //  i = fila->processo.ponteiro_leitura;
      }
    }
    // mvwprintw(listWin, 13, 1, "Total Remaining Time: [%d]             ", fila->processo.execTime);
    wrefresh(listWin);
    // usleep(1000);
  }
  clearlines(outputWin, 5, 5, 1);
  mvwprintw(outputWin, 4, 1, "[Executed Processing]                ");
  wrefresh(outputWin);
  sleep(1);
  if (fila->proximo) // Há mais elementos na fila
  {
    fila->proximo->processo.estado = 1;
    processFinish(listWin); // Retirar processo que finalizou da fila e apontar para o próximo
    imprimir(processWin);
  }
  else // Não há mais elementos na fila
  {
    *initSession = 0;
    clearlines(outputWin, 1, 2, 1);
    clearlines(processWin, 1, 5, 0);
    wattron(outputWin, COLOR_PAIR(1));
    wattron(processWin, COLOR_PAIR(1));
    mvwprintw(processWin, 1, 1, "[Finished]");
    mvwprintw(outputWin, 1, 1, "[All processes have been executed]");
    wattroff(processWin, COLOR_PAIR(1));
    wattroff(outputWin, COLOR_PAIR(1));
    wrefresh(processWin);
    wrefresh(outputWin);
    sleep(5);
  }
}

void showLoadingSymbol(WINDOW *menuWin, void *arg2)
{
  const char *symbols[] = {"|", "/", "-", "\\"};
  const int numSymbols = sizeof(symbols) / sizeof(symbols[0]);
  int i = 0;

  LoadThread *args2 = (LoadThread *)arg2;

  // clearlines(menuWin, 1, 10, 0);
  // usleep(10000);
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
  // usleep(10000);
  LoadThread *args2 = (LoadThread *)arg2;
  WINDOW *menuWin = args2->menuWin;
  showLoadingSymbol(menuWin, args2);
}

void *processCreateThread(void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;
  PCB pcb[tamanho];
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
    if (fila)
    {
      usleep(10000);
      if (*initSession != 1) // Se for diferente de 1, é porque acabou a fila ou a thread começou.
      {
        sem_wait(args->initProgram);
      }
      *initSession = 1; // Iniciou sessão
      processar(menuWin, outputWin, pcb, listWin, processWin, filenames, args, initSession, args->insertLower, diskWin, args->diskrun, args->waitreinsert);
    }
    else
    {
      sem_wait(args->emptysession);
    }
  }

  do
  {
    usleep(10000);
    if (*initSession != 1) // Se for diferente de 1, é porque acabou a fila ou a thread começou.
    {
      sem_wait(args->initProgram);
    }
    *initSession = 1; // Iniciou sessão
    processar(menuWin, outputWin, pcb, listWin, processWin, filenames, args, initSession, args->insertLower, diskWin, args->diskrun, args->waitreinsert);
  } while (fila->processo.estado == 1);

  if (fila->processo.estado == 0)
  {
    mvwprintw(menuWin, 10, 1, "                           ");
    mvwprintw(menuWin, 11, 1, "                           ");
    mvwprintw(menuWin, 10, 1, "Remaining time bigger than ");
    mvwprintw(menuWin, 11, 1, "the current process!       ");

    wattron(menuWin, COLOR_PAIR(2));

    mvwprintw(menuWin, 12, 1, "[Aborting process in 3.]   ");
    wrefresh(menuWin);
    sleep(1);
    mvwprintw(menuWin, 12, 1, "[Aborting process in 2..]  ");
    wrefresh(menuWin);
    sleep(1);
    mvwprintw(menuWin, 12, 1, "[Aborting process in 1...] ");
    wrefresh(menuWin);

    wattroff(menuWin, COLOR_PAIR(2));

    sleep(1);
    mvwprintw(menuWin, 10, 1, "                            ");
    mvwprintw(menuWin, 11, 1, "                            ");
    mvwprintw(menuWin, 12, 1, "                            ");
    wrefresh(menuWin);
    usleep(100000);
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
  ///////////////////////////////////////////////////////////
  while (1)
  {
    i = 0;
    if (!fila2)
    {
      sem_wait(args->diskrun);
    }
    while (fila2)
    {
      while (strcmp(fila2->processo.vetExec[fila2->processo.ponteiro_leitura].tipo, "read") == 0 ||
             strcmp(fila2->processo.vetExec[fila2->processo.ponteiro_leitura].tipo, "write") == 0)
      {
        while (i <= fila2->processo.vetExec[fila2->processo.ponteiro_leitura].time)
        {
          mvwprintw(diskWin, 13, 1, "[Moving] Read/Write head");
          mvwprintw(diskWin, 5, 1, "[%s] -> [%d]", fila2->processo.vetExec[fila2->processo.ponteiro_leitura].tipo, i);
          wrefresh(diskWin);
          i++;
          usleep(300000);
        }
        mvwprintw(diskWin, 13, 1, "                          ");
        mvwprintw(diskWin, 5, 1, "                          ");
        wrefresh(diskWin);
        // fila2->processo.execTime -= fila2->processo.vetExec[fila2->processo.ponteiro_leitura].time;
        // usleep(fila2->processo.vetExec[fila2->processo.ponteiro_leitura].time * 1000);
        fila2->processo.ponteiro_leitura++;
      }
      usleep(10000);
      // diskRequest();
      clearlines(diskWin, 2, 13, 0);
      // mvwprintw(diskWin, 2, 1, "                             ");
      usleep(10000);
      wrefresh(diskWin);
      recolocarFila(args->emptysession, args->waitreinsert, args->initProgram, args->initSession);
      imprimir(processWin);
      // diskFinish();
    }
    if (fila2)
    {
      fila2 = fila2->proximo;
      printDisklist(diskWin);
    }
  }
  return NULL;
}
