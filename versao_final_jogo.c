/*Cabeçalhos*/
#include <unistd.h>
#include <stdio.h>
#include <ncurses.h> // Para funçőes de desenhos
#include <stdlib.h>
#include <locale.h> // Para printw ler tabela unicode
#include <time.h>
#include <string.h>
#include <limits.h>
/*Constantes*/
#define MAXMONSTROS 5
#define MAXBOMBAS 5
#define MAXVIDAS 9
#define MAXRAIO 3
//Constantes para o menu
#define NOVO_JOGO 0
#define CARREGAR 1
#define HIGHSCORES 2
#define SAIR 3
//Constantes para associar teclas (tabela ASCII) (Năo existentes na ncurses.h)
#define ENTER 10
#define KEY_ESC 27
#define ESPACO 32
#define KEY_TAB 9
//Constantes para tamanho das matrizes
#define NLINHAS 15
#define NCOLUNAS 35
//Constantes para desenhar e atribuir valores ŕs matrizes
#define BRANCO 0
#define PAREDE 1
#define OBSTRUCAO 2
#define MONSTRO 3
#define BOMBA 4
#define POWERUP_RAIO 5
#define POWERUP_VIDA 6
#define POWERUP_BOMBA 7
#define FOGO 8
#define SAIDA 9
#define BOMBERMAN 10
//Constantes para o retorno da funçăo movimentador
#define MAISVIDA 1
#define MENOSVIDA -1
#define MAISRAIO 2
#define MAISBOMBA 3
#define ACHOUSAIDA 4
//Constantes de valores lógicos
#define VERDADEIRO 1
#define FALSE 0
//Strings da unicode
#define DESENHO_PAREDE "\u2588"
#define DESENHO_OBSTRUCAO "\u25A8"
#define DESENHO_MONSTRO "\u2C01"
#define DESENHO_BOMBERMAN "B" //"\u26F9"
#define DESENHO_VIDA "\u2665"
#define DESENHO_BOMBA "\u0DA7"
#define DESENHO_RAIO "\u2B4D"
#define DESENHO_POWER "\u2622"
#define DESENHO_FOGO "*"
#define DESENHO_SAIDA "#" //"\u498F"
#define DESENHO_MAIS_BOMBA "@"
//Definições dos valores de retorno da função save
#define SALVAR 1
#define NAO_SALVAR 0
#define CANCELAR -1

#define CIMA 3
#define BAIXO 2
#define ESQUERDA 1
#define DIREITA 0

#define MAX_RECORDES 10

typedef struct
{
    int level;
    int vida;
    int pontos;
    int raio;
    int nbombas;
} INFO; //Estrutura que guarda os valores da partida

typedef struct
{
    int linha;
    int coluna;
} POSICAO; //Estrutura para determinar linha e coluna no ecra da tela

typedef struct
{
    int ativada;
    clock_t inicio;
    clock_t fim;
    int explodir;
    POSICAO coord;
} TNT; //Estrutura para bomba. Os campos são utilizados para verificar se o clock de inicio e fim da bomba for maior ou igual a 3, e uma flag pra identificar a explosao


typedef struct
{
    int matrizjogo[NLINHAS][NCOLUNAS];
    int matrizaux[NLINHAS][NCOLUNAS];
} MAPA; //Matriz que recebe o conteúdo das fases, e posiciona obstruções, paredes, power-ups, saída e o próprio Bomberman

typedef struct
{
    int minutos;
    int segundos;
} RELOGIO; //Estrutura utilizada para representar o relógio no jogo

typedef struct
{
    char nome[16];
    int pontos;
} JOGADOR; //Estrutura definida para registros de score do jogador

/*Protótipos das funçőes*/
void chama_teclado(void);
int menu(void);
int controlador(INFO* info);
void recordes(void);
int areyousure(void);
int nivel(INFO* info);
int coloca_na_matriz(int level, MAPA* mapa, POSICAO* posicao);
void desenhoini(MAPA mapa);
void imprime_desenho(int linha, int maxitem, char str[]);
void imprime_Npu(int linha, int coluna, int matriz[NLINHAS][NCOLUNAS]);
int movimentador(POSICAO* posicao, int d_horiz, int d_vert, MAPA* mapa, int* contar_bomb);
int tempo(RELOGIO* relogio);
int salvar(void);
int explode_bomba(int raio, POSICAO local_bomb, MAPA* mapa);
int desenha_fogo(MAPA* mapa, int sinal, POSICAO local_bomb,int raio, char eixo[]);
int apaga_fogo(MAPA* mapa);
int movimenta_monstro(MAPA* mapa, int* pontos);
void recordes(void);
int maior_pont(void);
void salvarecorde(int pontos);
void organizador(JOGADOR melhores[MAX_RECORDES]);

//Função principal


int main(void)
{
    int opcao, finalizar=FALSE, sair=FALSE, erro=FALSE;
    INFO info;
    FILE* arq_carregar;

    setlocale(LC_ALL,""); // Para printw reconhecer tabela unicode
    initscr(); //Inicializa a biblioteca Ncurses
    do
    {
        opcao=menu(); //Chama a função menu para verificar a opção desejada pelo jogador
        switch(opcao)
        {
            case NOVO_JOGO: //Se o jogador desejar começar um novo jogo, o programa inicializa as informações do Bomberman no jogo, evitando assim que o jogador possua mais bombas ou algo do tipo pra iniciar um jogo
                info.level=1;
                info.nbombas=1;
                info.raio=1;
                info.pontos=0;
                info.vida=3;
                erro=controlador(&info); //Chama a função controlador para que o jogo inicie as fases
                if(erro) //Se o valor retornado pela função controlador através da variável sair for o de encerramento do jogo, então o programa ativa a flag que encerra a execução do programa
                    finalizar=TRUE;
                break;
            case CARREGAR: //Se o valor retornado pela função menu corresponde a CARREGAR
                if(arq_carregar=fopen("gamesave.bin","rb")) //Programa abre o arquivo e testa para ver se o arquivo foi aberto corretamente
                {
                    if(fread(&info,sizeof(info),1,arq_carregar)==1) //Lê o arquivo e realiza um teste para verificar se a leitura foi feita corretamente
                        controlador(&info); //Chama a função controlador e passa os dados salvos para o usuário
                    else
                        erro=TRUE;
                    fclose(arq_carregar);
                }
                else
                    erro=TRUE;
                break;
            //Se a opção retornada for Highscores, a função recordes é chamada pois ela ira inicializar uma nova tela, ler o arquivo que contem estes registros e em seguida imprimi-los para a visualização do usuário
            case HIGHSCORES:
                erase(); //Limpa a tela
                recordes();
                break;
            //Chama a função areyousure para verificar se o usuário deseja realmente sair
            case SAIR:
                sair=areyousure();
                if(sair) //Se o usuário selecionar sim na função, o programa ativa a flag para encerrar a execução do jogo
                    finalizar=TRUE;
        }
    }
    while(!finalizar); //Enquanto não finalizarem o jogo, ele segue sendo executado
    if(erro) //Se ocorrerem erros no controlador
        printf("Erro!!!");
    return 0;
}

//Essa função serve para reconhecer teclas na Ncurses

