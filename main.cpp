        if (mapaEscola[posY_Int][posX_Int] == 2) {
            cadernosColetados++;
            mapaEscola[posY_Int][posX_Int] = 0; // Transforma o local em corredor livre
            raivaDoBaldi += 0.3f;               // Baldi ganha velocidade extra
            printf("Caderno Coletado! Total: %d/%d\n", cadernosColetados, TOTAL_CADERNOS);
        }

        // 4. Verificação de Condição de Vitória na Saída (Número 3 na matriz)
        if (mapaEscola[posY_Int][posX_Int] == 3 && cadernosColetados >= TOTAL_CADERNOS) {
            estadoAtual = VITORIA;
            printf("Voce coletou tudo e escapou da escola! VITORIA!\n");
        }
    }
}

void inteligenciaBaldi() {
    cronometroRegua++;
    
    // Calcula o tempo de resposta do passo baseado na raiva acumulada
    int intervaloPasso = 60 / raivaDoBaldi; 

    if (cronometroRegua >= intervaloPasso) {
        cronometroRegua = 0;
        
        // Perseguição em Grade: Baldi recalcula e dá um passo focado no jogador
        if (baldi.x < jogador.x) baldi.x += 0.1f;
        if (baldi.x > jogador.x) baldi.x -= 0.1f;
        if (baldi.y < jogador.y) baldi.y += 0.1f;
        if (baldi.y > jogador.y) baldi.y -= 0.1f;
    }
}

void verificarColisoes() {
    // 1. Colisão Direta com o Baldi (Gera Game Over)
    float distBaldi = sqrt(pow(jogador.x - baldi.x, 2) + pow(jogador.y - baldi.y, 2));
    if (distBaldi < 0.4f) {
        estadoAtual = GAME_OVER;
        printf("O Baldi te pegou! GAME OVER.\n");
    }

    // 2. Lógica de Perseguição e Colisão do Meião (Arts and Crafters)
    float distMeiao = sqrt(pow(jogador.x - meiao.x, 2) + pow(jogador.y - meiao.y, 2));
    
    // Se o jogador chegar muito perto dele já portando os 7 cadernos, ele altera para ativo
    if (distMeiao < 1.5f && cadernosColetados >= TOTAL_CADERNOS && !meiao.ativo) {
        meiao.ativo = true;
        printf("[MEIAO]: WOOOOOOSH! Ficou bravo!\n");
    }

    if (meiao.ativo) {
        // Corre de forma extremamente veloz atrás do jogador
        if (meiao.x < jogador.x) meiao.x += meiao.velocidade;
        if (meiao.x > jogador.x) meiao.x -= meiao.velocidade;
        if (meiao.y < jogador.y) meiao.y += meiao.velocidade;
        if (meiao.y > jogador.y) meiao.y -= meiao.velocidade;

        // Se o Meião encostar, teleporta o jogador direto para a posição atual do Baldi
        if (distMeiao < 0.4f) {
            jogador.x = baldi.x;
            jogador.y = baldi.y;
            meiao.ativo = false; // Reseta o estado do meião após o susto
            printf("[MEIAO]: Teleportado direto para o Baldi!\n");
        }
    }
}
