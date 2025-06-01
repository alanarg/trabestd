#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<string.h>
#include<assert.h>

#define EMBEDDING_SIZE 128

typedef struct _reg {
    float embedding[EMBEDDING_SIZE];
    char id[100];
} treg;

void * aloca_reg(float embedding[], char id[]) {
    treg * reg;
    reg = malloc(sizeof(treg));
    for (int i = 0; i < EMBEDDING_SIZE; i++) {
        reg->embedding[i] = embedding[i];
    }
    strcpy(reg->id, id);
    return reg;
}


int comparador(void *a, void *b, int pos){
    float va = ((treg *)a)->embedding[pos];
    float vb = ((treg *)b)->embedding[pos];

    if (va < vb) return -1;
    else if (va > vb) return 1;
    else return 0;
}
double distancia(void * a, void *b){
    double soma = 0.0;
    for (int i = 0; i < EMBEDDING_SIZE; i++) {
        float dif = ((treg *)a)->embedding[i] - ((treg *)b)->embedding[i];
        soma += dif * dif;
    }
    return soma;
}

/*Definições desenvolvedor da biblioteca*/
typedef struct _node{
    void * key;
    struct _node * esq;
    struct _node * dir;
}tnode;

typedef struct _arv{
    tnode * raiz;
    int (*cmp)(void *, void *, int);
    double (*dist) (void *, void *);
    int k;
}tarv;



/*funções desenvolvedor da biblioteca*/

void kdtree_constroi(tarv * arv, int (*cmp)(void *a, void *b, int ),double (*dist) (void *, void *),int k){
    arv->raiz = NULL;
    arv->cmp = cmp;
    arv->dist = dist;
    arv->k = k;
}

/*teste*/
void test_constroi(){
    tarv arv;
    tnode node1;
    tnode node2;

    float emb1[EMBEDDING_SIZE] = {0};
    float emb2[EMBEDDING_SIZE] = {0};

    emb1[0] = 2.0;
    emb1[1] = 3.0;
    emb2[0] = 1.0;
    emb2[1] = 1.0;

    node1.key = aloca_reg(emb1, "Pessoa1");
    node2.key = aloca_reg(emb2, "Pessoa2");

    kdtree_constroi(&arv, comparador, distancia, EMBEDDING_SIZE);

    assert(arv.raiz == NULL);
    assert(arv.k == EMBEDDING_SIZE);
    assert(arv.cmp(node1.key, node2.key, 0) == 1);
    assert(arv.cmp(node1.key, node2.key, 1) == 1);
    assert(strcmp(((treg *)node1.key)->id, "Pessoa1") == 0);
    assert(strcmp(((treg *)node2.key)->id, "Pessoa2") == 0);

    free(node1.key);
    free(node2.key);
}


void _kdtree_insere(tnode **raiz, void * key, int (*cmp)(void *a, void *b, int),int profund, int k){
    if(*raiz == NULL){
        *raiz = malloc(sizeof(tnode));
        (*raiz)->key = key;
        (*raiz)->esq = NULL;
        (*raiz)->dir = NULL;
    }else{
        int pos = profund % k;
        if (cmp( (*(*raiz)).key , key ,pos) <0){
            _kdtree_insere( &((*(*raiz)).dir),key,cmp,profund + 1,k);
        }else{
            _kdtree_insere( &((*raiz)->esq),key,cmp,profund +1,k);
        }
    }
}

void kdtree_insere(tarv *arv, void *key){
    _kdtree_insere(&(arv->raiz),key,arv->cmp,0,arv->k);
}


void _kdtree_destroi(tnode * node){
    if (node!=NULL){
        _kdtree_destroi(node->esq);
        _kdtree_destroi(node->dir);
        free(node->key);
        free(node);
    }
}

void kdtree_destroi(tarv *arv){
    _kdtree_destroi(arv->raiz);
}