void chama_teclado(void)
{
    raw(); //Na medida em que os caracteres são digitados são repassados direto para o programa. Isso também permite o uso de teclas especiais
    noecho(); //Isso evita que os caracteres "ecoem" até chegar a uma nova linha
    keypad(stdscr,true); //Isso faz com que o programa aguarde o recebimento de uma tecla
    nodelay(stdscr,true); //Habilita o teclado, assim como o mapeamento de teclas especiais
    curs_set(false); //Oculta o cursor na tela
}

//Função menu
int menu(void)
{
    int carac, escolheu=FALSE, opcao, apagar=FALSE; //Declaração de variáveis utilizadas no programa
    POSICAO cursor, palavra_bomberman={5,13};
    FILE *point_arq;

    initscr(); //Inicializa a Ncurses
    mvprintw(0,1,">  Novo Jogo");
    mvprintw(1,4,"Carregar");
    mvprintw(2,4,"Highscores");
    mvprintw(3,4,"Sair"); //Os printw's inseridos ate aqui foram para colocar as opções do menu disponíveis para o usuário
    refresh(); //Atualiza o que está sendo impresso no ecra
    chama_teclado(); //Chama a funçao que ativa o teclado para que o usuário possua usufruir dele
    cursor.linha=0;
    cursor.coluna=1; //Guarda a posição do cursor
    move(0,1); //Posiciona o cursor na opção novo jogo
    do
    {
        carac=getch(); //Pega o caracter digitado pelo usuário
        switch(carac)
        {
            case ENTER: //Caso ele aperte enter, o programa verifica a opção seleciona pelo usuário
                if(cursor.linha==1) //Caso a opção selecionada seja carregar o jogo, o programa verifica se possui algum arquivo salvo anteriormente
                {
                    if(point_arq=fopen("gamesave.bin","rb")) //Abre o arquivo gamesave que é utilizado para
                    {
                        opcao=cursor.linha;
                        escolheu=TRUE;
                        fclose(point_arq);
                    }
                    else //Caso o usuário não tenha salvo nenhum um jogo anteriormente, o programa retorna uma mensagem ao usuário
                    {
                        mvprintw(1,20,"Nenhum jogo salvo");
                        apagar=TRUE;
                    }
                }
                else //Senão for carregar o jogo a opção desejada, o programa pega o valor da linha selecionada e retorna isso para o main verificar o que foi selecionado pelo usuário
                {
                    opcao=cursor.linha; //Envia o indice da linha que é utilizado na função principal, retornando qual a opção foi selecionada pelo usuário
                    escolheu=TRUE; //Ativa a flag para quebrar a entrada no laço
                }
                break;
            case KEY_UP:  //Verifica se o usuário pressionou a seta para cima do teclado
                if(apagar) //Verifica a flag apagar
                {
                    mvprintw(1,20,"                 "); //Limoa a tela
                    apagar=FALSE;
                }
                if(cursor.linha>0) //Realiza o movimento da seleção da opção, desenhando assim o cursor na nova opção
                {
                    mvprintw(cursor.linha,cursor.coluna,"  ");
                    cursor.linha=cursor.linha-1;
                    mvprintw(cursor.linha,cursor.coluna,">");
                    refresh();
                }
                break;
            case KEY_DOWN:  //Verifica se o usuário pressionou a seta para baixo do teclado. Faz basicamente o mesmo que o caso anterior, no entanto esse imprime uma carinha triste quando o usuário posicionar o cursor na opção sair
                if(apagar)
                {
                    mvprintw(1,20,"                 ");
                    apagar=FALSE;
                }
                if(cursor.linha<3)
                {
                    mvprintw(cursor.linha,cursor.coluna,"  ");
                    cursor.linha=cursor.linha+1;
                    move(cursor.linha,cursor.coluna);
                    if(cursor.linha==3)
                        printw(":(");
                    else
                        printw(">");
                }
        }
        //Esse bloco de comandos imprime a palavra Bomberman aleatóriamente no canto superior direito da tela
        mvprintw(palavra_bomberman.linha,palavra_bomberman.coluna,"         ");
        srand(time(NULL));
        palavra_bomberman.linha=5+rand()%((14-5)+1);
        palavra_bomberman.coluna=13+rand()%((29-13)+1); //Esses dois ultimos comandos evitam que a impressão da palavra BOMBERMAN consoma com as opções, delimitando-as ao canto inferior direito do ecra
        mvprintw(palavra_bomberman.linha,palavra_bomberman.coluna,"BOMBERMAN");
        refresh();
    }
    while(!escolheu); //Enquanto o usuário não escolher o programa segue executando a função
    erase();
    endwin();

    return opcao; //Retorna a opção selecionada pelo usuário
}

int areyousure(void) //Como somos muito apelativos para que o usuário continue gastando seu tempo no nosso jogo, nos certificamos de que ele realmente deseja isso
{
    int carac, escolheu=FALSE, sim=FALSE;

    initscr();
    mvprintw(5,12,"VOCÊ TEM CERTEZA?");
    mvprintw(7,13,"SIM");
    mvprintw(7,23,">NÃO<");
    refresh();
    chama_teclado();
    move(7,23);
    do
    {
        carac=getch();
        switch(carac) //Pega o caractere pressionado pelo usuário, e verifica se ele ja se decidiu ao apertar o enter. Ao pressionar uma das setas, a opção fica pré-selecionada, e o enter só funciona como um critério de parada da função
        {
            case ENTER:
                escolheu=TRUE; //Flag para quebrar o laço
                erase();
                break;
            case KEY_RIGHT:
                mvprintw(7,12," SIM ");
                mvprintw(7,23,">NÃO<");
                refresh();
                sim=FALSE;
                break;
            case KEY_LEFT:
                mvprintw(7,12,">SIM<");
                mvprintw(7,23," NÃO ");
                refresh();
                sim=TRUE;
        }
    }
    while(!escolheu); //Enquanto ele não apertar enter, o programa ficara executando essa tela
    endwin();
    return sim;
}

//Função que controla as fases do jogo
int controlador(INFO* info)
{
    int sair=FALSE, erro=FALSE;;
    do
    {
        sair=nivel(info); //Chama a função fase para executar a fase na qual o jogador se encontra e recebe um valor retornado pela função na variável sair pois há um valor utilizado para detectar se acabou a partida para o usuário
        if(sair==-1) //Verifica se o valor retornado pela função nível é a condição para quebrar a condição do laço e encerrar as iterações
        {
            sair=TRUE;
            erro=TRUE;
        }
        info->level=info->level+1; //Quando o jogador termina a fase, o campo level das suas informações é incrementado em uma unidade para que quando a função nível for chamada na próxima iteração ela carregue uma nova fase
        if(info->level>5) //Se o usuário jogar as cinco fases, então a flag para sair do laço também é ativada pois não ha mais fases para serem jogadas
            sair=TRUE;
    }
    while(!sair); //Enquanto o jogador não encerrar o jogo(seja via teclado, por término das fases ou por esgotamento de vidas) a função continua a realizar o controle, garantido a passagem do jogador para a próxima fase
    return erro; //Retorna uma flag para a main
}

