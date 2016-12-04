#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

int main ()
{
	int i, j;
	double start, finish, elapsed;
	float filesize = 0.0;

	char* alphabet = "abcdefghijklmnopqrstuvwxyz";
	int englishLetterPercent [26] = {5, 2, 3, 4, 16, 2, 2, 6, 7, 0, 1, 4, 2, 7, 8, 2, 0, 6, 6, 9, 3, 1, 2, 0, 2, 0};
	int*   englishLetterFreq = (int *) malloc(sizeof (int) * 100);	

	FILE *f;

	printf ("Enter the size of the file in Gb > ");
	scanf  ("%f", &filesize);

	int bytesPerGig = 1073741824;
	int charSize = sizeof (char);

	// number of characters
	int iterations = (filesize * bytesPerGig) / charSize; 

	f = fopen ("file.txt", "w");
	if (f == NULL) {
		printf ("Error opening file!\n");
		exit(1);
	}

	/* fill the letter array */
	int   range = 0;
	int lastMax = 0;
	for (i = 0; i < 26; i++)
	{
		range += englishLetterPercent[i];
		for (j = lastMax; j < range; j++)
		{
			englishLetterFreq[j] = i;
		}

		lastMax = range + 1;
	}

	/* fill the file */
	GET_TIME (start);
	for (i = 0; i < iterations; i++) {
		fprintf (f, "%c", alphabet[englishLetterFreq[rand() % 99]]);
	}

	fclose(f);
	GET_TIME (finish);
	elapsed = finish - start;

	printf ("File created successfully!\nCode took %f seconds to complete.\n", elapsed);

	return 0;
}
