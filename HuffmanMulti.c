#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "timer.h"

#define len(x) ((long long) log10(x) + 1)

/* Node of the huffman tree */
struct node 
{
    long long value;
    char letter;
    struct node *left, *right;
};

typedef struct node Node;

long long englishLetterFrequencies [27] = {8, 2, 3, 4, 12, 2, 2, 6, 7, 2, 0, 4, 2, 6, 7, 2, 1, 5, 5, 7, 3, 1, 2, 0, 2, 0, 5};   
long long codeTable[27], codeTable2[27]; 
long long original = 0, compressed = 0;

/* file values */
int filesize;
FILE *input;
void *buf;

/* multithreading variables */
pthread_mutex_t myMutex;
pthread_cond_t condVar;
int threadCount;

/* finds and returns the smallest sub-tree in the forest */
long long findSmaller (Node *array[], long long differentFrom)
{
    long long smaller;
    long long i = 0L;

    while (array[i]->value == -1)
        i++;
    smaller = i;

    if (i == differentFrom)
    {
        i++;
        while (array[i]->value == -1)
            i++;
        smaller=i;
    }

    for (i = 1; i < 27; i++)
    {
        if (array[i]->value == -1)
            continue;
        if (i == differentFrom)
            continue;
        if (array[i]->value < array[smaller]->value)
            smaller = i;
    }

    return smaller;
}

/* builds the huffman tree and returns its address by reference */
void buildHuffmanTree (Node **tree)
{
    Node *temp;
    Node *array[27];
    long long i, subTrees = 27L;
    long long smallOne, smallTwo;

    for (i = 0; i < 27; i++)
    {
        array[i] = malloc (sizeof (Node));
        array[i]->value = englishLetterFrequencies[i];
        array[i]->letter = i;
        array[i]->left = NULL;
        array[i]->right = NULL;
    }

    while (subTrees > 1)
    {
        smallOne = findSmaller (array, -1);
        smallTwo = findSmaller (array, smallOne);
        temp = array[smallOne];
        array[smallOne] = malloc (sizeof (Node));
        array[smallOne]->value = temp->value + array[smallTwo]->value;
        array[smallOne]->letter = 127;
        array[smallOne]->left = array[smallTwo];
        array[smallOne]->right = temp;
        array[smallTwo]->value = -1;
        subTrees--;
    }

    *tree = array[smallOne];

    return;
}

/* builds the table with the bits for each letter. 1 stands for binary 0 and 2 for binary 1 (used to facilitate arithmetic) */
void fillTable(long long codeTable[], Node *tree, long long Code)
{
    if (tree->letter < 27)
        codeTable[(long long) tree->letter] = Code;
    else
    {
        fillTable (codeTable, tree->left, Code * 10 + 1);
        fillTable (codeTable, tree->right, Code * 10 + 2);
    }

    return;
}

/* function to compress the input; multi or single threaded */
void* compressFile (void* rank)
{
    char bit, c, x = 0;
    int n, length, bitsLeft = 8;
    int originalBits = 0, compressedBits = 0;

    /* threading values */
    long myRank = (long) rank;
    int split = filesize / threadCount;
    int first = myRank * split;
    int last;
    int i = 0;

    char strbuffer[12];
    sprintf (strbuffer, "part-00%i.txt", (int) myRank);
    FILE *output = fopen (strbuffer, "w");

    /* last thread goes to end of file no matter what */
    if (threadCount - 1 == myRank) 
    {
	    last = filesize;
    }
    else
    {
	    last = (myRank + 1) * split;
    }

    /* Continue to loop while the file still has characters. * 
     * Use multithreading here to break the file into chunks */	
    char *charbuf = (char *)buf; 
    
    while (((c = charbuf[i]) != 10) && i < (last - first))
    {
	i++;
        originalBits++;
        if (c == 32)
	{
            length = len (codeTable[26]);
            n = codeTable[26];
        }
        else
	{
            length = len (codeTable[c-97]);
            n = codeTable[c-97];
        }

        while (length > 0)
	{
            compressedBits++;
            bit = n % 10 - 1;
            n /= 10;
            x = x | bit;
            bitsLeft--;
            length--;
            if (bitsLeft == 0)
	    {
            	fputc(x, output);
            	x = 0;
            	bitsLeft = 8;
            }
            x = x << 1;
        }
    }

    if (bitsLeft != 8)
    {
        x = x << (bitsLeft - 1);
        fputc (x, output);
    }

    free (output);

    pthread_mutex_lock (&myMutex);
    original += originalBits;
    compressed += compressedBits;
    pthread_mutex_unlock (&myMutex);

    return;
}