//A função nivel fica responsavel por gerenciar cada fase bem como os elementos pertencentes a cada nivel
int nivel(INFO* info)
{
    MAPA mapa={{BRANCO},{BRANCO}}; //Estrutura do tipo mapa para armazenar os elementos pertencentes a fase
    POSICAO coordenada; //Estrutura do tipo mapa para armazenar os elementos pertencentes a fase
    RELOGIO relogio; //Estrutura do tipo relógio para realizar o timer do jogo
    FILE* point_save; //Estrutura do tipo arquivo para poder gravar o progresso recorrente do jogador
    INFO teste;
    TNT bomb[5]={FALSE,0,0,FALSE,{0,0}};
    int carac, cont, bomba_indice, hi, bombas_desativadas, gravar=FALSE;
    int sair=FALSE, opcao=0, morreu=FALSE, aux=0, o_q_deseja=CANCELAR, contar_rel=TRUE, ativei=FALSE, exibir=FALSE, end_level=FALSE;
    clock_t inicio_rel, fim_rel;

    initscr(); //Inicia a tela para uso dos comandos da ncurses
    chama_teclado(); //Habilita o uso do teclado para o jogador
    hi=maior_pont(); //Chama a função maior_pont para ir registrando a pontuação do jogador no arquivo highscores.bin
    do
    {
        //Inicializa as bombas do usuário
        for(cont=0;cont<MAXBOMBAS;cont++)
        {
            bomb[cont].ativada=FALSE;
            bomb[cont].inicio=0;
            bomb[cont].fim=0;
            bomb[cont].explodir=FALSE;
            bomb[cont].coord.linha=0;
            bomb[cont].coord.coluna=0;
        }
        //A função abaixo preenche na matriz os valores que identificam cada elemento na fase
        opcao=coloca_na_matriz(info->level,&mapa,&coordenada);
        if(opcao==-1) //Se o valor retornado pela função for -1, o programa recebe uma flag para encerrar o jogo
            sair=TRUE;
        desenhoini(mapa); //Chama a função que desenha cada elemento representado na matriz
        mvprintw(7,22,"NIVEL %d",info->level); //Mostra na tela o nível no qual o jogador se encontra
        move(coordenada.linha,coordenada.coluna); //Move linha e coluna para a origem
        relogio.minutos=3;
        relogio.segundos=0; //Inicializa o relógio
        bombas_desativadas=info->nbombas; //Essa variável controla o numero de bombas inseridas pelo jogador. A cada bomba colocada, essa variável é decrementada e depois de explodirem as bombas ela retoma seu valor original
        while(!morreu && !sair)  //Enquanto o jogador não perder todas as vidas e não desejar sair
        {
            imprime_desenho(1,info->vida,DESENHO_VIDA); //Essa função  chamada para imprimir a quantidade de vidas que o usuário tem disponível
            if(exibir) //Verifica se a flag exibir foi ativada e imprime a quantidade de powerups presentes no cenário
                imprime_Npu(8,23,mapa.matrizaux);
            if(info->pontos<1000000) //Se os pontos não atingirem ao valor indicado(a impressão do valor indicado no if iria sobrepor partes do layout da fase) os pontos são impressos normalmente
                mvprintw(1,10,"%6d",info->pontos);
            else
                mvprintw(1,10,"  %.1fM",(float)info->pontos/1000000); //Senão os pontos são adaptados para uma nova notação para que não ocorra esse problema do layout
            if(info->pontos>=hi) //Se o usuário atingir um número de pontos maior que o do highscore registrado, o programa atualiza o novo valor de highscore
                hi=info->pontos;
            mvprintw(12,17,"HI:%14d",hi); //Imprime o highscore anterior
            mvprintw(13,17,"P: %14d",info->pontos); //Imprime a pontuação do jogador
            mvprintw(3,12,DESENHO_RAIO);
            printw(" =%d",info->raio); //Os dois prints anteriores servem para imprimir o numero de powerup raio da bomba obtido pelo jogador
            mvprintw(3,1,"     ");
            imprime_desenho(3,bombas_desativadas,DESENHO_BOMBA); //Imprime para o usuário a quantidade de bombas obtida pelo usurio
            mvprintw(2,2,"%d min ",relogio.minutos);
            printw("%2d seg", relogio.segundos); //Os dois prints anteriores se referem a impressão do timer da fase
            refresh(); //Atualiza
            carac=FALSE;
            carac=getch(); //Pega o valor de uma tecla pressionado pelo usuário
            if(contar_rel) //Pega o clock inicial da execução do código
            {
                inicio_rel=clock();
                contar_rel=FALSE;
            }
            //Varre o vetor que armazena as bombas do jogador
            for(cont=0;cont<MAXBOMBAS;cont++)
            {
                //Se o jogador decidir colocar uma bomba, então será registrado o inicio do clock da bomba e também será ativado uma flag para que possamos contar a variação de clock para controlar o decorrer de 3 segundos
                if(bomb[cont].ativada)
                {
                    bomb[cont].inicio=clock();
                    bomb[cont].explodir=TRUE;
                }
            }

            //Verifica o caractere pressionado pelo usuário. Se a tecla for uma das setas, a função movimentador é chamada para que realize o movimento do Bomberman.
            switch(carac)
            {
                //Caso especial: Caso o usuário aperte TAB, o programa ira alterar a flag para que seja impresso a quantidade de power-ups presentes na fase
                case KEY_TAB:
                    exibir=!exibir;
                    mvprintw(8,23,"     ");
                    break;

                //Observação importante: a função movimentador só recebe como parâmetro um campo da estrutura TNT, utilizada para definição da bomba, o programa só ira inserir a bomba se após pressionada a barra de espaço, o jogador realize algum movimento em seguida
                case KEY_UP:
                    aux=movimentador(&coordenada,-1,0,&mapa,&bomb[bomba_indice].ativada);
                    break;
                case KEY_DOWN:
                    aux=movimentador(&coordenada,1,0,&mapa,&bomb[bomba_indice].ativada);
                    break;
                case KEY_LEFT:
                    aux=movimentador(&coordenada,0,-1,&mapa,&bomb[bomba_indice].ativada);
                    break;
                case KEY_RIGHT:
                    aux=movimentador(&coordenada,0,1,&mapa,&bomb[bomba_indice].ativada);
                    break;

                //Se o usuário pressionar espaço, o programa inicia o processo de execução da bomba
                case ESPACO:
                    if(bombas_desativadas>0) //Verifica se o jogador possui bombas disponíveis para uso
                    {
                        if(mapa.matrizaux[coordenada.linha][coordenada.coluna]!=BOMBA) //Verifica se já nao consta uma bomba no local onde se deseja inseri-la e então faz a inserção da bomba atráves da matriz auxliar
                        {
                            mapa.matrizaux[coordenada.linha][coordenada.coluna]=BOMBA; //Insere o valor de bomba na matriz
                            bombas_desativadas=bombas_desativadas-1; //Decrementa a variável no intuito de informar no programa que uma bomba foi ativada
                            bomba_indice=0; //Começa a executar as bombas a partir da primeira, ou seja, as bombas estouram na sequencia em que são inseridas e não de forma aleatória
                            do
                            {
                                //Verifica se a bomba está disponível no vetor para uso e se está não se encontra ativada
                                if(!(bomb[bomba_indice].explodir) && !(bomb[bomba_indice].ativada))
                                {
                                    ativei=TRUE; //Modifica a flag indicando que a bomba foi ativafa
                                    bomb[bomba_indice].coord=coordenada; //Guarda a coordenada onde a bomba foi inserida
                                }
                                else //Senão verifica a próxima presente no vetor
                                    bomba_indice++;
                            }
                            while(!ativei); //Ele executa cada bomba individualmente, sendo assim o programa sai do laço logo que encontrar a primeira bomba ativada
                            ativei=FALSE; //Desabilita a flag para poder ser reutilizada para as demais bombas
                        }
                    }
                    break;

                //Caso o usuário digite ESC, o programa vai suspender a fase através de uma função salvar
                case KEY_ESC:
                    o_q_deseja=salvar(); //Chama a função salvar para ver as opções que o usuário deseja no "pause"
                    if(o_q_deseja==SALVAR) //Se o usuário decidiu salvar, o programa abre o arquivo para escrita
                    {
                        if(point_save=fopen("gamesave.bin","wb"))
                        {
                            if(fwrite(info,sizeof(teste),1,point_save)!=1) //Testa se a escrita em disco foi correta
                                opcao=-1;
                            else
                                opcao=1;
                            fclose(point_save); //Fecha o arquivo após armazenar as informações da partida do jogador
                            gravar=TRUE;
                        }
                        else
                            opcao=-1;
                        sair=TRUE;
                    }
                    else if(o_q_deseja==NAO_SALVAR) //Se o usuário deseja apenas sair do jogo e não salvar seu progresso, o programa recebe apenas a alteraço da flag e encerra o laço responsável pela execução da fase
                    {
                        sair=TRUE;
                        opcao=1;
                    }
                    break;
            }
            fim_rel=clock(); //Pega um clock "final" para que assim possamos pegar a variação de tempo em função do clock
            if((fim_rel-inicio_rel)/CLOCKS_PER_SEC>=1) //Calculo da variação de clocks por segundo
            {
                morreu=tempo(&relogio); //chama a função tempo para ir decrementando um segundo do relógio
                info->pontos=info->pontos+apaga_fogo(&mapa); //Chama a função apaga fogo que limpa a explosão na tela e pega os valores da pontuados pelo usuário ao explodir algo
                if(!morreu) //Verifica se o usuário não morreu em função do valor retornado pelo relógio
                {
                    morreu=movimenta_monstro(&mapa,&info->pontos); //Chama a função que realiza o movimento dos monstros no mapa
                    if(morreu) //Ativa um sleep para impedir que o movimento dos monstros seja rápido demais
                        sleep(1);
                }

                contar_rel=TRUE; //
            }

            //Percorre o vetor bomb e verifica as bombas que foram ativadas
            for(cont=0;cont<MAXBOMBAS;cont++)
            {
                //Se a bomba foi ativada, o programa pega o clock final(o clock inicial da bomba é pego logo quando o usuário pressiona espaço) e realiza o cálculo da variação dividido pela constante CLOCKS_PER_SEC
                if(bomb[cont].explodir)
                {
                    bomb[cont].ativada=FALSE;
                    bomb[cont].fim=clock();
                    //Verifica se essa variação já tingiu três segundos e chama a função explode_bomba caso isso aconteça
                    if(((bomb[cont].fim)-(bomb[cont].inicio))/CLOCKS_PER_SEC>=3)
                    {
                        morreu=explode_bomba(info->raio,bomb[cont].coord,&mapa);
                        if(morreu)
                            sleep(1); //Para evitar que a explosão ocorra de forma demasiado rápida
                        bombas_desativadas=bombas_desativadas+1; //Informa a variável de controle das bombas que uma delas já explodiu
                        bomb[cont].explodir=FALSE; //Desabilita a flag que da inicio ao processo de explosão
                    }
                }
            }
            //Verifica os elementos encontrado pelo usuário. Os power-ups incrementam em 200  a pontuação do usuário
            switch(aux)
            {
                case MENOSVIDA:
                    morreu=TRUE;
                    break;
                //Verifica se o jogador não atingiu o limite maximo de vidas e então incrementa uma vida
                case MAISVIDA:
                    if(info->vida<MAXVIDAS)
                        info->vida=info->vida+1;
                    info->pontos=info->pontos+200;
                    break;
                //Se o usuário passou em um power-up logo ele é testado primeiro ara verificar se nao atingiu ainda o limite maximo e em seguida incrementa o efeito do power-up obtido
                case MAISBOMBA:
                    if(info->nbombas<MAXBOMBAS)
                    {
                        info->nbombas=info->nbombas+1;
                        bombas_desativadas++;
                    }
                    info->pontos=info->pontos+200;
                    break;
                case MAISRAIO:
                    if(info->raio<MAXRAIO)
                        info->raio=info->raio+1;
                    info->pontos=info->pontos+200;
                    break;
                //Caso o usuário ache a saida, o programa ativa uma flag pra encerrar a fase para que o jogo retorne a função controlador e assim possa ser chamado a proxima fase
                case ACHOUSAIDA:
                    sair=TRUE;
                    end_level=TRUE;
            }
            //Se o jogador morreu ele perde os power-ups obtidos durante a partida e perde uma vida é claro
            if(morreu)
            {
                info->vida=info->vida-1;
                info->nbombas=1;
                info->raio=1;
                opcao=1;
                erase();
                //Caso acabe as vidas do jogador, é impresso uma mensagem informando isso ao usuário
                if(info->vida>0 && !sair)
                {
                    mvprintw(5,15,"VOCÊ MORREU!");
                    mvprintw(6,18,DESENHO_VIDA);
                    printw(" x %d",info->vida);
                    refresh();
                    sleep(3); //Evita que a mensagem seja exibida de forma muito rápida
                }
            }
            aux=0;
            carac=0;
        }
        //Se o usuário não desejar sair e ainda tiver vidas para seguir jogando, o programa desabilita as flags que poderiam encerrar o jogo de forma inesperada
        if(info->vida!=0 && o_q_deseja==CANCELAR)
        {
            morreu=FALSE;
            opcao=0;
        }
    }
    while(!morreu && !sair);

    //Depois de encerrar a fase, o tempo restante é convertido em pontos para o jogador e então informa ao jogador que este terminou o jogo e exibe seus pontos
    if(!morreu)
    {
        info->pontos=info->pontos+(((60*relogio.minutos)+relogio.segundos)*5);
    }

    //Verifica se o usuário morreu ou desejou sair
    if(morreu || info->level==5)
    {
        //Se o jogador morrer e não decidir sair do jogo essa mensagem é impressa na tela
        if(morreu && !sair)
        {
            erase();
            mvprintw(5,10,"QUE PENA, VOCÊ PERDEU!");
            mvprintw(6,10,"PONTUAÇÃO: %10d",info->pontos);
            refresh();
            sleep(3);
        }
        //Verifica se o jogador não morreu, se chegou na ultima fase e se encerrou a ultima fase
        if(info->level==5 && !morreu && end_level==TRUE)
        {
            erase();
            mvprintw(5,10,"PARABÉNS, VOCÊ VENCEU!");
            mvprintw(6,10,"PONTUAÇÃO: %10d",info->pontos);
            refresh();
            sleep(3);
        }
        //Verificação para gravar o score do jogador
        if(!gravar)
        {
            erase();
            salvarecorde(info->pontos);
            erase();
            recordes();
        }
    }
    erase();
    refresh();
    endwin();
    return opcao;
}

