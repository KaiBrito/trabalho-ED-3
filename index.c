#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct synonyms {
    char synonymsWord[48];
    struct synonyms *nextSynonyms; 
}synonyms;

typedef struct dictionaries {
    char keyWord[48];
    int height;
    struct dictionaries *left;
    struct dictionaries *rigth;
    synonyms *nextSynonyms; 
}dictionaries;

#define tableSize 1373

int hashFunction(char *str);
void initializeTable(dictionaries **hashTable);

dictionaries *createNode(char *str);
int bigger(int subLeft, int subRigth);
int heightNode(dictionaries *node);
int balancingFactor(dictionaries *node);

dictionaries *leftRotation(dictionaries *node);
dictionaries *rigthRotation(dictionaries *node);
dictionaries *leftDoubleRotation(dictionaries *node);
dictionaries *rigthDoubleRotation(dictionaries *node);
dictionaries *balance(dictionaries *node);

void insertTable(dictionaries **hashTable, char *str1, char *str2);
dictionaries *insertDictionaries(dictionaries *root, char *str);
void insertSynonyms(dictionaries *root, char *str1, char *str2);

dictionaries *searchDictionaries(dictionaries *root, char *str);
synonyms *searchSynonyms(synonyms *pointerSynonyms, char *str);
void searchWord(dictionaries **tabHash, char *str);

dictionaries *removeNode(dictionaries *root, char *str);
void removeWord(dictionaries **tabHash, char *str);
void removeWords(dictionaries **tabHash, char *str1, char *str2);

void loadFiles(dictionaries **tabHash);
void saveFiles(dictionaries **tabHash);
void saveWords(dictionaries *root, FILE *file);


int main() {
    dictionaries *hashTable[tableSize];
    char option[103], command[7], str1[48], str2[48];

    initializeTable(hashTable);
    loadFiles(hashTable);

    do
    {
        scanf(" %[^\n]", option);
        sscanf(option, "%s %s %s", command, str1, str2);

        if (strcmp(command, "insere") == 0) {
            insertTable(hashTable, str1, str2);
            insertTable(hashTable, str2, str1);
        }
        if (strcmp(command, "busca") == 0) {
            searchWord(hashTable, str1);
        }
        if (strcmp(command, "remove") == 0) {
            if (strlen(str2) > 1) {
                removeWords(hashTable, str1, str2);
                removeWords(hashTable, str2, str1);
            } else {
                removeWord(hashTable, str1);
            }        
        }

        strcpy(str2, "");  // Limpa a variável str2
        
    } while (strcmp(command, "fim") != 0);

    saveFiles(hashTable);

    return 0;
}

// Método para calcular o valor hash de uma string
int hashFunction(char *str) {
    int i, sum = 0; 

    for (i = 0; i < strlen(str); i++)
        sum += str[i];
    
    return sum%tableSize;
}

// Método para inicializar toda a tabela hash com valor NULL
void initializeTable(dictionaries **hashTable) {
    int i;

    for (i = 0; i < tableSize; i++)
        hashTable[i] = NULL;  
}

// Método cria um nó e retorna ele mesmo 
dictionaries *createNode(char *str){
    dictionaries *node = (dictionaries *) malloc(sizeof(dictionaries));

    if (node != NULL){
        strcpy(node->keyWord, str);
        node->height = 1;
        node->left = NULL;
        node->rigth = NULL;
        node->nextSynonyms = NULL;
    } else 
        printf("Opps, falha na alocacao de memoria!\n");

    return node;  
}

// Método para descobrir qual a maior sub-arvore
int bigger(int subLeft, int subRigth){
    if (subLeft > subRigth)
        return subLeft;
    else 
        return subRigth;
}

// Método para descobrir a altura de um nó
int heightNode(dictionaries *node){
    if (node == NULL)
        return -1;
    else 
        return node->height;
    
}

// Calcula o fb de um nó
int balancingFactor(dictionaries *node){
    if (node != NULL)
        return heightNode(node->left) - heightNode(node->rigth);
    else 
        return 0;
}

// Método para fazer uma rotação à esquerda
dictionaries *leftRotation(dictionaries *node){
    dictionaries *v, *u;

    // v = filho direito do node e u = filho de esquerdo de v
    v = node->rigth;
    u = v->left;

    // v se torna o pai do node e o node recebe o filho esquerdo de v como sendo seu filho direito
    v->left = node;
    node->rigth = u;

    // Recalcula a altura de ambos 
    node->height = bigger(heightNode(node->left), heightNode(node->rigth)) + 1;
    v->height = bigger(heightNode(v->left), heightNode(v->rigth)) + 1;

    return v;
}

