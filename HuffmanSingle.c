#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

long long englishLetterFrequencies [26] = {8, 2, 3, 4, 13, 2, 2, 6, 7, 0, 1, 4, 2, 7, 8, 2, 0, 6, 6, 9, 3, 1, 2, 0, 2, 0}; 

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

    for (i = 1; i < 26; i++)
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
    Node *array[26];
    long long i, subTrees = 26L;
    long long smallOne, smallTwo;

    for (i = 0; i < 26; i++)
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
    if (tree->letter < 26)
        codeTable[(long long) tree->letter] = Code;
    else
    {
        fillTable (codeTable, tree->left, Code * 10 + 1);
        fillTable (codeTable, tree->right, Code * 10 + 2);
    }

    return;
}

/* function to compress the input; multi or single threaded */
void compressFile(FILE *input, FILE *output, long long codeTable[])
{
    char bit, c, x = 0; // thread local
    long long n, length, bitsLeft = 8; // thread local
    long long originalBits = 0, compressedBits = 0; // thread global

    /* print out the code table */
    int j;
    printf("\n\n");
    for (j = 0; j < sizeof(codeTable); j++)
    {
	printf ("%i\t%lli\n", j, codeTable[j]);
    }
    printf ("\n\n");


    /* Continue to loop while the file still has characters. * 
     * Use multithreading here to break the file into chunks */
    while ((c = fgetc (input)) != EOF)
    {
        originalBits++;
        if (c == 0x20)
	{
            length = len (codeTable[25]);
            n = codeTable[25];
        }
        else
	{
            length = len (codeTable[c - 0x61]);
            n = codeTable[c - 0x61];
        }

	/* Compress the character */
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
                fputc (x, output);
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

    /* prlong long details of compression on the screen */
    fprintf (stderr, "Original bits = %lli\n", originalBits * 8);
    fprintf(stderr, "Compressed bits = %lli\n", compressedBits);
    fprintf(stderr, "Saved %.2f%% of memory\n", ((float) compressedBits / (originalBits * 8)) * 100);

    return;
}

/* function to decompress the input */
void decompressFile (FILE *input, FILE *output, Node *tree)
{
    Node *current = tree;
    char c, bit;
    char mask = 1 << 7;
    long long i;

    while ((c = fgetc (input)) != EOF)
    {
        for (i = 0; i < 8; i++)
	{
            bit = c & mask;
            c = c << 1;
            if (bit == 0)
	    {
                current = current->left;
                if (current->letter != 0x7F)
		{
                    if (current->letter == 25)
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
                    if (current->letter == 25)
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

    for (i = 0; i < 26; i++)
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

int main()
{
    Node *tree;
    long long codeTable[26], codeTable2[26];
    long long compress;
    char inFile[20];
    char outFile[20];
    FILE *input, *output;

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
    printf ("Type the name of the output file > ");
    scanf  ("%s", outFile);
    printf ("Type 1 to compress and 2 to decompress > ");
    scanf  ("%lli", &compress);

    input = fopen(inFile, "r");
    output = fopen(outFile, "w");

    GET_TIME (start);
    if (compress == 1) {
	    printf ("Compressing the file, please wait...\n");
	    compressFile (input, output, codeTable2);
    }
    else {
	    printf ("Decompressing the file, please wait...\n");
    	    decompressFile (input, output, tree);
    }
    GET_TIME (finish);

    elapsed = finish - start;
    printf ("Code took %f seconds to complete.\n", elapsed);  

    return 0;
}