//Essa função tem por obejtivo preencher os valores que identificam cada elemento na matriz. Lembrando que cada valor foi definido previamente no inicio do programa para fins de legibilidade do código e praticidade na hora de realizar testes
int coloca_na_matriz(int level, MAPA* mapa, POSICAO* posicao)
{
    int erro=0, linha, coluna, parede;
    FILE* arq_fase;
    char fase[]="nivelx.txt", carac;

    fase[5]=level+48; //Isso faz com que o numero do level seja preenchido em ASC no string fase, evitando assim ter que realizar sucessivos testes com o parametro de informações do jogo

    //Preenche a parte externa da matriz com o valor parede
    for(linha=0;linha<NLINHAS;linha++)
        for(coluna=0;coluna<NCOLUNAS;coluna++)
            if((linha==4 && coluna<17) || linha==14 || coluna==0 || coluna==16 || linha==0 || coluna==34)
                mapa->matrizjogo[linha][coluna]=PAREDE;
    if(arq_fase=fopen(fase,"r")) //Abre o arquivo fae para leitura do cenário
    {
        linha=5;
        coluna=1; //Posicionamento de linha e coluna de modo que o preenchimento dos elementos que compoem o cenário se encontrem dentro da caixa desenhada
        while(!(feof(arq_fase))) //Enquanto não chegar no fim do arquivo
        {
            carac=getc(arq_fase); //Pega o caracter apontado no arquivo
            switch(carac) //Verifica o que esse caracter representa na fase e então preenche um valor na matriz para representar o elemento, lembrando que esses valores foram pré-definidos no inicio do programa para convencionar um metodo de representação de elementos da cada fase
            {
                //Caso especial: se o caractere encontrado for um \n, o programa faz a leitura passar para a proxima linha
                case '\n':
                    linha++;
                    coluna=1;
                    break;
                case ' ':
                    mapa->matrizjogo[linha][coluna]=BRANCO;
                    break;
                case 'P':
                    mapa->matrizjogo[linha][coluna]=PAREDE;
                    break;
                case 'B':
                    mapa->matrizjogo[linha][coluna]=BOMBERMAN;
                    posicao->linha=linha;
                    posicao->coluna=coluna;
                    break;
                case 'M':
                    mapa->matrizjogo[linha][coluna]=MONSTRO;
                    break;
                case 'V':
                    mapa->matrizaux[linha][coluna]=POWERUP_VIDA;
                    mapa->matrizjogo[linha][coluna]=OBSTRUCAO;
                    break;
                case 'R':
                    mapa->matrizaux[linha][coluna]=POWERUP_RAIO;
                    mapa->matrizjogo[linha][coluna]=OBSTRUCAO;
                    break;
                case 'Q':
                    mapa->matrizaux[linha][coluna]=POWERUP_BOMBA;
                    mapa->matrizjogo[linha][coluna]=OBSTRUCAO;
                    break;
                case 'S':
                    mapa->matrizaux[linha][coluna]=SAIDA;
                case 'O':
                    mapa->matrizjogo[linha][coluna]=OBSTRUCAO;
            }
            if(carac!='\n') //Enquanto o ponteiro não encontrar \n, ele passa para o próximo elemento da lista
                coluna++;
        }
        fclose(arq_fase); //Após ler todos os elementos da fase, o arquivo é fechado
    }
    else //Se der erro para carregar a fase, o programa retorna o valor -1, definido arbitrariamente como um indicador de erro
        erro=-1;
    return erro;
}

