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
  sem_t *printSemaphore;
  pthread_mutex_t *printMutex;
  WINDOW *menuWin;
  WINDOW *outputWin;
  WINDOW *listWin;
} ThreadArgs;

No *r, *fila = NULL;

void processCreate(FILE *instr, PCB *pcb, int op1, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin)
{
  char verificarSemaforo;
  char execucao[50];
  int tempo;
  int posVetExec = 0;
  char linha[100];
  int foundEmptyLine = 0;
  pcb[op1].execIndex = 0;
  pcb[op1].execTime = 0;
  pcb[op1].estado = 1;
  fflush(stdout);
  // wclear(outputWin);
  // mvwprintw(outputWin, 1, 1, "Carregando o programa...");
  // wrefresh(outputWin);

  // Ler até a linha vazia
  rewind(instr);
  fgets(pcb[op1].programa.pname, sizeof(pcb[op1].programa.pname), instr);
  fscanf(instr, "%d", &pcb[op1].programa.ID);
  fscanf(instr, "%d", &pcb[op1].programa.PO);
  fscanf(instr, "%d", &pcb[op1].programa.segmentSize);
  memLoadReq(pcb, op1, outputWin, listWin);

  mvwprintw(outputWin, 2, 1, "[Sucessful Loaded Process]    ");
  wrefresh(outputWin);
  sleep(3); // aqui
  fseek(instr, 1, SEEK_CUR);
  char c;
  int f = 0;
  while (c != '\n')
  {
    fscanf(instr, "%c", &verificarSemaforo); // ate aqui ta certo
    strcpy(semaforos[f].nome, &verificarSemaforo);
    semaforos[f].existe = 1;
    semaforos[f].P = 0;
    c = fgetc(instr);
    f++;
  }
  // printf("Semáforo 1: %s\n", semaforos[0].nome);
  // printf("Semáforo 2: %s\n", semaforos[1].nome);

  // Ler os comandos "exec" e seus tempos correspondentes
  char tipo[10];
  int time;
  int tempototal = 0;
  while (!feof(instr))
  {
    fscanf(instr, "%s %d", tipo, &time);

    mvwprintw(listWin, 2, 1, "Reading: [%s] %d      ", tipo, time);
    mvwprintw(listWin, 3, 1, "Total Time: [%d]                 ", tempototal);
    wrefresh(listWin); // ...
    usleep(500000);
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
    strcpy(pcb[op1].vetExec[pcb[op1].execIndex].tipo, tipo);
    pcb[op1].vetExec[pcb[op1].execIndex].time = time;
    tempototal += time;
    pcb[op1].execIndex++;
    // printf("Fim\n");
  }

  mvwprintw(outputWin, 5, 1, "[Completed Reading. Starting Process...]");
  wrefresh(outputWin); // ...
  sleep(2);
  mvwprintw(outputWin, 5, 1, "                                              ");
  wrefresh(outputWin);
  pcb[op1].execTime = tempototal;

  if (inserir_com_prioridade(pcb, op1, instr, menuWin) == 1) // o que foi inserido tem mais prioridade
  {
    if (fila->proximo)
    {                                     // vai ter que parar um processo
      fila->proximo->processo.estado = 0; // o erro ta acontecendo aqui por causa de ponteiro eu acho
    }
  }
  // imprimir(fila);
  // sleep(5);
  /////////////// processar programa /////////////////
  if (fila->processo.estado == 1)
  {
    processar(menuWin, outputWin, pcb, listWin);
  }
  else if (fila->processo.estado == 0)
  {
    mvwprintw(menuWin, 10, 1, "                                                          ");
    mvwprintw(menuWin, 10, 1, "Remaining time bigger than the current process!           ");

    wattron(menuWin, COLOR_PAIR(2));

    mvwprintw(menuWin, 11, 1, "[Aborting process in 3.]                                  ");
    wrefresh(menuWin);
    sleep(1);
    mvwprintw(menuWin, 11, 1, "[Aborting process in 2..]                                 ");
    wrefresh(menuWin);
    sleep(1);
    mvwprintw(menuWin, 11, 1, "[Aborting process in 1...]                                ");
    wrefresh(menuWin);

    wattroff(menuWin, COLOR_PAIR(2));

    sleep(1);
    mvwprintw(menuWin, 10, 1, "                                                          ");
    mvwprintw(menuWin, 11, 1, "                                                          ");
    wrefresh(menuWin);
    usleep(100000);
  }
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

void *processCreateThread(void *arg)
{
  ThreadArgs *args = (ThreadArgs *)arg;
  FILE *instr = args->instr;
  int op1 = args->op1;
  PCB pcb[tamanho];
  WINDOW *menuWin = args->menuWin;
  WINDOW *outputWin = args->outputWin;
  WINDOW *listWin = args->listWin;

  mvwprintw(outputWin, 1, 1, "Loading process...");
  wrefresh(outputWin);
  sleep(1);

  processCreate(instr, pcb, op1, menuWin, outputWin, listWin);

  sem_post(args->printSemaphore);

  return NULL;
}

int paused = 0;
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
  int menuWidth = COLS / 2; // Largura da janela do menu
  int menuHeight = LINES;   // Altura da janela do menu
  int menuX = 0;            // Posição X da janela do menu
  int menuY = 0;            // Posição Y da janela do menu

  int outputWidth = COLS / 2;   // Largura da janela de saída
  int outputHeight = LINES / 2; // Altura da janela de saída
  int outputX = COLS / 2;       // Posição X da janela de saída
  int outputY = 0;              // Posição Y da janela de saída

  int thirdWidth = COLS / 2;   // Largura da terceira janela
  int thirdHeight = LINES / 2; // Altura da terceira janela
  int thirdX = COLS / 2;       // Posição X da terceira janela
  int thirdY = LINES / 2;      // Posição Y da terceira janela

  WINDOW *menuWin = newwin(menuHeight, menuWidth, menuY, menuX);
  WINDOW *outputWin = newwin(outputHeight, outputWidth, outputY, outputX);
  WINDOW *listWin = newwin(thirdHeight, thirdWidth, thirdY, thirdX); // Criação da terceira janela

  // Verifica se as janelas foram criadas corretamente
  if (menuWin == NULL || outputWin == NULL || listWin == NULL)
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

  PCB pcb[tamanho];
  int op = -1;
  int op1 = 1;
  int indexFile = 0;
  char FileName[25];
  pcb->execIndex = 0;

  pthread_t processthread;
  pthread_t testthread;

  int threadResult;
  int testresult;

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

  box(menuWin, 0, 0);
  box(outputWin, 0, 0);
  box(listWin, 0, 0);

  wrefresh(outputWin);
  wrefresh(listWin);
  wrefresh(menuWin);
  do
  {
    op = -1;
    while (paused == 1)
    {
      usleep(10000);
    }
    mvwprintw(menuWin, 1, 1, "[0] Start process                      ");

    mvwprintw(menuWin, 2, 1, "[1] Pause process                      ");
    mvwprintw(menuWin, 3, 1, "[2] End simulation                     ");
    mvwprintw(menuWin, 4, 1, "                                       ");
    mvwprintw(menuWin, 5, 1, "                                       ");
    mvwprintw(menuWin, 6, 1, "                                       ");
    mvwprintw(menuWin, 7, 1, "                                       ");

    mvwprintw(listWin, 1, 1, "                             ");
    mvwprintw(listWin, 2, 1, "                             ");

    wrefresh(outputWin);
    wrefresh(listWin);
    wrefresh(menuWin);
    // Lê a opção do usuário
    mvwprintw(menuWin, 4, 1, "Choose an option: ");
    wscanw(menuWin, " %d", &op);

    fflush(stdout);

    if (op == 0)
    { // Rodar processo
      fflush(stdout);
      wattron(menuWin, COLOR_PAIR(1));
      mvwprintw(menuWin, 1, 1, "Available processes:          ");
      wattroff(menuWin, COLOR_PAIR(1));

      for (i = 0; i < file_count; i++)
      {
        mvwprintw(menuWin, i + 2, 1, "[%d] %s             ", i, filenames[i]);
      }

      mvwprintw(menuWin, file_count + 2, 1, "Select an option: ");

      flushinp();

      wrefresh(outputWin);
      wrefresh(listWin);
      wrefresh(menuWin);

      wscanw(menuWin, " %d", &op1);
      mvwprintw(menuWin, 7, 1, "                                ");

      if (op1 >= 0 && op1 <= 4)
      {
        selectedFile = instr[op1];

        fflush(stdout);
        // Criação da estrutura de argumentos da thread
        ThreadArgs threadArgs;
        threadArgs.instr = selectedFile;
        threadArgs.op1 = op1;
        threadArgs.printSemaphore = &printSemaphore;
        threadArgs.printMutex = &printMutex;
        threadArgs.menuWin = menuWin;
        threadArgs.outputWin = outputWin;
        threadArgs.listWin = listWin;

        // Criação da thread para a função processCreate
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

  return 0;
}
