#ifndef COMUNICACAOCAN_H
#define COMUNICACAOCAN_H

#include <SPI.h>
#include <mcp2515.h>

// Declarações
void inicializarCAN();
void enviarMensagem(int comando, int dado);
void enviarMensagem2(int comando, int dado);
float receberMensagemCAN(int* idMensagem, byte* dados);

#endif