//Essa função varre a matriz, e printa um desenho para cada elemento representado na matriz
void desenhoini(MAPA mapa)
{
    int linha, coluna, old_line;

    //Essa função varre a matriz, e printa um desenho para cada elemento representado na matriz
    for(linha=0;linha<NLINHAS;linha++)
    {
        for(coluna=0;coluna<NCOLUNAS;coluna++)
        {
            move(linha,coluna);
            if(old_line!=linha) //Se o elemento estiver na proxima linha da matriz o programa realiza uma quebra de linha para imprimir corretamente os elementos da linha abaixo
            {
                printw("\n");
                old_line=linha;
            }
            //Verifica cada elemento da matriz e printa algo que corresponda ao identificador encontrado. Definimos no inicio do programa os valores da tabela unicode referente ao desenho que escolhemos para cada elemento no intuito de deixar o código mais legível
            switch(mapa.matrizjogo[linha][coluna])
            {
                case BRANCO:
                    printw(" ");
                    break;
                case PAREDE:
                    printw(DESENHO_PAREDE);
                    break;
                case OBSTRUCAO:
                    printw(DESENHO_OBSTRUCAO);
                    break;
                case MONSTRO:
                    printw(DESENHO_MONSTRO);
                    break;
                case BOMBERMAN:
                    printw(DESENHO_BOMBERMAN);
            }
        }
    }
     //Desenha o contorno da caixa para identificar os limites externos de cada fase que são padronizados
    for(linha=0;linha<NLINHAS;linha++)
        for(coluna=0;coluna<NCOLUNAS;coluna++)
            if((linha==4 && coluna<17) || linha==14 || coluna==0 || coluna==16 || linha==0 || coluna==34)
                mvprintw(linha,coluna,DESENHO_PAREDE);
    mvprintw(15,0," ");
    refresh();
}

//Essa função generica imprime os powerups na linha indicada
void imprime_desenho(int linha, int maxitem, char str[])
{
    int cont;
    for(cont=1;cont<=maxitem;cont++)
        mvprintw(linha,cont,"%s",str);
}

//Imprime o numero de power-ups disponiveis na fase
void imprime_Npu(int linha, int coluna, int matriz[NLINHAS][NCOLUNAS])
{
    int i, j, npu=0;

    for(i=0;i<NLINHAS;i++)
        for(j=0;j<NCOLUNAS;j++)
            if(matriz[i][j]!=BRANCO && matriz[i][j]!=SAIDA && matriz[i][j]!=BOMBA)
                npu++;
    mvprintw(linha,coluna,DESENHO_POWER);
    printw(" x %d",npu);
}

//Essa função é responsável pelo movimento do Bomberman
int movimentador(POSICAO* posicao, int d_horiz, int d_vert, MAPA* mapa, int* contar_bomb)
{
    int opcao=0, linha, coluna;
    linha=posicao->linha+d_horiz;
    coluna=posicao->coluna+d_vert;
    //Verifica se a posição na qual o personagem está posicionado na mesma posição que um monstro ou que do fogo da bomba, power-up ou se achou a saída e retorna um valor definido previamente para a função fase lidar com cada caso
    switch(mapa->matrizjogo[linha][coluna])
    {
        case FOGO:
        case MONSTRO:
            opcao=MENOSVIDA;
            break;
        case POWERUP_BOMBA:
            opcao=MAISBOMBA;
            break;
        case POWERUP_RAIO:
            opcao=MAISRAIO;
            break;
        case POWERUP_VIDA:
            opcao=MAISVIDA;
            break;
        case SAIDA:
            opcao=ACHOUSAIDA;
    }

    //Primeiro verifica se a posição para a qual se deseja movimentar o bomberman não possui uma parede, uma obstrução ou uma bomba. O movimento não pode ocorrer caso nse encontre um desses elementos na posição para qual o bomberman será deslocado
    if(mapa->matrizjogo[linha][coluna]!=PAREDE && mapa->matrizjogo[linha][coluna]!=OBSTRUCAO && mapa->matrizjogo[linha][coluna]!=BOMBA)
    {
        mapa->matrizjogo[posicao->linha][posicao->coluna]=BRANCO; //Insere um branco na matriz indicando que o bomberman não se encontra mais nessa posição
        mvprintw(posicao->linha,posicao->coluna," "); //Limpa o desenho do bomberman na posição anterior
        posicao->linha=linha;
        posicao->coluna=coluna; //A variável posição recebe a nova posição do bomberman na matriz
        if(opcao!=MENOSVIDA) //Se o jogador não caiu em um caso no qual ele deva perder uma vida
        {
            mvprintw(posicao->linha,posicao->coluna,DESENHO_BOMBERMAN); //Imprime o bomberman na posição seguinte
            mapa->matrizjogo[posicao->linha][posicao->coluna]=BOMBERMAN; //Coloca o novo valor do bomberman na matriz
            refresh();
        }
        //Caso o usuário tenha pressionado espaço, a matriz auxiliar vai receber um valor indicando que ha uma bomba a ser inserida matrizjogo
        if(mapa->matrizaux[linha-d_horiz][coluna-d_vert]==BOMBA)
        {
            mapa->matrizjogo[linha-d_horiz][coluna-d_vert]=BOMBA; //Vai identificar a bomba inserida na matrizjogo
            mapa->matrizaux[linha-d_horiz][coluna-d_vert]=BRANCO; //Vai remover esse registro pendente da inserção da bomba na matrizaux
            *contar_bomb=TRUE; //Vai mandar uma flag dizendo que o tempo da bomba deve ser contado
            mvprintw(linha-d_horiz,coluna-d_vert,DESENHO_BOMBA); //Imprime o desenho da bomba
        }
    }

    return opcao;
}