// Método para fazer uma rotação à direita
dictionaries *rigthRotation(dictionaries *node){
    dictionaries *v, *u;

    // v = filho esquerdo do node e u = filho direito de v
    v = node->left;
    u = v->rigth;

    // v se torna o pai do node e o node recebe o filho direito de v como sendo seu filho esquerdo
    v->rigth = node;
    node->left = u;

    // Recalcula a altura de ambos 
    node->height = bigger(heightNode(node->left), heightNode(node->rigth)) + 1;
    v->height = bigger(heightNode(v->left), heightNode(v->rigth)) + 1;

    return v;
}

// Método para fazer uma rotação dupla à esquerda
dictionaries *leftDoubleRotation(dictionaries *node){
    node->rigth = rigthRotation(node->rigth);
    return leftRotation(node);
}

// Método para fazer uma rotação dupla à direita
dictionaries *rigthDoubleRotation(dictionaries *node){
    node->left = leftRotation(node->left);
    return rigthRotation(node);
}

// Método para fazer o balanceamento de um nó desregulado 
dictionaries *balance(dictionaries *node){
    int fb = balancingFactor(node);

    // Rotação à esquerda
    if (fb < -1 && balancingFactor(node->rigth) <= 0)
        node = leftRotation(node);
    // Rotação à direita
    else if (fb > 1 && balancingFactor(node->left) >= 0)
        node = rigthRotation(node);
    // Rotação dupla à direita
    else if (fb > 1 && balancingFactor(node->left) < 0)
        node = rigthDoubleRotation(node);
    // Rotação dupla à esquerda
    else if (fb < -1 && balancingFactor(node->rigth) > 0)
        node = leftDoubleRotation(node);

    return node;
}

// Método para definir se palavra será inserida apenas como sinônimo ou como palavra-chave
void insertTable(dictionaries **hashTable, char *str1, char *str2) {
    int id;
    dictionaries *verify;

    id = hashFunction(str1);
    // Verifica se a palavra chave já existe na tabela hash
    verify = searchDictionaries(hashTable[id], str1);

    if (verify == NULL)
        hashTable[id] = insertDictionaries(hashTable[id], str1);
         
    insertSynonyms(hashTable[id], str1, str2);     
}

// Método para inserir elementos na AVL
dictionaries *insertDictionaries(dictionaries *root, char *str){
    if (root == NULL)
        root = createNode(str);
    else {
        if (strcmp(root->keyWord, str) == 1)
            root->left = insertDictionaries(root->left, str);  
        else
            root->rigth = insertDictionaries(root->rigth, str);
    }

    // Recalcula a altura de todos os nós entre a raiz e o novo nó
    root->height = bigger(heightNode(root->left), heightNode(root->rigth)) + 1;

    // Verifica se o nó atual está desregulado 
    root = balance(root);

    return root;
}

// Método para adicionar sinônimos a um nó
void insertSynonyms(dictionaries *root, char *str1, char *str2) {
    synonyms *current, *new, *back, *verify;
    dictionaries *pointerDicti;

    new = (synonyms *) malloc(sizeof(synonyms));
    if (new == NULL) {
		fprintf(stderr, "Opps, falha na alocação de memoria!\n");
		return;
	}

    pointerDicti = searchDictionaries(root, str1);

    // Verifica se a palavra já existe na lista de sinonimos
    verify = searchSynonyms(pointerDicti->nextSynonyms, str2);

    if (verify == NULL) {
        current = pointerDicti->nextSynonyms;
        back = NULL;

        while (current != NULL && strcmp(current->synonymsWord, str2) < 0) {
            back = current;
            current = current->nextSynonyms; 
        }

        if (back == NULL) {
            pointerDicti->nextSynonyms = new;
        } else {
            back->nextSynonyms = new; 
        }

        // Dados do nó
        strcpy(new->synonymsWord, str2);
        new->nextSynonyms = current;
    }
}

// Método para procurar por um elemento informado pelo o usúario
dictionaries *searchDictionaries(dictionaries *root, char *str){
    if (root == NULL)
        return NULL;
    else if (strcmp(root->keyWord, str) == 0)
        return root;
    else if (strcmp(root->keyWord, str) == 1)
        searchDictionaries(root->left, str);
    else 
        searchDictionaries(root->rigth, str);  
}

// Método para fazer a busca de sinônimos
synonyms *searchSynonyms(synonyms *pointerSynonyms, char *str) {
    synonyms *current = pointerSynonyms;

    while (current != NULL && strcmp(current->synonymsWord, str) != 0)
        current = current->nextSynonyms;  

    if (current == NULL)
        return NULL;
    else 
        return current; 
}

// Método para buscar por uma palavra e imprimir na tela todos os seus sinônimos
void searchWord(dictionaries **tabHash, char *str){
    dictionaries *verify;
    synonyms *word;
    int id;

    id = hashFunction(str);

    verify = searchDictionaries(tabHash[id], str);

    if (verify != NULL) {
        word = verify->nextSynonyms;

        while (word != NULL) {
            printf("%s\n", word->synonymsWord);
            word = word->nextSynonyms;
        }      
    }
    else
        printf("hein?\n");   
}

