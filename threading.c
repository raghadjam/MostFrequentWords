#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#define MAX_WORD_LEN 100

//Raghad Iyad jamhour
//1220212, section:1

typedef struct AVLnode *AVLNode;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



typedef struct array
{
    char word[MAX_WORD_LEN];
    int frequency;
} array;

typedef struct sortedArray
{
    array *wordFreq;
    int size;
} sortedArray;

struct AVLnode
{
    char word[MAX_WORD_LEN];
    AVLNode left;
    AVLNode right;
    int height;
    int frequency;
};

AVLNode makeEmpty(AVLNode T);
AVLNode insert(char s[MAX_WORD_LEN], AVLNode T);
void traverseInOrder(AVLNode t, sortedArray *sort);
sortedArray *create();
sortedArray* printTopWords(sortedArray *sort);
void *threadFunc(void *buffer);
sortedArray *arrayThread = NULL;

void insertWord(sortedArray *sort, const char *word, int frequency)
{
    sort->wordFreq = realloc(sort->wordFreq, sizeof(array) * (sort->size + 1));
    strcpy(sort->wordFreq[sort->size].word, word);
    sort->wordFreq[sort->size].frequency = frequency;
    sort->size++;
}

AVLNode makeEmpty(AVLNode T)
{
    if (T != NULL)
    {
        makeEmpty(T->left);
        makeEmpty(T->right);
        free(T);
    }
    return NULL;
}

int height(AVLNode P)
{
    if (P == NULL)
        return -1;
    else
        return P->height;
}

int Max(int Lhs, int Rhs)
{
    return Lhs > Rhs ? Lhs : Rhs;
}

void traverseInOrder(AVLNode t, sortedArray *sort)
{
    if (t != NULL)
    {
        traverseInOrder(t->left, sort);
        insertWord(sort, t->word, t->frequency);
        traverseInOrder(t->right, sort);

    }
}


AVLNode singleRotateLeft(AVLNode T2)
{
    AVLNode T1;

    T1 = T2->left;
    T2->left = T1->right;
    T1->right = T2;

    T2->height = Max(height(T2->left), height(T2->right)) + 1;
    T1->height = Max(height(T1->left), T2->height) + 1;

    return T1;
}

AVLNode singleRotateRight(AVLNode T1)
{
    AVLNode T2;

    T2 = T1->right;
    T1->right = T2->left;
    T2->left = T1;

    T1->height = Max(height(T1->left), height(T1->right)) + 1;
    T2->height = Max(height(T2->right), T1->height) + 1;

    return T2;
}

AVLNode doubleRotateLeft(AVLNode T1)
{
    T1->left = singleRotateRight(T1->left);
    return singleRotateLeft(T1);
}

AVLNode doubleRotateRight(AVLNode T1)
{
    T1->right = singleRotateLeft(T1->right);
    return singleRotateRight(T1);
}

AVLNode insert(char s[MAX_WORD_LEN], AVLNode T)
{
    if (T == NULL)
    {
        T = malloc(sizeof(struct AVLnode));
        if (T == NULL)
            printf("Out of space.");
        else
        {
            strcpy(T->word, s);
            T->height = 0;
            T->frequency = 1;
            T->left = T->right = NULL;
        }
    }
    else if (strcmp(s, T->word) < 0)
    {
        T->left = insert(s, T->left);
        if (height(T->left) - height(T->right) == 2)
            if (strcmp(s, T->left->word) < 0)
                T = singleRotateLeft(T);
            else
                T = doubleRotateLeft(T);
    }
    else if (strcmp(s, T->word) > 0)
    {
        T->right = insert(s, T->right);
        if (height(T->right) - height(T->left) == 2)
            if (strcmp(s, T->right->word) > 0)
                T = singleRotateRight(T);
            else
                T = doubleRotateRight(T);
    }
    else
    {
        T->frequency++;
    }

    T->height = Max(height(T->left), height(T->right)) + 1;
    return T;
}

sortedArray *create()
{
    sortedArray *sort = malloc(sizeof(sortedArray));
    sort->wordFreq = NULL;
    sort->size = 0;
    return sort;
}

int compare(const void *a, const void *b)
{
    return ((array *)b)->frequency - ((array *)a)->frequency;
}


sortedArray* printTopWords(sortedArray *sort)
{
    for (int i = 0; i < 100; i++)  //to take all the max values when threads are 8
    {
        insertWord(arrayThread, sort->wordFreq[i].word, sort->wordFreq[i].frequency);
    }
}

void* print(sortedArray *sort)
{
    for (int i = 0; i < 10; i++)
    {
        printf("%s: %d\n", sort->wordFreq[i].word, sort->wordFreq[i].frequency);
    }
}


void mergeThreads(sortedArray *sort) {
    sortedArray *arrayThreadSorted = create();

    for (int i = 0; i < 800; i++) {
        if (sort->wordFreq[i].frequency != 0) {
            for (int j = i + 1; j < 800; j++) {
                if (strcmp(sort->wordFreq[i].word, sort->wordFreq[j].word) == 0) {
                    sort->wordFreq[i].frequency += sort->wordFreq[j].frequency;
                    sort->wordFreq[j].frequency = 0;
                }
            }
        }
    }

    for (int i = 0; i < 800; i++) {
        if (sort->wordFreq[i].frequency > 0) {
            insertWord(arrayThreadSorted, sort->wordFreq[i].word, sort->wordFreq[i].frequency);
        }
    }

    qsort(arrayThreadSorted->wordFreq, arrayThreadSorted->size, sizeof(array), compare);
    print(arrayThreadSorted);
}



void *threadFunc(void *buffer)
{
    char *chunk = (char *)buffer;
    AVLNode tree = NULL;
    char word[MAX_WORD_LEN];
    char *ptr = chunk;

    while (*ptr != '\0')
    {
        while (*ptr == ' ' || *ptr == '\n' || *ptr == '\t')
            ptr++;

        int i = 0;
        while (*ptr != '\0' && *ptr != ' ' && *ptr != '\n' && *ptr != '\t')
        {
            word[i++] = *ptr++;
        }
        word[i] = '\0';

        if (i > 0)
        {
            tree = insert(word, tree);
        }
    }

    sortedArray *sort = create();
    traverseInOrder(tree, sort);
    qsort(sort->wordFreq, sort->size, sizeof(array), compare);

    pthread_mutex_lock(&mutex);
    printTopWords(sort);
    pthread_mutex_unlock(&mutex);


    free(sort->wordFreq);
    free(sort);
    makeEmpty(tree);

    return buffer;
}

void partitionFile(const char *filename, int numChunks)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    long chunkSize = fileSize / numChunks;
    long remainder = fileSize % numChunks;

    pthread_t threads[numChunks];
    char *buffers[numChunks];

    for (int i = 0; i < numChunks; i++)
    {
        long currentChunkSize = chunkSize + remainder;
        buffers[i] = malloc(currentChunkSize + 1);

        fseek(file, i * chunkSize + remainder, SEEK_SET);
        fread(buffers[i], 1, currentChunkSize, file);

        buffers[i][currentChunkSize] = '\0';
        pthread_create(&threads[i], NULL, threadFunc, buffers[i]);
    }

    for (int i = 0; i < numChunks; i++)
    {
        pthread_join(threads[i], NULL);
        free(buffers[i]);
    }

    fclose(file);
}

int main() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    arrayThread = create();
    partitionFile("text8.txt", 8);  //number of threads
    mergeThreads(arrayThread);

    gettimeofday(&end, NULL);
    double timeTaken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Execution took %f seconds to complete.\n", timeTaken);

    return 0;
}
