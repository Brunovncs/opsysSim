#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "funcoes.h"
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include <dirent.h>

No *r, *cpuQueue, *diskQueue = NULL;
listaBloqueado *bloqueados = NULL;

int paused = 0;
int finishedLoad = 0;
int reinsert = 0;
int startedprocessing = 0;
#define MAX_FILES 6

Semaforo semaforos[5];

int main()
{
  char linha[100];
  // Aplicativos
  FILE **instr = NULL;
  int i, file_count = 0;
  char **filenames = NULL;
  int initSession = 0;

  DIR *dir;

  struct dirent *ent;
  dir = opendir("./programs");

  if (dir != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      char *filename = ent->d_name;
      if (strstr(filename, ".sint") != NULL || strstr(filename, ".txt") != NULL || strstr(filename, ".dat") != NULL)
      {
        // Construa o caminho completo para o arquivo
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "./programs/%s", filename);
        
        FILE *file = fopen(filepath, "r");
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
    init_color(COLOR_WHITE, 900, 1000, 900);
    init_pair(3, COLOR_WHITE, -3);
  }

  // Calcula as dimensões e posições das janelas
  int menuWidth = COLS / 4, menuHeight = LINES / 2, menuX = 0, menuY = 0;
  int logWidth = COLS / 4, logHeight = LINES / 2, logX = 0, logY = LINES / 2;

  int processWidth = COLS / 4, processHeight = LINES / 2, processX = COLS / 4, processY = 0;
  int diskWidth = COLS / 4, diskHeight = LINES / 2, diskX = COLS / 4, diskY = LINES / 2;

  int outputWidth = COLS / 2, outputHeight = LINES / 2, outputX = COLS / 2, outputY = 0;
  int thirdWidth = COLS / 2, thirdHeight = LINES / 2, thirdX = COLS / 2, thirdY = LINES / 2;

  WINDOW *menuWin = newwin(menuHeight, menuWidth, menuY, menuX);
  WINDOW *outputWin = newwin(outputHeight, outputWidth, outputY, outputX);
  WINDOW *listWin = newwin(thirdHeight, thirdWidth, thirdY, thirdX);
  WINDOW *processWin = newwin(processHeight, processWidth, processY, processX);
  WINDOW *diskWin = newwin(diskHeight, diskWidth, diskY, diskX);
  WINDOW *logWin = newwin(logHeight, logWidth, logY, logX);

  // Verifica se as janelas foram criadas corretamente
  if (menuWin == NULL || outputWin == NULL || listWin == NULL || processWin == NULL || diskWin == NULL)
  {
    endwin();
    wattron(menuWin, COLOR_PAIR(2));
    printf("Error creating windows\n");
    wattroff(menuWin, COLOR_PAIR(2));
    return 1;
  }

  // Atualiza as janelas
  refresh();

  PCB* pcb = (PCB*)malloc(file_count * sizeof(PCB));

  int op = -1;
  int op1 = 1;
  int indexFile = 0;
  char FileName[25];
  pcb->execIndex = 0;

  pthread_t processthread;
  pthread_t loadSymbThread;
  pthread_t diskthread;

  int threadResult;
  int loadsymresult;
  int diskthreadres;

  for (int i = 0; i < 5; i++)
    semaforos[i].existe = 0;

    if (file_count == 0)
    {
      wclear(menuWin);
      box(menuWin, 0, 0);
      wattron(menuWin, COLOR_PAIR(2));
      mvwprintw(menuWin, 1, 1, "Error: No available programs to run");
      wattroff(menuWin, COLOR_PAIR(2));
      wrefresh(menuWin);
      sleep(5);
      return 1;
    }

  // Criação de semáforos
  sem_t pauseSemaphore;
  sem_t loadingSymbol;
  sem_t initProgram;
  sem_t insertLower;
  sem_t diskrun;
  sem_t emptysession;
  sem_t waitreinsert;
  sem_t waitprocessing;

  // MENU
  int it = 0;
  clear();

  wbkgd(menuWin, COLOR_PAIR(3));
  wbkgd(outputWin, COLOR_PAIR(3));
  wbkgd(listWin, COLOR_PAIR(3));
  wbkgd(processWin, COLOR_PAIR(3));
  wbkgd(diskWin, COLOR_PAIR(3));
  wbkgd(logWin, COLOR_PAIR(3)); 

  box(menuWin, 0, 0);
  box(outputWin, 0, 0);
  box(listWin, 0, 0);
  box(processWin, 0, 0);
  box(diskWin, 0, 0);
  box(logWin, 0, 0);

  wrefresh(outputWin);
  wrefresh(listWin);
  wrefresh(menuWin);
  wrefresh(processWin);
  wrefresh(diskWin);
  wrefresh(logWin);

  sem_init(&initProgram, 0, 0);
  sem_init(&insertLower, 0, 0);
  sem_init(&diskrun, 0, 0);
  sem_init(&emptysession, 0, 0);
  sem_init(&waitreinsert, 0, 0);
  sem_init(&waitprocessing, 0, 0);

  ThreadArgs threadArgs;

  threadArgs.filenames = filenames;

  threadArgs.menuWin = menuWin;
  threadArgs.outputWin = outputWin;
  threadArgs.listWin = listWin;
  threadArgs.processWin = processWin;
  threadArgs.diskWin = diskWin;
  threadArgs.logWin = logWin;

  threadArgs.pauseSemaphore = &pauseSemaphore;
  threadArgs.initProgram = &initProgram;
  threadArgs.insertLower = &insertLower;
  threadArgs.diskrun = &diskrun;
  threadArgs.emptysession = &emptysession;
  threadArgs.waitreinsert = &waitreinsert;
  threadArgs.waitprocessing = &waitprocessing;

  threadArgs.initSession = &initSession;

  threadResult = pthread_create(&processthread, NULL, processCreateThread, (void *)&threadArgs);
  diskthreadres = pthread_create(&processthread, NULL, diskThread, (void *)&threadArgs);

  do
  {
    op = -1;
    while (paused == 1)
    {
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

    mvwprintw(processWin, 1, 1, "[CPU usage]");

    mvwprintw(diskWin, 1, 1, "[Disk usage]");

    mvwprintw(logWin, 1, 1, "[Log Window]");

    mvwprintw(listWin, 1, 1, "[Command Window]");
    
    mvwprintw(listWin, 5, 1, "[Locked Processes]");

    mvwprintw(outputWin, 1, 1, "[Processes Window]                      ");

    clearlines(listWin, 2, 2, 1);

    wrefresh(outputWin);
    wrefresh(listWin);
    wrefresh(menuWin);
    wrefresh(processWin);
    wrefresh(diskWin);
    wrefresh(logWin);

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

      if (op1 >= 0 && op1 <= file_count)
      {
        sem_init(&loadingSymbol, 0, 0);

        LoadThread loadthread;
        loadthread.menuWin = menuWin;
        loadthread.loadingSymbol = &loadingSymbol;

        finishedLoad = 1;

        usleep(100000);

        // Criação da thread para a função processCreate
        loadsymresult = pthread_create(&loadSymbThread, NULL, loadingThread, (void *)&loadthread);

        mvwprintw(logWin, 2, 1, "Loading process...");
        wrefresh(logWin);
        sleep(1);
        mvwprintw(logWin, 2, 1, "                   ");
        wrefresh(logWin);

        if (processCreate(instr[op1], pcb, op1, menuWin, outputWin, listWin, processWin, filenames, &insertLower, logWin, &initSession, &emptysession, &waitreinsert, &waitprocessing) == 1)
        {
          sem_post(&initProgram);
          imprimir(processWin);
          if (cpuQueue->proximo)
          {
            cpuQueue->proximo->processo.estado = 0;
          }
        }

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