/* function to decompress the input */
void decompressFile (FILE *input, FILE *output, Node *tree)
{
    Node *current = tree;
    char c, bit;
    char mask = 1 << 7;
    int i, readin = 0;

    while (filesize > readin)
    {
	c = fgetc (input);
	readin++;
        for (i = 0; i < 8; i++)
	{
            bit = c & mask;
            c = c << 1;
            if (bit == 0)
	    {
                current = current->left;
                if (current->letter != 0x7F)
		{
                    if (current->letter == 26)
                        fputc(0x20, output);
                    else
                        fputc (current->letter + 0x61, output);
                    current = tree;
                }
            }
            else
	    {
                current = current->right;
                if (current->letter != 0x7F)
		{
                    if (current->letter == 26)
                        fputc (0x20, output);
                    else
                        fputc (current->letter + 0x61, output);
                    current = tree;
                }
            }
        }
    }

    return;
}

/* invert the codes in codeTable2 so they can be used with mod operator by compressFile function */
void invertCodes(long long codeTable[], long long codeTable2[])
{
    long long i, n, copy;

    for (i = 0; i < 27; i++)
    {
        n = codeTable[i];
        copy = 0L;
        while (n > 0)
	{
            copy = copy * 10l + n % 10l;
            n /= 10l;
        }
        codeTable2[i] = copy;
    }

return;
}

int main(int argc, char* argv[])
{
    if (sizeof (argv) == 0) 
    {
	    printf ("Usage: ./huffman <number of threads>");
	    return 0;
    }

    Node *tree;
    int compress;
    char inFile[20];

    /* initialize multithreading values */
    long thread;
    pthread_t* threadHandles;
    threadCount = strtol (argv[1], NULL, 10);
    threadHandles = (pthread_t*) malloc (threadCount * sizeof (pthread_t));
    pthread_mutex_init (&myMutex, NULL);
    pthread_cond_init (&condVar, NULL);

    double start, finish, elapsed;

    printf ("Building the Huffman Tree...\n");
    buildHuffmanTree (&tree);

    printf ("Populating the code table...\n");
    fillTable (codeTable, tree, 0L);

    printf ("Inverting the codes...\n");
    invertCodes (codeTable, codeTable2);

    /* get input details from user */
    printf ("Type the name of the file to process >");
    scanf  ("%s", inFile);
    printf ("Type 1 to compress and 2 to decompress > ");
    scanf  ("%i", &compress);

    input = fopen(inFile, "r");

    /* determine size of file in bytes */
    fseek (input, 0L, SEEK_END);
    filesize = (int) ftell (input);    
    fseek (input, 0L, SEEK_SET);  

    GET_TIME (start);
    if (compress == 1) 
    {
	buf = (void *) malloc (filesize);
	pread (fileno (input), buf, (size_t)(filesize), (off_t)0);
	free (input);

	/* thread creation loop */
	for (thread = 0; thread < threadCount; thread++) 
	{
	    pthread_create (&threadHandles[thread], NULL, compressFile, (void*) thread);
	}

	printf ("Compressing the file, please wait...\n");

	/* Rejoin threads */
	for (thread = 0; thread < threadCount; thread++) 
	{
	    pthread_join (threadHandles[thread], NULL);
	    pthread_detach (threadHandles[thread]);
	}
    }
    else 
    {
	printf ("Decompressing the file, please wait...\n");
	FILE *output = fopen ("decompressed.txt", "w");
    	decompressFile (input, output, tree);
	free (input);
	free (output);
    }
    GET_TIME (finish);

    if (compress == 1)
    {
	system ("./combine.sh");

	fprintf (stderr, "\nOriginal bits = %lli\n", original * 8);
	fprintf (stderr, "Compressed bits = %lli\n", compressed);
	fprintf (stderr, "Saved %.2f%% of memory\n", ((float) compressed / (original * 8)) * 100);
    }

    elapsed = finish - start;
    printf ("Code took %f seconds to complete.\n", elapsed);  

    return 0;
}
