#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <libpad.h>
#include <gsKit.h> 

// Configurações do Jogo
#define LARGURA_MAPA 12
#define ALTURA_MAPA 12
#define TOTAL_CADERNOS 7

// Estados do Jogo
enum EstadoJogo { TELA_MENU, GAMEPLAY, GAME_OVER, VITORIA };

// Estrutura para Jogador e Baldi
struct Entidade {
    float x;
    float y;
    float velocidade;
};

// Estrutura para os outros personagens
struct Obstaculo {
    float x;
    float y;
    bool ativo;
    float velocidade;
};

// Mapa da Escola
int mapaEscola[ALTURA_MAPA][LARGURA_MAPA] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,2,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1},
    {1,2,1,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,1,2,0,1,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,1,2,0,0,0,1,2,1},
    {1,3,1,1,1,1,1,1,1,1,1,1}
};

// Variáveis Globais
EstadoJogo estadoAtual = TELA_MENU;
Entidade jogador = {1.5f, 1.5f, 0.05f};
Entidade baldi = {5.5f, 5.5f, 0.02f}; 
Obstaculo bully = {7.5f, 2.5f, true, 0.0f};      
Obstaculo meiao = {10.5f, 1.5f, false, 0.15f};    

int cadernosColetados = 0;
int itensJogador = 1;         
float raivaDoBaldi = 1.0f;    
int cronometroRegua = 0;
static char padBuf[256] __attribute__((aligned(64)));

// Protótipos
void inicializarControles();
void atualizarGameplay();
void verificarColisoes();
void inteligenciaBaldi();

int main(int argc, char *argv[]) {
    SifInitRpc(0);
    inicializarControles();

    GSGLOBAL *gsGlobal = gsKit_init_global();
    gsGlobal->Mode = GS_MODE_NTSC; 
    gsKit_init_screen(gsGlobal);

    while (1) {
        u64 corFundo = GS_SETREG_RGBAQ(0, 0, 0, 0, 0x80);
        gsKit_clear(gsGlobal, corFundo);

        switch (estadoAtual) {
            case TELA_MENU: {
                struct padButtonStatus buttons;
                if (padRead(0, 0, &buttons) != 0) {
                    u32 b = 0xFFFF ^ buttons.btns;
                    if (b & PAD_START) {
                        estadoAtual = GAMEPLAY;
                    }
                }
                break;
            }
            case GAMEPLAY:
                atualizarGameplay();
                inteligenciaBaldi();
                verificarColisoes();
                break;

            case GAME_OVER:
                break;

            case VITORIA:
                break;
        }

        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
    }

    return 0;
}

void inicializarControles() {
    padInit(0);
    padPortOpen(0, 0, padBuf);
}

void atualizarGameplay() {
    struct padButtonStatus buttons;
    int ret = padRead(0, 0, &buttons);

    if (ret != 0) {
        u32 b = 0xFFFF ^ buttons.btns;
        float proximoX = jogador.x;
        float proximoY = jogador.y;

        if (b & PAD_UP)    proximoY -= jogador.velocidade;
        if (b & PAD_DOWN)  proximoY += jogador.velocidade;
        if (b & PAD_LEFT)  proximoX -= jogador.velocidade;
        if (b & PAD_RIGHT) proximoX += jogador.velocidade;

        float distBully = sqrt(pow(proximoX - bully.x, 2) + pow(proximoY - bully.y, 2));
        if (distBully < 0.5f && bully.ativo) {
            if (itensJogador > 0) {
                itensJogador--; 
                bully.ativo = false; 
            } else {
                proximoX = jogador.x;
                proximoY = jogador.y;
            }
        }

        if (mapaEscola[(int)proximoY][(int)proximoX] != 1) {
            jogador.x = proximoX;
            jogador.y = proximoY;
        }

        int posX_Int = (int)jogador.x;
        int posY_Int = (int)jogador.y;
        if (mapaEscola[posY_Int][posX_Int] == 2) {
            cadernosColetados++;
            mapaEscola[posY_Int][posX_Int] = 0; 
            raivaDoBaldi += 0.3f;               
        }

        if (mapaEscola[posY_Int][posX_Int] == 3 && cadernosColetados >= TOTAL_CADERNOS) {
            estadoAtual = VITORIA;
        }
    }
}

void inteligenciaBaldi() {
    cronometroRegua++;
    int intervaloPasso = 60 / raivaDoBaldi; 

    if (cronometroRegua >= intervaloPasso) {
        cronometroRegua = 0;
        if (baldi.x < jogador.x) baldi.x += 0.1f;
        if (baldi.x > jogador.x) baldi.x -= 0.1f;
        if (baldi.y < jogador.y) baldi.y += 0.1f;
        if (baldi.y > jogador.y) baldi.y -= 0.1f;
    }
}

void verificarColisoes() {
    float distBaldi = sqrt(pow(jogador.x - baldi.x, 2) + pow(jogador.y - baldi.y, 2));
    if (distBaldi < 0.4f) {
        estadoAtual = GAME_OVER;
    }

    float distMeiao = sqrt(pow(jogador.x - meiao.x, 2) + pow(jogador.y - meiao.y, 2));
    
    if (distMeiao < 1.5f && cadernosColetados >= TOTAL_CADERNOS && !meiao.ativo) {
        meiao.ativo = true;
    }

    if (meiao.ativo) {
        if (meiao.x < jogador.x) meiao.x += meiao.velocidade;
        if (meiao.x > jogador.x) meiao.x -= meiao.velocidade;
        if (meiao.y < jogador.y) meiao.y += meiao.velocidade;
        if (meiao.y > jogador.y) meiao.y -= meiao.velocidade;

        if (distMeiao < 0.4f) {
            jogador.x = baldi.x;
            jogador.y = baldi.y;
            meiao.ativo = false; 
        }
    }
}
