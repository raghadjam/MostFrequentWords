#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

//Raghad Iyad jamhour
//1220212, section:1

#define MAX_WORD_LEN 100

typedef struct AVLnode *AVLNode;

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
void insertWord(sortedArray *sort, const char *word, int frequency);
void mergeResults(sortedArray *result_array, sortedArray *local_arrays[], int numChunks);
void printTopWords(sortedArray *sort);
void writeTopWordsToFile(const char *filename, sortedArray *sort);
void readWordsFromFile(const char *filename, sortedArray *sort);

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
    if (P == NULL) return -1;
    else return P->height;
}

int Max(int Lhs, int Rhs)
{
    return Lhs > Rhs ? Lhs : Rhs;
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
        if (T == NULL) printf("Out of space.");
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

void insertWord(sortedArray *sort, const char *word, int frequency)
{
    sort->wordFreq = realloc(sort->wordFreq, sizeof(array) * (sort->size + 1));
    strcpy(sort->wordFreq[sort->size].word, word);
    sort->wordFreq[sort->size].frequency = frequency;
    sort->size++;
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

int compare(const void *a, const void *b)
{
    return ((array *)b)->frequency - ((array *)a)->frequency;
}

void printTopWords(sortedArray *sort)
{
    for (int i = 0; i < 10 && i < sort->size; i++)
    {
        printf("%s: %d\n", sort->wordFreq[i].word, sort->wordFreq[i].frequency);
    }
}

void writeTopWordsToFile(const char *filename, sortedArray *sort)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }
    for (int i = 0; i < 100 && i < sort->size; i++)   //writing Top 100 words of each child
    {
        fprintf(file, "%s %d\n", sort->wordFreq[i].word, sort->wordFreq[i].frequency);
    }
    fclose(file);
}

void readWordsFromFile(const char *filename, sortedArray *sort)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    char word[MAX_WORD_LEN];
    int frequency;
    while (fscanf(file, "%s %d", word, &frequency) != EOF)
    {
        insertWord(sort, word, frequency);
    }

    fclose(file);
}

void mergeResults(sortedArray *result_array, sortedArray *local_arrays[], int numChunks)
{
    for (int i = 0; i < numChunks; i++)
    {
        for (int j = 0; j < local_arrays[i]->size; j++)
        {
            if (local_arrays[i]->wordFreq[j].frequency != 0)
            {
                int found = 0;
                for (int k = 0; k < result_array->size; k++)
                {
                    if (strcmp(result_array->wordFreq[k].word, local_arrays[i]->wordFreq[j].word) == 0)
                    {
                        result_array->wordFreq[k].frequency += local_arrays[i]->wordFreq[j].frequency;
                        found = 1;
                        break;
                    }
                }

                if (!found)
                {
                    insertWord(result_array, local_arrays[i]->wordFreq[j].word, local_arrays[i]->wordFreq[j].frequency);
                }
            }
        }
    }

    qsort(result_array->wordFreq, result_array->size, sizeof(array), compare);

    printTopWords(result_array);
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

    pid_t pids[numChunks];
    sortedArray *local_arrays[numChunks];
    char *buffers[numChunks];
    char *filenames[numChunks];

    for (int i = 0; i < numChunks; i++)
    {
        filenames[i] = malloc(50);
        sprintf(filenames[i], "output_chunk_%d.txt", i);

        local_arrays[i] = create();
        long currentChunkSize = chunkSize + remainder;
        buffers[i] = malloc(currentChunkSize + 1);

        fseek(file, i * chunkSize + remainder, SEEK_SET);
        fread(buffers[i], 1, currentChunkSize, file);
        buffers[i][currentChunkSize] = '\0';

        if ((pids[i] = fork()) == 0)
        {
            AVLNode tree = NULL;
            char word[MAX_WORD_LEN];
            char *ptr = buffers[i];

            while (*ptr != '\0')
            {
                while (*ptr == ' ' || *ptr == '\n' || *ptr == '\t')
                {
                    ptr++;
                }

                int j = 0;
                while (*ptr != '\0' && *ptr != ' ' && *ptr != '\n' && *ptr != '\t')
                {
                    word[j++] = *ptr++;
                }
                word[j] = '\0';

                if (j > 0)
                {
                    tree = insert(word, tree);
                }
            }

            traverseInOrder(tree, local_arrays[i]);
            qsort(local_arrays[i]->wordFreq, local_arrays[i]->size, sizeof(array), compare);
            writeTopWordsToFile(filenames[i], local_arrays[i]);

            exit(0);
        }
    }

    for (int i = 0; i < numChunks; i++)
    {
        waitpid(pids[i], NULL, 0);
        free(buffers[i]);
    }

    sortedArray *result_array = create();

    for (int i = 0; i < numChunks; i++)
    {
        readWordsFromFile(filenames[i], local_arrays[i]);
    }

    mergeResults(result_array, local_arrays, numChunks);

    for (int i = 0; i < numChunks; i++)
    {
        free(local_arrays[i]->wordFreq);
        free(filenames[i]);
    }

    fclose(file);
}

int main()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    partitionFile("text8.txt", 8);  // number of processes

    gettimeofday(&end, NULL);
    double timeTaken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Execution took %f seconds to complete.\n", timeTaken);

    return 0;
}

