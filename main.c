#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "funcoes.h"
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include <dirent.h>

#define tamanho 1000

typedef struct
{
  FILE *instr;
  int op1;
  char **filenames;

  WINDOW *menuWin;
  WINDOW *outputWin;
  WINDOW *listWin;
  WINDOW *processWin;

  sem_t *printSemaphore;
  sem_t *pauseSemaphore;
  sem_t *nextThread;
  pthread_mutex_t *printMutex;
} ThreadArgs;

typedef struct
{
  WINDOW *menuWin;
  sem_t *loadingSymbol;
} LoadThread;

No *r, *fila = NULL;

int processCreate(FILE *instr, PCB *pcb, int op1, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin, char **filenames, void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;

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
    ///////////
    novo->processo.execIndex = 0;
    novo->processo.execTime = 0;
    novo->processo.estado = 1;

    rewind(instr);

    fgets(novo->processo.programa.pname, sizeof(novo->processo.programa.pname), instr);
    fscanf(instr, "%d", &novo->processo.programa.ID);
    fscanf(instr, "%d", &novo->processo.programa.PO);
    fscanf(instr, "%d", &novo->processo.programa.segmentSize);

    strcpy(novo->processo.filename, filenames[op1]);

    memLoadReq(pcb, op1, outputWin, listWin);

    mvwprintw(outputWin, 2, 1, "[Sucessful Loaded Process]    ");
    wrefresh(outputWin);
    sleep(3); // aqui
    fseek(instr, 1, SEEK_CUR);

    char c;
    int f = 0; // Aqui ta errado, ele ta sobrescrevendo os semaforos de cada processo novo
    while (c != '\n')
    {
      fscanf(instr, "%c", &verificarSemaforo); // ate aqui ta certo
      strcpy(semaforos[f].nome, &verificarSemaforo);
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
      usleep(100000);
      mvwprintw(listWin, 2, 1, "                                      ");
      mvwprintw(listWin, 3, 1, "                                      ");
      wrefresh(listWin);

      if (strcmp(tipo, "P(s)") == 0)
      {
        time = 200;
        // semaphoreP(semaforos, 0, 's');
      }
      else if (strcmp(tipo, "P(t)") == 0)
      {
        time = 200;
        // semaphoreP(semaforos, 0, 's');
      }
      else if ((strcmp(tipo, "V(t)") == 0))
      {
        // semaphoreV(&semaforos, 't');
        time = 0;
      }
      else if (strcmp(tipo, "V(s)") == 0)
      {
        // semaphoreV(&semaforos, 't');
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
    mvwprintw(outputWin, 5, 1, "[Completed Reading. Starting Process...]");
    wrefresh(outputWin); // ...
    sleep(2);
    mvwprintw(outputWin, 5, 1, "                                              ");
    wrefresh(outputWin);

    ///////////

    // execucao
    /* for (int i = 0; i < novo->processo.execIndex; i++)
    {
      novo->processo.vetExec[i].time = num[op1].vetExec[i].time;
      strcpy(novo->processo.vetExec[i].tipo, num[op1].vetExec[i].tipo);
    } */

    novo->proximo = NULL;
    if (fila == NULL)
    {
      fila = malloc(sizeof(No));
      novo->processo.estado = 1;
      fila = novo;
      fclose(novo->processo.arquivo);
      finishedLoad = 0;
      return 1;
    }
    else
    {
      auxScheduleProcess++;
      // sem_init(args->nextThreadWait[auxScheduleProcess], 0, 0);
      novo->processo.nextThreadWait = (sem_t *)malloc(sizeof(sem_t));
      sem_init(novo->processo.nextThreadWait, 0, 0);
      if ((fila)->processo.execTime > novo->processo.execTime) //
      {
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
        // fila = novo; <- ISSO AQUI
        imprimir(processWin);
        finishedLoad = 0;
        sem_wait(aux->proximo->processo.nextThreadWait);
        return 0; // Processo entrou na fila
      }
    }
  }
  /////////////////////////////////////////////////////////
}

void semaphoreP(int i, WINDOW *menuWin, WINDOW *outputWin)
{
  char stringaux[10];
  /// Verifica se o semáforo está ocupado ou não para inciar
  for (int t = 0; t < 5; t++) // Verifica se o semáforo está sendo utilizado,
                              // percorrendo o vetor de semáforos
  {
    stringaux[0] = fila->processo.vetExec[i].tipo[2];
    if ((stringaux[0] == semaforos[t].nome[0]) && semaforos[t].P == 0) // Se entrar nesse if, encontrou o semáforo e ele não está em uso
    {                                                                  // passei o paramaetro semaforos corretamente?
                                                                       // printf("Entrou no if que compara se strinaux é igual a semaforosnome, t = %d\n", t);
      // sleep(3);
      semaforos[t].P = 1;
      usleep(100000);
      mvwprintw(outputWin, 1, 1, "Execution number %d                 ", i);
      mvwprintw(outputWin, 2, 1, "                                             ");
      wattron(outputWin, COLOR_PAIR(1));
      mvwprintw(outputWin, 2, 1, "Semaphore [%c]: Enabled                       ", fila->processo.vetExec[i].tipo[2]);
      wattroff(outputWin, COLOR_PAIR(1));
      wrefresh(outputWin);

      sleep(3);
      break; // Sai do for...
    }
  }
}

void semaphoreV(int i, WINDOW *menuWin, WINDOW *outputWin)
{
  char stringaux[10];
  /// Desocupa o semáforo ocupado correspondente à posição i
  for (int t = 0; t < 5; t++) // Percorre o vetor de semáforos
  {
    stringaux[0] = fila->processo.vetExec[i].tipo[2];
    if ((stringaux[0] == semaforos[t].nome[0]) && semaforos[t].P == 1) // Desocupa esse semáforo setando como 0!!
    {
      semaforos[t].P = 0;
      usleep(100000);
      mvwprintw(outputWin, 1, 1, "Execution number %d                          ", i);
      mvwprintw(outputWin, 2, 1, "                                             ");
      wattron(outputWin, COLOR_PAIR(2));
      mvwprintw(outputWin, 2, 1, "Semaphore [%c]: Disabled                     ", fila->processo.vetExec[i].tipo[2]);
      wattroff(outputWin, COLOR_PAIR(2));

      wrefresh(outputWin);

      sleep(3);
      break;
    }
  }
}

void processar(WINDOW *menuWin, WINDOW *outputWin, PCB *pcb, WINDOW *listWin, WINDOW *processWin, char **filenames, void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;

  char stringaux[10];
  int k = 0;
  int opPaused = -1;

  srand(time(NULL));
  for (int i = 0; i < fila->processo.execIndex; i++)
  { // Percorrer cada comando
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
      fflush(stdout);
      // Se a função foi despausada, atualiza a mensagem e continua a execução
      clearlines(menuWin, 1, 2, 0);
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
  clearlines(outputWin, 5, 5, 1);
  // mvwprintw(outputWin, 5, 1, "                                     ");
  mvwprintw(outputWin, 4, 1, "[Executed Processing]                ");
  wrefresh(outputWin);
  usleep(100000);
  if (fila->proximo) // ainda nao é o ultimo
  {
    fila->proximo->processo.estado = 1;
    processFinish(listWin); // tira o processo da fila
    imprimir(processWin);
    /* mvwprintw(processWin, 6, 1, "passou do process finish");
    wrefresh(processWin);
    sleep(5); */

    // DESPAUSAR SEMAFORO DA THREAD Q TA ESPERANDO TERMINAR
    ScheduleProcess++;
    sem_post(args->nextThread);
    // sem_post(args->nextThreadWait[ScheduleProcess]);
    sem_post(fila->processo.nextThreadWait);
    // processar(menuWin, outputWin, pcb, listWin, processWin, filenames, args);
    // processCreate(fila->proximo->processo.arquivo, pcb, 0, menuWin, outputWin, listWin, processWin, filenames, args);
  }
  else // fila esta vazia
  {
    // wclear(outputWin);
    // box(outputWin, 1, 1);
    clearlines(outputWin, 1, 2, 1);
    wattron(outputWin, COLOR_PAIR(1));
    mvwprintw(outputWin, 1, 1, "[All processes have been executed]");
    wattroff(outputWin, COLOR_PAIR(1));
    wrefresh(outputWin);
    ////// variavel que vai zerar
    ScheduleProcess = 0;
    sleep(5);
  }
}

void showLoadingSymbol(WINDOW *menuWin, void *arg2)
{
  const char *symbols[] = {"|", "/", "-", "\\"};
  const int numSymbols = sizeof(symbols) / sizeof(symbols[0]);
  int i = 0;

  LoadThread *args2 = (LoadThread *)arg2;

  clearlines(menuWin, 1, 10, 0);

  while (1)
  {
    mvwprintw(menuWin, 4, 1, "Loading %s", symbols[i]);
    wrefresh(menuWin);
    // fflush(stdout);
    i = (i + 1) % numSymbols;
    usleep(100000); // Ajuste o tempo de espera conforme necessário

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
  FILE *instr = args->instr;
  int op1 = args->op1;
  PCB pcb[tamanho];
  WINDOW *menuWin = args->menuWin;
  WINDOW *outputWin = args->outputWin;
  WINDOW *listWin = args->listWin;
  WINDOW *processWin = args->processWin;
  char **filenames = args->filenames;

  mvwprintw(outputWin, 1, 1, "Loading process...");
  wrefresh(outputWin);
  sleep(1);

  if (processCreate(instr, pcb, op1, menuWin, outputWin, listWin, processWin, filenames, args) == 1)
  {
    if (fila->proximo)
    {                                     // vai ter que parar um processo
      fila->proximo->processo.estado = 0; // o erro ta acontecendo aqui por causa de ponteiro eu acho
      // sem_wait(args->nextThreadWait[ScheduleProcess]);
    }
  }
  sem_init(args->nextThread, 0, 0);

  imprimir(processWin);
  // clearlines(processWin, 2, 5, 0);
  // sleep(5);
  /////////////// processar programa /////////////////
  usleep(100000);
  // if (aux->proximo)
  // sem_wait(fila->proximo->processo.nextThreadWait);
  if (fila->processo.estado == 1)
  {
    processar(menuWin, outputWin, pcb, listWin, processWin, filenames, args);
  }
  else if (fila->processo.estado == 0)
  { // PAUSAR PROCESSO ATÉ QUE POSSUA O MENOR TEMPO
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

  sem_post(args->printSemaphore);

  return NULL;
}

int paused = 0;
int ScheduleProcess = 0;
int auxScheduleProcess = 0;
int finishedLoad = 0;
#define MAX_FILES 6

Semaforo semaforos[5];

int main()
{
  char linha[100];
  // Aplicativos
  FILE **instr = NULL;
  int i, file_count = 0;
  char **filenames = NULL;

  DIR *dir;

  struct dirent *ent;
  dir = opendir("."); // Abre o diretório atual

  if (dir != NULL)
  {

    while ((ent = readdir(dir)) != NULL)
    {
      char *filename = ent->d_name;
      if (strstr(filename, ".sint") != NULL)
      {
        FILE *file = fopen(filename, "r");
        if (file != NULL)
        {
          instr = realloc(instr, (file_count + 1) * sizeof(FILE *));
          instr[file_count] = file;

          filenames = realloc(filenames, (file_count + 1) * sizeof(char *));
          filenames[file_count] = strdup(filename);

          file_count++;
        }
      }
    }
    closedir(dir);
  }

  // Inicialização da biblioteca ncurses
  initscr();
  start_color();
  cbreak();
  echo();

  if (has_colors())
  {
    // Ative o uso de pares de cores
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -2);
  }

  // Calcula as dimensões e posições das janelas
  int menuWidth = COLS / 4; // Largura da janela do menu
  int menuHeight = LINES;   // Altura da janela do menu
  int menuX = 0;            // Posição X da janela do menu
  int menuY = 0;            // Posição Y da janela do menu

  int processWidth = COLS / 4;
  int processHeight = LINES;
  int processX = COLS / 4;
  int processY = 0;

  int outputWidth = COLS / 2;
  int outputHeight = LINES / 2;
  int outputX = COLS / 2;
  int outputY = 0;

  int thirdWidth = COLS / 2;
  int thirdHeight = LINES / 2;
  int thirdX = COLS / 2;
  int thirdY = LINES / 2;

  WINDOW *menuWin = newwin(menuHeight, menuWidth, menuY, menuX);
  WINDOW *outputWin = newwin(outputHeight, outputWidth, outputY, outputX);
  WINDOW *listWin = newwin(thirdHeight, thirdWidth, thirdY, thirdX);
  WINDOW *processWin = newwin(processHeight, processWidth, processY, processX);

  // Verifica se as janelas foram criadas corretamente
  if (menuWin == NULL || outputWin == NULL || listWin == NULL || processWin == NULL)
  {
    endwin();
    wattron(menuWin, COLOR_PAIR(2));
    printf("Error creating windows\n");
    wattroff(menuWin, COLOR_PAIR(2));
    return 1;
  }

  // Atualiza as janelas
  refresh();
  wrefresh(menuWin);
  wrefresh(outputWin);
  wrefresh(listWin);
  wrefresh(processWin);

  PCB pcb[tamanho];
  int op = -1;
  int op1 = 1;
  int indexFile = 0;
  char FileName[25];
  pcb->execIndex = 0;

  pthread_t processthread;
  pthread_t loadSymbThread;

  int threadResult;
  int loadsymresult;

  for (int i = 0; i < 5; i++)
    semaforos[i].existe = 0;

  if (instr[0] == NULL || instr[1] == NULL || instr[2] == NULL || instr[3] == NULL || instr[4] == NULL)
  {
    wclear(menuWin);
    box(menuWin, 0, 0);
    wattron(menuWin, COLOR_PAIR(2));
    mvwprintw(menuWin, 1, 1, "Error: file not found");
    wattroff(menuWin, COLOR_PAIR(2));
    wrefresh(menuWin);
    sleep(5);
    return 1;
  }

  // Crie o semáforo de impressão
  sem_t printSemaphore;
  sem_t pauseSemaphore;
  sem_t nextThread;
  sem_t loadingSymbol;
  sem_init(&printSemaphore, 0, 1);

  // Crie o mutex para sincronização da impressão
  pthread_mutex_t printMutex;
  pthread_mutex_init(&printMutex, NULL);

  FILE *selectedFile = NULL;

  // MENU
  int it = 0;
  wclear(menuWin);
  wclear(outputWin);
  wclear(listWin);
  wclear(processWin);

  box(menuWin, 0, 0);
  box(outputWin, 0, 0);
  box(listWin, 0, 0);
  box(processWin, 0, 0);

  wrefresh(outputWin);
  wrefresh(listWin);
  wrefresh(menuWin);
  wrefresh(processWin);
  do
  {
    op = -1;
    while (paused == 1)
    {
      // usleep(10000);
      sem_wait(&pauseSemaphore);
    }

    if (finishedLoad == 1)
    {
      sem_wait(&loadingSymbol);
    }

    mvwprintw(menuWin, 1, 1, "[0] Start process       ");
    mvwprintw(menuWin, 2, 1, "[1] Pause process       ");
    mvwprintw(menuWin, 3, 1, "[2] End simulation      ");
    clearlines(menuWin, 4, 7, 0);

    mvwprintw(processWin, 1, 1, "List of tasks:");

    clearlines(listWin, 1, 2, 1);

    wrefresh(outputWin);
    wrefresh(listWin);
    wrefresh(menuWin);
    wrefresh(processWin);
    // Lê a opção do usuário
    mvwprintw(menuWin, 4, 1, "Choose an option: ");
    wscanw(menuWin, " %d", &op);

    fflush(stdout);

    if (op == 0)
    { // Rodar processo
      fflush(stdout);
      wattron(menuWin, COLOR_PAIR(1));
      mvwprintw(menuWin, 1, 1, "Available processes:    ");
      wattroff(menuWin, COLOR_PAIR(1));

      for (i = 0; i < file_count; i++)
      {
        mvwprintw(menuWin, i + 2, 1, "[%d] %s    ", i, filenames[i]);
      }

      mvwprintw(menuWin, file_count + 2, 1, "Select an option: ");

      flushinp();

      wrefresh(outputWin);
      wrefresh(listWin);
      wrefresh(menuWin);

      wscanw(menuWin, " %d", &op1);
      clearlines(menuWin, 1, 9, 0);

      if (op1 >= 0 && op1 <= 4)
      {
        selectedFile = instr[op1];

        fflush(stdout);
        // Criação da estrutura de argumentos da thread
        ThreadArgs threadArgs;
        threadArgs.instr = selectedFile;
        threadArgs.op1 = op1;
        threadArgs.filenames = filenames;

        threadArgs.menuWin = menuWin;

        threadArgs.outputWin = outputWin;
        threadArgs.listWin = listWin;
        threadArgs.processWin = processWin;

        threadArgs.printMutex = &printMutex;
        threadArgs.printSemaphore = &printSemaphore;
        threadArgs.pauseSemaphore = &pauseSemaphore;
        threadArgs.nextThread = &nextThread;

        sem_init(&loadingSymbol, 0, 0);

        LoadThread loadthread;
        loadthread.menuWin = menuWin;
        loadthread.loadingSymbol = &loadingSymbol;

        finishedLoad = 1;

        // Criação da thread para a função processCreate
        loadsymresult = pthread_create(&loadSymbThread, NULL, loadingThread, (void *)&loadthread);
        threadResult = pthread_create(&processthread, NULL, processCreateThread, (void *)&threadArgs);

        sleep(1);
        if (threadResult != 0)
        {
          wclear(menuWin);
          mvwprintw(menuWin, 1, 1, "Error creating thread for processCreate function.");
          wrefresh(menuWin);
          return 1;
        }
      }
    }
    else if (op == 1)
    {
      paused = 1;
      sem_init(&pauseSemaphore, 0, 0);
    }
    else if (op == 2)
    {
      pthread_cancel(processthread);
      wclear(menuWin);
      wclear(outputWin);
      wclear(listWin);
      endwin();
      clearTerminal();
    }
  } while (op != 2);

  // Aguarde a conclusão da thread antes de encerrar o programa
  pthread_join(loadSymbThread, NULL);
  pthread_join(processthread, NULL);

  sem_destroy(&printSemaphore);
  pthread_mutex_destroy(&printMutex);

  // Feche os arquivos após a conclusão do programa
  for (i = 0; i < 5; i++)
  {
    if (instr[i] != NULL)
    {
      fclose(instr[i]);
    }
  }

  // free(threadArgs.nextThreadWait);
  return 0;
}