//Recebe o ponteiro da variável relógio para decrementar os segundos
int tempo(RELOGIO* relogio)
{
    int morreu=FALSE;

    //Se os segundos atingirem zero, então o programa decrementa um do campo dos minutos e inicializa os segundos em 59
    if(relogio->segundos==0)
    {
        relogio->minutos=relogio->minutos-1;
        relogio->segundos=59;
    }
    else //Senão decrementa um segundo do ponteiro dos segundos
        relogio->segundos=relogio->segundos-1;
    if(relogio->segundos==0 && relogio->minutos==0) //Se os dois campos do relógio zerarem, o programa altera a flag morreu para que possa ser retirada uma vida do bomberman
        morreu=TRUE;
    return morreu;
}

//A função salvar é basicamente um mini-menu para que usuário escolha uma opção referente a sua partida. O programa retorna um valor para a função nível para que seja executada uma ação para o usuário
int salvar(void)
{
    WINDOW* janela;
    int carac, linha, opcao, escolheu=FALSE;

    janela=newwin(4,17,1,17);
    mvwprintw(janela,0,0,"  Deseja sair?");
    mvwprintw(janela,1,0,"  Sim e salvar.");
    mvwprintw(janela,2,0,"  Sim.");
    mvwprintw(janela,3,0,"> Não.");
    wrefresh(janela);
    chama_teclado();
    linha=3;
    opcao=CANCELAR;
    do
    {
        carac=getch();
        switch(carac)
        {
            case KEY_ESC:
                opcao=CANCELAR;
            case ENTER:
                escolheu=TRUE;
                werase(janela);
                wrefresh(janela);
                delwin(janela);
                break;
            case KEY_UP:
                if(linha>1)
                {
                    mvwprintw(janela,linha,0," ");
                    linha--;
                    mvwprintw(janela,linha,0,">");
                    wrefresh(janela);
                    opcao++;
                }
                break;
            case KEY_DOWN:
                if(linha<3)
                {
                    mvwprintw(janela,linha,0," ");
                    linha++;
                    mvwprintw(janela,linha,0,">");
                    wrefresh(janela);
                    opcao--;
                }
        }
    }
    while(!escolheu);

    return opcao;
}

//Essa função realiza o procedimento de testar a area atingida pela explosão da bomba
int explode_bomba (int raio, POSICAO local_bomb, MAPA* mapa)
{
    int aux=FALSE, morreu=FALSE;
    //Quando a bomba estoura, o programa insere um caractere identificando o fogo na matrizjogo
    mapa->matrizjogo[local_bomb.linha][local_bomb.coluna]=FOGO;
    //Printa em seguida a origem da explosão da bomba com um identificador para FOGO
    mvprintw(local_bomb.linha,local_bomb.coluna,DESENHO_FOGO);

    //A variável aux chama a função desenha bomba para fazer a explosão crescer ate o tamanho do raio que o jogador possui
    //Os quatro if's abaixo são para extender a explosão nos quatro sentidos da origem
    aux=desenha_fogo(mapa,1,local_bomb,raio,"linha");
    if(aux)
        morreu=TRUE;
    aux=desenha_fogo(mapa,-1,local_bomb,raio,"linha");
    if(aux)
        morreu=TRUE;
    aux=desenha_fogo(mapa,1,local_bomb,raio,"coluna");
    if(aux)
        morreu=TRUE;
    aux=desenha_fogo(mapa,-1,local_bomb,raio,"coluna");
    if(aux)
        morreu=TRUE;

    return morreu;
}

//Essa função é responsável por preencher a explosão dentro da matrizjogo bem como printar a explosão para que o usuário possa ver o poder de alcance de sua bomba
int desenha_fogo(MAPA* mapa, int sinal, POSICAO local_bomb,int raio, char eixo[])
{
    int avancar=TRUE, cont=1, linha, coluna, morreu=FALSE;

    do
    {
        //Verifica se o eixo não é igual a linha, se for, então o programa atualiza a linha e a coluna, e acerscenta o sinal através da multiplicação com o contador do laço para que a função verifique se vai desenhar para esquerda ou para direita
        if(!(strcmp("linha",eixo)))
        {
            linha=local_bomb.linha+(cont*sinal);
            coluna=local_bomb.coluna;
        }
        else //Se o eixo para preenchimento não for linha então o programa faz basicamente a mesma coisa só que para coluna, isto é, acresce a coordenada de um sinal para verificar se o preenchimento da explosão deve ser para cima ou para baixo
        {
            linha=local_bomb.linha;
            coluna=local_bomb.coluna+(cont*sinal);
        }
        //Verifica a atualização da nova posição da matriz e desenha a explosão na posição
        switch(mapa->matrizjogo[linha][coluna])
        {
            //Se nessa posição se encontrar um monstro então ele substitui por fogo esse valor na matriz jogo e grava a posição do monstro na matrizaux para depois pode realizar a contagem dos pontos
            case MONSTRO:
                mapa->matrizjogo[linha][coluna]=FOGO;
                mvprintw(linha,coluna,DESENHO_FOGO);
                mapa->matrizaux[linha][coluna]=MONSTRO;
                break;
            //Se na posição se encontrar uma obstrução en tao o programa substitui o valor na matriz jogo por fogo e ativa a flag avançar para evitar que a explosão atinja mais partes sendo que ja bateu em um objeto
            case OBSTRUCAO:
                mapa->matrizjogo[linha][coluna]=FOGO;
                mvprintw(linha,coluna,DESENHO_FOGO);
                avancar=FALSE;
                break;
            //Se a explosão encontrar o bomberman, ela o substitui por fogo e ativa a flag morreu para depois a função nivel lhe retirar uma vida e dar inicio a fase novamente
            case BOMBERMAN:
                mapa->matrizjogo[linha][coluna]=FOGO;
                mvprintw(linha,coluna,DESENHO_FOGO);
                morreu=TRUE;
                break;
            //Caso a próxima posição da explosão for uma parede ou uma bomba, a flag avançar é ativada para dizer que encontrou um obstaculo e que não deve seguir adiante
            case PAREDE:
            case BOMBA:
                avancar=FALSE;
                break;
            //Por padrao, se o elemento encontrado for diferente de SAIDA (o que neste ultimo caso seria BRANCO) o programa vai preenchendo valores FOGO na matriz para que possamos verificar aquilo que foi atingido pela bomba
            default:
                if(mapa->matrizjogo[linha][coluna]!=SAIDA)
                {
                    mapa->matrizjogo[linha][coluna]=FOGO;
                    mvprintw(linha,coluna,DESENHO_FOGO);
                }
        }
        cont++; //Incrementa o contador para que a bomba avance até o tamanho do raio disponivel ao usuário
    }
    while(avancar && cont<=raio); //Enquanto a flag que interrompe o avanço nao for ativada e a explosão não ficar com o tamanho do raio obtido pelo jogador, o programa segue no laço
    refresh();

    return morreu;
}
//Essa função fica responsável por limpar a explosão realizada pela função explode_bomba
int apaga_fogo(MAPA* mapa)
{
    int linha, coluna, pontos=0;

    for(linha=0;linha<NLINHAS;linha++)
        for(coluna=0;coluna<NCOLUNAS;coluna++)
        {
            //Verifica as posições na matriz das quais o raio da bomba conseguiu atingir
            if(mapa->matrizjogo[linha][coluna]==FOGO)
            {
                mapa->matrizjogo[linha][coluna]=mapa->matrizaux[linha][coluna]; //Troca o identificador de fogo por um valor que se encontra presente na matrizaux
                mapa->matrizaux[linha][coluna]=BRANCO; //Logo após o programa atualiza a matrizaux para que não haja elementos infinitos no jogo
                switch(mapa->matrizjogo[linha][coluna]) //Verifica qual o novo elemento presente no mapa e printa ele para o jogador ver aqui que foi modificado após a explosão e faz a contagem dos pontos obtidos por monstros mortos na explosão
                {
                    case POWERUP_BOMBA:
                        mvprintw(linha,coluna,DESENHO_MAIS_BOMBA);
                        break;
                    case POWERUP_RAIO:
                        mvprintw(linha,coluna,DESENHO_RAIO);
                        break;
                    case POWERUP_VIDA:
                        mvprintw(linha,coluna,DESENHO_VIDA);
                        break;
                    case SAIDA:
                        mvprintw(linha,coluna,DESENHO_SAIDA);
                        break;
                    //Caso o elemento que foi atingido pela exploso foi um monstro, o programa remove ele da matrizjogo e incrementa a varivel
                    case MONSTRO:
                        mapa->matrizjogo[linha][coluna]=BRANCO;
                        pontos=pontos+100;
                    case BRANCO:
                        mvprintw(linha,coluna," ");

                }
            }
        }
    return pontos;
}

