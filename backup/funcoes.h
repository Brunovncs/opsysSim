#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

typedef struct program {
  char pname[50];  // Nome do programa
  int ID;          // Identificador do Segmento
  int PO;          // Prioridade original
  int segmentSize; // Tamanho do segmento
  float rt;          // Tempo total restante do programa
} program;

typedef struct semaforo {
  int P; // 0 ou 1
  char nome[1];
  int existe; // 0 ou 1
  int ID;     // Nome do programa
} Semaforo;

typedef struct vetExec {
  char tipo[10]; // exec, write, read...
  float time;      // tempo de execução do comando
} vetExec;

typedef struct PCB {
  int estado; // 1 = rodando, 0 = em espera
  program programa;
  vetExec vetExec[100]; // Seria melhor se fosse alocado apenas o necessário
  int execTime;
  int execIndex;
  FILE *arquivo;
} PCB;

typedef struct no {
  PCB processo;
  struct no *proximo;
} No;

extern Semaforo semaforos[5];

extern No *fila;

extern int paused;

void processar(WINDOW *menuWin, WINDOW *outputWin, PCB *pcb, WINDOW *listWin);

void simulateTime(PCB *pcb, int op1);

void clearTerminal();

void processInterrupt(PCB *pcb, int op1);

void semaphoreP(int i, WINDOW *menuWin, WINDOW *outputWin);

void semaphoreV(int i, WINDOW *menuWin, WINDOW *outputWin);

void memLoadReq(PCB *pcb, int op1, WINDOW *outputWin, WINDOW* listWin);

void memLoadFinish();

void processCreate(FILE *instr, PCB *pcb, int op1, WINDOW *menuWin, WINDOW *outputWin, WINDOW* listWin);

int inserir_com_prioridade(PCB *num, int op1, FILE *arquivo, WINDOW* menuWin);

int existenciaSemaforo(char verificarSemaforo);

void processFinish(WINDOW* listWin);

void imprimir();