void _kdtree_busca(tarv *arv, tnode ** atual, void * key, int profund, double *menor_dist, tnode **menor){
    tnode ** lado_principal; 
    tnode ** lado_oposto;    
    if (*atual != NULL){
        double dist_atual = arv->dist((*atual)->key, key);
        if (dist_atual < *menor_dist){
            *menor_dist = dist_atual;
            *menor = *atual;
        }
        int pos = profund % arv->k;
        int comp = arv->cmp(key, (*atual)->key, pos);

    	printf("%s dist %4.3f menor_dist %4.3f comp %d\n", ((treg *)((tnode *)*atual)->key)->id, dist_atual, *menor_dist, comp);

        /* define lado principal para buscar */
        if (comp < 0){
            lado_principal =  &((*atual)->esq);
            lado_oposto    =  &((*atual)->dir); 
        }else{
            lado_principal =  &((*atual)->dir);
            lado_oposto    =  &((*atual)->esq); 
        }

        _kdtree_busca(arv, lado_principal, key, profund + 1, menor_dist, menor);

        /* Verifica se deve buscar também no outro lado*/

        if (comp*comp < *menor_dist) {
            printf("tentando do outro lado %f\n",comp*comp);
            _kdtree_busca(arv, lado_oposto, key, profund + 1, menor_dist, menor);
        }
    }
}


tnode * kdtree_busca(tarv *arv, void * key){
    tnode * menor = NULL;
    double menor_dist = DBL_MAX;
    _kdtree_busca(arv,&(arv->raiz),key,0,&menor_dist,&menor);
    return menor;
}

treg buscar_mais_proximo(tarv *arv, treg query) {
    double menor_dist = 1e20;
    tnode *menor = NULL;
    _kdtree_busca(arv, &(arv->raiz), &query, 0, &menor_dist, &menor);
    return *((treg *)(menor->key));
}


tarv arvore_global;

tarv* get_tree() {
    return &arvore_global;
}

void inserir_ponto(treg p) {
    treg *novo = malloc(sizeof(treg));
    *novo = p;  // cópia de estrutura
    kdtree_insere(&arvore_global,novo);
}
void kdtree_construir() {
    arvore_global.k = 2;
    arvore_global.dist = distancia;
    arvore_global.cmp = comparador;
    arvore_global.raiz = NULL;
}

void test_busca(){
    tarv arv;
    kdtree_constroi(&arv, comparador, distancia, EMBEDDING_SIZE);

    float embA[EMBEDDING_SIZE] = {0};
    float embB[EMBEDDING_SIZE] = {0};
    float embC[EMBEDDING_SIZE] = {0};
    float embD[EMBEDDING_SIZE] = {0};
    float embE[EMBEDDING_SIZE] = {0};
    float embF[EMBEDDING_SIZE] = {0};

    // Criando diferenças sutis nos vetores
    embA[0] = 10.0; embA[1] = 10.0;
    embB[0] = 20.0; embB[1] = 20.0;
    embC[0] = 1.0;  embC[1] = 10.0;
    embD[0] = 3.0;  embD[1] = 5.0;
    embE[0] = 7.0;  embE[1] = 15.0;
    embF[0] = 4.0;  embF[1] = 11.0;

    kdtree_insere(&arv, aloca_reg(embA, "a"));
    kdtree_insere(&arv, aloca_reg(embB, "b"));
    kdtree_insere(&arv, aloca_reg(embC, "c"));
    kdtree_insere(&arv, aloca_reg(embD, "d"));
    kdtree_insere(&arv, aloca_reg(embE, "e"));
    kdtree_insere(&arv, aloca_reg(embF, "f"));

    tnode *raiz = arv.raiz;
    assert(strcmp(((treg *)raiz->key)->id, "a") == 0); // pode variar pela ordem da inserção

    float query1[EMBEDDING_SIZE] = {0};
    query1[0] = 7.0; query1[1] = 14.0;
    tnode *resultado = kdtree_busca(&arv, aloca_reg(query1, "x"));
    assert(strcmp(((treg *)resultado->key)->id, "e") == 0);

    float query2[EMBEDDING_SIZE] = {0};
    query2[0] = 9.0; query2[1] = 9.0;
    resultado = kdtree_busca(&arv, aloca_reg(query2, "x"));
    assert(strcmp(((treg *)resultado->key)->id, "a") == 0);

    float query3[EMBEDDING_SIZE] = {0};
    query3[0] = 4.0; query3[1] = 5.0;
    resultado = kdtree_busca(&arv, aloca_reg(query3, "x"));
    assert(strcmp(((treg *)resultado->key)->id, "d") == 0);

    float query4[EMBEDDING_SIZE] = {0};
    query4[0] = 4.0; query4[1] = 9.0;
    resultado = kdtree_busca(&arv, aloca_reg(query4, "x"));
    assert(strcmp(((treg *)resultado->key)->id, "f") == 0);

    kdtree_destroi(&arv);
}


int main(void){
    test_constroi();
    test_busca();
    printf("SUCCESS!!\n");
    return EXIT_SUCCESS;
}
