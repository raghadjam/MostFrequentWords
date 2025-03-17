#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//Raghad Iyad jamhour
//1220212, section:1

typedef struct AVLnode *AVLNode;

typedef struct array
{
    char word[100];
    int frequency;
} array;

typedef struct sortedArray
{
    array *wordFreq;
    int size;   // Current size of the dynamic array
} sortedArray;

struct AVLnode
{
    char word[50];
    AVLNode left;
    AVLNode right;
    int height;
    int frequency;
};

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

AVLNode insert(char s[50], AVLNode T)
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

void printTopWords(sortedArray *sort)
{
    for (int i = 0; i < 10; i++)
    {
        printf("%s: %d\n", sort->wordFreq[i].word, sort->wordFreq[i].frequency);
    }
}



int main()
{
    clock_t t;
    t = clock();

    AVLNode tree = NULL;
    tree = makeEmpty(tree);
    sortedArray *sort = create();

    char word[50];
    FILE *pF = fopen("text8.txt", "r");
    if (pF == NULL)
    {
        printf("Error opening file!\n");
    }
    else
    {
        while (fscanf(pF, "%49s", word) == 1)
        {
                tree = insert(word, tree);
        }
        fclose(pF);
    }

    t = clock() - t;
    traverseInOrder(tree, sort);
    qsort(sort->wordFreq, sort->size, sizeof(array), compare);

    // Print top 10 words by frequency
    printTopWords(sort);

    double timeTaken = ((double)t) / CLOCKS_PER_SEC; // in seconds
    printf("Execution took %f seconds to complete.\n", timeTaken);

    // Free allocated memory
    free(sort->wordFreq);
    free(sort);
    makeEmpty(tree); // Free AVL tree memory

    return 0;
}

