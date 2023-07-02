#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include <dirent.h>

typedef struct program
{
  char pname[50];  // Nome do programa
  int ID;          // Identificador do Segmento
  int PO;          // Prioridade original
  int segmentSize; // Tamanho do segmento
  float rt;        // Tempo total restante do programa
} program;

typedef struct semaforo
{
  int P; // 0 ou 1
  char nome;
  int existe; // 0 ou 1
  int ID;     // Nome do programa
} Semaforo;

typedef struct vetExec
{
  char tipo[10]; // exec, write, read...
  float time;    // tempo de execução do comando
} vetExec;

typedef struct PCB
{
  int estado; // 1 = rodando, 0 = em espera
  program programa;
  int abort;
  int continuar;
  int ponteiro_leitura;
  vetExec vetExec[100]; // Seria melhor se fosse alocado apenas o necessário
  int execTime;
  int execIndex;
  char filename[20];

  sem_t *nextThreadWait;
  FILE *arquivo;
} PCB;

typedef struct no
{
  PCB processo;
  struct no *proximo;
} No;

typedef struct bloqueados
{
  char semaforo;
  PCB processo;
  struct bloqueados *proximo;
} listaBloqueado;

typedef struct
{
  char **filenames;
  int *initSession;

  WINDOW *menuWin;
  WINDOW *outputWin;
  WINDOW *listWin;
  WINDOW *processWin;
  WINDOW *diskWin;
  WINDOW *logWin;

  sem_t *pauseSemaphore;
  sem_t *initProgram;
  sem_t *insertLower;
  sem_t *diskrun;
  sem_t *emptysession;
  sem_t *waitreinsert;
  sem_t *waitprocessing;
} ThreadArgs;

typedef struct
{
  WINDOW *menuWin;
  sem_t *loadingSymbol;
} LoadThread;

extern Semaforo semaforos[5];

extern No *fila, *fila2;
extern listaBloqueado *bloqueados;

extern int paused, ScheduleProcess, auxScheduleProcess;
extern int finishedLoad, reinsert, startedprocessing;

void processar(WINDOW *menuWin, WINDOW *outputWin, PCB *pcb, WINDOW *listWin, WINDOW *processWin, char **filenames, void *arg, int *initSession, sem_t *insertLower, WINDOW *diskWin, sem_t *diskrun, sem_t *waitreinsert);

void simulateTime(PCB *pcb, int op1);

void clearTerminal();

void clearlines(WINDOW *window, int first, int last, int size);

void processInterrupt(PCB *pcb, int op1);

void semaphoreP(int i, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin);

void semaphoreV(int i, WINDOW *menuWin, WINDOW *outputWin, sem_t *waitreinsert, WINDOW *processWin, WINDOW *listWin);

void memLoadReq(PCB *pcb, int op1, WINDOW *outputWin, WINDOW *listWin, WINDOW *logWin);

void memLoadFinish();

int processCreate(FILE *instr, PCB *pcb, int op1, WINDOW *menuWin, WINDOW *outputWin, WINDOW *listWin, WINDOW *processWin, char **filenames, sem_t *insertLower, WINDOW *logWin, int *initSession, sem_t *emptysession, sem_t *waitreinsert, sem_t *waitprocessing);

int existenciaSemaforo(char verificarSemaforo);

void processFinish(WINDOW *listWin);

void imprimir(WINDOW *processWin);

void showLoadingSymbol(WINDOW *menuWin, void *arg);

void loadingBar(WINDOW *outputWin, int sizebar, int iteration);

int colocarDisco(WINDOW *processWin, WINDOW *diskWin, sem_t *diskrun, int *initSession);

void simpleProcessFinishDisk();

void simpleProcessFinish(WINDOW *listWin);

int recolocarFila(sem_t *emptysessio, sem_t *waitreinsert, sem_t *initProgram, int *initSession, sem_t *waitprocessing);

void deleteBlocked(char remover);

void printRequest();

void printFinish(WINDOW *diskWi);

void printDisklist(WINDOW *diskWin);

void printDisklist(WINDOW *diskWin);

void printBlocked(WINDOW *listWin);

void *loadingThread(void *arg2);

void *processCreateThread(void *arg);

void *diskThread(void *arg);