//Função que realiza o movimento dos monstros no jogo
int movimenta_monstro(MAPA* mapa, int* pontos)
{
    POSICAO monstro[MAXMONSTROS], bomberman, nova_pos;
    int num_monstros=0, linha, coluna, achou=FALSE, dist_horiz, dist_vert, sinal_horiz, sinal_vert, lado, matou=FALSE, cont=0;

    linha=5;
    coluna=1;
    //Enquanto os monstros não acharem o bomberman e enquanto o programa não chegar na ultima linha
    while(!achou && linha<NLINHAS)
    {
        //Localiza o bomberman para que os monstros possam persegui-los a partir de sua localização
        if(mapa->matrizjogo[linha][coluna]==BOMBERMAN)
        {
            bomberman.linha=linha;
            bomberman.coluna=coluna;
            achou=TRUE;
        }

        //Se a leitura atingir a ultima coluna da matrizjogo, a variável linha é incrementada e coloca a variável coluna na primeira coluna da linha para que assim possa começar a ser lido uma nova linha
        if(coluna+1==NCOLUNAS)
        {
            linha++;
            coluna=1;
        }
        else //Senao segue verificando o próximo elemento da linha
            coluna++;
    }

    //Verifica onde cada monstro se encontra e grava suas posições em um vetor para que posteriormente isso seja utilizado para determinar o caminho mais curto para chegar ao bomberman
    for(linha=5;linha<NLINHAS;linha++)
        for(coluna=1;coluna<NCOLUNAS;coluna++)
        {
            if(mapa->matrizjogo[linha][coluna]==MONSTRO)
            {
                monstro[cont].linha=linha;
                monstro[cont].coluna=coluna;
                num_monstros++;
                cont++;
            }
        }

    //Esse laçom serve para calcular a distancia de cada monstro em relação ao bomberman e assim fazer com que o monstro decida ir pelo caminho mais curto
    for(cont=0;cont<num_monstros;cont++)
    {
        //Verifica a distancia da posição onde o monstro se econtra em relação ao bomberman em termos de distancia vertical e horizontal
        dist_horiz=bomberman.coluna-monstro[cont].coluna;
        dist_vert=bomberman.linha-monstro[cont].linha;
        //Verifica o sentido horizontal dessa distancia, ou seja, se o programa deve incrementar ou decrementar o campo que armazena a coordenada horizontal do plano
        if(dist_horiz!=0)
            sinal_horiz=dist_horiz/abs(dist_horiz);
        else //Se a distancia horizontal for zero, isso significa que eles se encontram na mesma coluna
            sinal_horiz=0;
        //Verifica o sentido vertical dessa distancia, ou seja, se o programa deve incrementar ou decrementar o campo que armazena a coordenada vertical do plano
        if(dist_vert!=0)
            sinal_vert=dist_vert/abs(dist_vert);
        else //Se a distancia vertical é zero, isso significa que eles estão na mesma linha
            sinal_vert=0;
        //Verifica qual é modulo maior entre as distancias vertical e horizontal ou se a coordenada horizontal está em zero. Se isso acontece o monstro opta por perseguir o bomberman atrvés do incremento/decremento através das linhas
        if((abs(dist_vert)<abs(dist_horiz) && dist_vert!=0) || dist_horiz==0)
        {
            nova_pos.linha=monstro[cont].linha+sinal_vert;
            nova_pos.coluna=monstro[cont].coluna;
        }
        else //Senão o programa opta por perseguir o bomberman atualizando sua posção através das colunas
        {
            nova_pos.coluna=monstro[cont].coluna+sinal_horiz;
            nova_pos.linha=monstro[cont].linha;
        }
        //Verifica se a nova posição do monstro é acessível
        switch(mapa->matrizjogo[nova_pos.linha][nova_pos.coluna])
        {
            //Verifica se nova posição coincide com um desses cinco elementos, o programa executa um comando para gerar um movimento aleatório até que este saia da parede
            case PAREDE:
            case BOMBA:
            case OBSTRUCAO:
            case SAIDA:
            case MONSTRO:
                lado=0+(rand()%((4-0)+1));
                switch(lado)
                {
                    //Verifica pra qual lado o monstro deve se movimentar e vai atualizando a nova posição para onde o monstro deve se mover para chegar até o bomberman
                    case DIREITA:
                        nova_pos.coluna=monstro[cont].coluna+1;
                        nova_pos.linha=monstro[cont].linha;
                        break;
                    case ESQUERDA:
                        nova_pos.coluna=monstro[cont].coluna-1;
                        nova_pos.linha=monstro[cont].linha;
                        break;
                    case CIMA:
                        nova_pos.linha=monstro[cont].linha-1;
                        nova_pos.coluna=monstro[cont].coluna;
                        break;
                    case BAIXO:
                        nova_pos.linha=monstro[cont].linha+1;
                        nova_pos.coluna=monstro[cont].coluna;
                        break;
                }
                //Verifica a nova posição onde o monstro se encontra
                switch(mapa->matrizjogo[nova_pos.linha][nova_pos.coluna])
                {
                    //Mesmo se houver um power-up na próxima posição, o monstro consome com o power-up, dificultando um pouco mais o jogo, e então ele printa o monstro na posição onde este se encontra
                    case POWERUP_BOMBA:
                    case POWERUP_VIDA:
                    case POWERUP_RAIO:
                    case BRANCO:
                        mapa->matrizjogo[monstro[cont].linha][monstro[cont].coluna]=BRANCO;
                        mvprintw(monstro[cont].linha,monstro[cont].coluna," ");
                        mapa->matrizjogo[nova_pos.linha][nova_pos.coluna]=MONSTRO;
                        mvprintw(nova_pos.linha,nova_pos.coluna,DESENHO_MONSTRO);
                        break;
                }
                break;
            //Se o próximo movimento movimento do monstro coincidir com uma área atingida pela explosão da bomba então o monstro é removido e o jogador ganha 100 pontos
            case FOGO:
                mapa->matrizjogo[monstro[cont].linha][monstro[cont].coluna]=BRANCO;
                mvprintw(monstro[cont].linha,monstro[cont].coluna," ");
                *pontos=*pontos+100;
                break;
            //Se o monstro encontrar o bomberman na proxima posição, é ativada uma flag para informar a função fase que o bomberman foi morto
            case BOMBERMAN:
                matou=TRUE;
            //Por padrão ele vai executando o movimento do monstro caso o programa não entre em nenhumn desses casos
            default:
                mapa->matrizjogo[monstro[cont].linha][monstro[cont].coluna]=BRANCO;
                mvprintw(monstro[cont].linha, monstro[cont].coluna, " ");
                mapa->matrizjogo[nova_pos.linha][nova_pos.coluna]=MONSTRO;
                mvprintw(nova_pos.linha,nova_pos.coluna,DESENHO_MONSTRO);
                break;
        }
    }
    refresh();
    return matou;
}