// Método para remover um nó da árvore
dictionaries *removeNode(dictionaries *root, char *str){
    dictionaries *aux;

    if (root == NULL)
        return NULL;
    else 
    {
        if (strcmp(root->keyWord, str) == 0){
            // remove o nó folha
            if (root->left == NULL && root->rigth == NULL)
            {
                free(root);
                return NULL;
            } else {
                // Remove o nó que só possui um filho 
                if (root->left == NULL || root->rigth == NULL)
                {
                    if (root->left != NULL)
                        aux = root->left;
                    else 
                        aux = root->rigth;

                    free(root);
                    return aux;
                } else { 
                    // Remove o nó que possuí 2 filhos
                    aux = root->left;

                    while (aux->rigth != NULL)
                        aux = aux->rigth;

                    strcpy(root->keyWord, aux->keyWord);
                    root->nextSynonyms = aux->nextSynonyms;
                    strcpy(aux->keyWord, str);

                    root->left = removeNode(root->left, str);
                    return root;
                }   
            }
        } else {
            if (strcmp(root->keyWord, str) == 1)
                root->left = removeNode(root->left, str);
            else 
                root->rigth = removeNode(root->rigth, str);
        }

        // Recalcula a altura de todos os nós entre a raiz e o novo nó
        root->height = bigger(heightNode(root->left), heightNode(root->rigth)) + 1;
        
        // Verifica se há algum nó para balancear na árvore
        root = balance(root);

        return root;
    }  
}

// Método para fazer a remoção de uma palavra-chave e todos seus sinônimos
void removeWord(dictionaries **tabHash, char *str){
    dictionaries *pointerNode;
    synonyms *currentS, *backS;
    int id;

    id = hashFunction(str);
    
    pointerNode = searchDictionaries(tabHash[id], str);

    if (pointerNode != NULL) {
        currentS = pointerNode->nextSynonyms;

        // Libera todos os sinônimos da palavra chave
        while (currentS != NULL) {
            backS = currentS;
            currentS = currentS->nextSynonyms;
            free(backS);
        }

        // Remove o nó da estrutura
        tabHash[id] = removeNode(tabHash[id], str);
    }
}

// Método para fazer a remoção de uma palavra-chave e um sinônimo 
void removeWords(dictionaries **tabHash, char *str1, char *str2){
    dictionaries *pointerNode;
    synonyms *currentS, *backS;
    int id;

    id = hashFunction(str1);

    pointerNode = searchDictionaries(tabHash[id], str1);

    if (pointerNode != NULL) {
        currentS = pointerNode->nextSynonyms;
        backS = NULL;

        while (currentS != NULL && strcmp(currentS->synonymsWord, str2) != 0) {
            backS = currentS;
            currentS = currentS->nextSynonyms;
        }

        // Excluí o sinonimo
        if (backS == NULL) {
            pointerNode->nextSynonyms = currentS->nextSynonyms;
        } else {
            backS->nextSynonyms = currentS->nextSynonyms;
        }

        // Libera memória
        free(currentS);

        // Verifica se a lista ficou vazia e remove ele da estrutura caso tenha ficado vazia
        if (pointerNode->nextSynonyms == NULL)
            tabHash[id] = removeNode(tabHash[id], str1);
    }
}

// Método para carregar os dados anteriores
void loadFiles(dictionaries **tabHash){
    FILE *loadWords;
    char str1[48], str2[48];

    loadWords = fopen("dicionario.txt", "a+");

    if (loadWords == NULL) {
		printf("Infelizmente seu arquivo dicionario.txt não foi aberto!\n");
		return;
	}

    fscanf(loadWords, "%s %s", str1, str2);

    while (!feof(loadWords)) {
        insertTable(tabHash, str1, str2);
        fscanf(loadWords, "%s %s", str1, str2);
    }  

    fclose(loadWords);
}

// Método para salvar os dados ao fim do programa
void saveFiles(dictionaries **tabHash){
    FILE *saveWord;
    int i;

    saveWord = fopen("dicionario.txt", "w+");

    if (saveWords == NULL) {
		printf("Infelizmente seu arquivo dicionario.txt não foi aberto!\n");
		return;
	}

    for (i = 0; i < tableSize; i++)
        saveWords(tabHash[i], saveWord);    
    
    fclose(saveWord);
}


// Método para fazer a leitura dos dados em um arquivo
void saveWords(dictionaries *root, FILE *file){
    synonyms *node, *memoryFree;

    if (root != NULL){
        node = root->nextSynonyms;
        while (node != NULL){
            memoryFree = node;

            fprintf(file, "%s %s\n", root->keyWord, node->synonymsWord);
            node = node->nextSynonyms;
            
            //Libera memória
            free(memoryFree);
        }
        
        saveWords(root->left, file);
        saveWords(root->rigth, file);        
    } 
}