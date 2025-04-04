#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    tipoI,
    tipoJ,
    tipoR,
    outros
} classe;

typedef struct {
    classe tipoInst;
    char instChar[17];
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
} instrucao;
//funcao pra converter a string pra um int pq string ia ser mais dificil de manipular
int binarioInt(const char *binario) {
    int valor = 0;
    for (int i = 0; binario[i] != '\0'; i++) { //percorrer a string até \0
        valor = valor * 2 + (binario[i] - '0'); // deslocamento pra esquerda 
    //tipo se for 0001 (1), então 1 * 2 = 2 (em binário 0010)
    }
    return valor;
}
//funcao para decodificar a instrucao
void decodificar(instrucao *inst) {
    int bin_inst = binarioInt(inst->instChar); //converte string bin pra int
    inst->opcode = (bin_inst >> 12) & 0xF; // pega o OPCODE bits 15-12, 4 mais significativos 
    
    if (inst->opcode == 0x0) {  // Tipo R
        inst->tipoInst = tipoR;
        inst->rs = (bin_inst >> 9) & 0x7;
        inst->rt = (bin_inst >> 6) & 0x7;
        inst->rd = (bin_inst >> 3) & 0x7; //reg destino
        inst->funct = bin_inst & 0x7; //funcao
        
        switch (inst->funct) { //vai definir o nome da instrucao
            case 0x0: strcpy(inst->instChar, "add"); break;
            case 0x2: strcpy(inst->instChar, "sub"); break;
            case 0x4: strcpy(inst->instChar, "and"); break;
            case 0x5: strcpy(inst->instChar, "or"); break;
            default: inst->tipoInst = outros;
        }
    }
    else if (inst->opcode == 0x2) {  // Tipo J
        inst->tipoInst = tipoJ;
        inst->addr = bin_inst & 0xFF; // 7-0 endereco
        strcpy(inst->instChar, "j");
    }
    else {  // Tipo I
        inst->tipoInst = tipoI;
        inst->rs = (bin_inst >> 9) & 0x7; //11-9
        inst->rt = (bin_inst >> 6) & 0x7; //8-6
        inst->imm = bin_inst & 0x3F; //5-0
        
        if (inst->imm & 0x20) { //ve se o 6° bit é 1 (negativo)
            inst->imm |= 0xFFFFFFC0;  // Extensão de sinal para 32 bits
        }
        
        switch (inst->opcode) {
            case 0x4: strcpy(inst->instChar, "addi"); break;
            case 0x8: strcpy(inst->instChar, "beq"); break;
            case 0xB: strcpy(inst->instChar, "lw"); break;
            case 0xF: strcpy(inst->instChar, "sw"); break;
            default: inst->tipoInst = outros;
        }
    }
}
//carregar instrucoes de um arquivo abrindo o arquivo, lendo até 11 intrucoes
//copia para a memória e decodifica cada instrucao
void salvaInst(instrucao *mem, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nomeArquivo);
        exit(1);
    }

    char binario[17];
    for (int i = 0; i < 11; i++) {
        if (fgets(binario, sizeof(binario), arquivo) != NULL) {
            binario[strcspn(binario, "\n")] = '\0';
            strcpy(mem[i].instChar, binario);
            decodificar(&mem[i]);
        }
    }
    fclose(arquivo);
}
//imprime oq tem na memoria, a posicao, nome da instrucao, tipo e registradores
// imediato e endereco
void imprimeMem(instrucao *mem) {
    printf("Conteúdo da memória decodificado:\n");
    for (int i = 0; i < 11; i++) {
        printf("Memória[%d]: %s | ", i, mem[i].instChar);
        
        switch (mem[i].tipoInst) {
            case tipoR:
                printf("Tipo-R | rs=$%d, rt=$%d, rd=$%d, funct=%d\n",
                       mem[i].rs, mem[i].rt, mem[i].rd, mem[i].funct);
                break;
            case tipoI:
                printf("Tipo-I | rs=$%d, rt=$%d, imm=%d\n",
                       mem[i].rs, mem[i].rt, mem[i].imm);
                break;
            case tipoJ:
                printf("Tipo-J | addr=0x%X\n", mem[i].addr);
                break;
            default:
                printf("Tipo desconhecido\n");
        }
    }
}
// aloca memoria pra 11 instrucoes, decodifica as intrucoes do arquivo, imprime
// e libera a memoria alocada
int main() {
    instrucao *mem = (instrucao *)malloc(11 * sizeof(instrucao));
    salvaInst(mem, "instrucoes.txt");
    imprimeMem(mem);
    free(mem);
    return 0;
}