//Função para registrar os highscores dos jogadores
void recordes(void)
{
    FILE* arq_recordes;
    int i,j,colocacao=1, ordenado, contador=0, sair=FALSE, button=0;
    JOGADOR melhores[MAX_RECORDES]={" ",0}, teste;
    initscr();
    chama_teclado();
    box(stdscr,'|','-'); //Utilizado para desenhar uma caixa na qual será inseridos os nomes e os scores
    //Laço para desenhar a caixa e o preenchimento das linhas de inserção dos scores
    for(i=1;i<NLINHAS-1;i++)
        for(j=1;j<NCOLUNAS+3;j++)
        {
            if(i==2 || i==13)
                mvprintw(i,j,"-");
            else
                if((i>=3 && i<=12) && j==1)
                {
                    mvprintw(i,j,"%d",colocacao);
                    colocacao=colocacao+1;
                }
                else
                    if((j==3 || j==19) && i!=13)
                        mvprintw(i,j,"|");
        }
    //Identificadores de cada campo colocado na caixa
    mvprintw(0,15,"Recordes");
    mvprintw(1,4,"\tNome");
    mvprintw(1,20,"\tPontuação");
    mvprintw(14,2,"Pressione ENTER para voltar ao menu");

    //Abre o arquivo para ler os scores já salvos para exibição do usuário
    if(arq_recordes=fopen("highscores.bin","rb"))
    {
        sair=FALSE;
        do
        {
            //Se o programa consegue ler elementos ele segue inserindo estes nos campos da caixa para visualização
            if(fread(&melhores[contador],sizeof(teste),1,arq_recordes)==1)
            {
                mvprintw(contador+3,4,"%s",melhores[contador].nome);
                mvprintw(contador+3,20,"%d",melhores[contador].pontos);
                contador++;
            }
            else //Quando não houver mais elementos, ocorrerá um erro na leitura, e isso determinara o encerramento da leitura do arqui
                sair=TRUE;
        }
        while(!sair);
        fclose(arq_recordes);
    }
    else
    {   //Desenha as laterais adjacentes aos scores exibidos
        contador=MAX_RECORDES;
        for(i=0;i<contador;i++)
            mvprintw(i+3,4,"xxxx");
    }
    refresh();
    do
    {
        button=getch(); //Enquanto o usuário não pressionar enter, o highscores continua sendo exibido para o usuário
    }
    while(button!=ENTER);
    erase();
}

//Pega a maior pontuação do highscores e retorna esse valor a uma variável na função fase para que o jogador seja informado de qual é o maior recorde do jogo
int maior_pont(void)
{
    FILE* arq_maior;
    JOGADOR maior;
    int hi=0;

    if(arq_maior=fopen("highscores.bin","rb"))
    {
        fread(&maior,sizeof(maior),1,arq_maior);
        fclose(arq_maior);
        hi=maior.pontos;
    }
    return hi;
}

//Essa função é responsável por registrar o recorde do jogador caso este tenha sido maior que o menor recorde gravado no arquivo
void salvarecorde(int pontos)
{
    FILE* arq_highscore;
    JOGADOR melhores[MAX_RECORDES]={" ",0}, teste;
    int sair=FALSE, contador=0;

    //Abre o arquivo para leitura e le todos os registros do arquivo
    if(arq_highscore=fopen("highscores.bin","rb"))
    {
        do
        {
            if(fread(&melhores[contador],sizeof(teste),1,arq_highscore)==1)
                contador++;
            else
                sair=TRUE;
        }
        while(!sair);
        fclose(arq_highscore); //Fecha o arquivo após a leitura
    }
    organizador(melhores); //Chama essa função que faz um bubble sort. Decidirmos chamar está função antes da inserção do recorde para que ela sempre organize os recordes a cada inserção de dados de modo que os recordes continuem em ordem decrescente
    //Verifica se o recorde é maior que o menor recorde
    if(pontos>melhores[MAX_RECORDES-1].pontos)
    {
        //Informa para o usuário que ele entrou para o quadro de recorde e pega o nome dele para registrar juntamente com a pontuação obtida
        mvprintw(0,0,"Parabéns!\nVocê entrou para o quadro de recordes!\nQual o seu nome? ");
        keypad(stdscr,false);
        echo();
        nodelay(stdscr,false);
        curs_set(true);
        scanw("%[^\n]s",melhores[MAX_RECORDES-1].nome);
        melhores[MAX_RECORDES-1].pontos=pontos;
        organizador(melhores); //Chamamos essa função novamente pra ela verificar se esse recorde registrado não é maior que os outros recordes pré-existentes
        //Abre o arquivo e reescreve novamente todos os highscores de forma ordenada
        if(arq_highscore=fopen("highscores.bin","wb"))
        {
            fwrite(melhores,sizeof(teste),MAX_RECORDES,arq_highscore);
            fclose(arq_highscore);
        }
    }

}

//Função que realiza o método bubble sort no vetor
void organizador(JOGADOR melhores[MAX_RECORDES])
{
    int ordenado, i;
    JOGADOR aux;

    do
    {
        ordenado=TRUE;
        for (i=0;i<(MAX_RECORDES-1);i++)
        {
            if(melhores[i].pontos<melhores[i+1].pontos)
            {
                aux=melhores[i];
                melhores[i]=melhores[i+1];
                melhores[i+1]=aux;
                ordenado=FALSE;
            }
        }
    }
    while(!ordenado);
}
