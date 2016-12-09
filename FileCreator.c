#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

int main ()
{
	int i, j;
	double start, finish, elapsed;
	float filesize = 0.0;
	char output[20];

	char* alphabet = "abcdefghijklmnopqrstuvwxyz ";
	int englishLetterPercent [27] = {8, 2, 3, 4, 12, 2, 2, 6, 7, 2, 0, 4, 2, 6, 7, 2, 1, 5, 5, 7, 3, 1, 2, 0, 2, 0, 5};
	int*   englishLetterFreq = (int *) malloc(sizeof (int) * 100);	

	FILE *f;

	printf ("Enter the size of the file in Gb > ");
	scanf  ("%f", &filesize);
	printf ("Enter the name of the output file > ");
	scanf  ("%s", output);


	int bytesPerGig = 1073741824;

	// number of characters
	int iterations = (filesize * bytesPerGig); 

	f = fopen (output, "w");
	if (f == NULL) {
		printf ("Error opening file!\n");
		exit(1);
	}

	/* fill the letter array */
	int   range = 0;
	int lastMax = 0;
	for (i = 0; i < 27; i++)
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
		fprintf (f, "%c", alphabet[englishLetterFreq[rand() % 100]]);
	}

	fclose(f);
	GET_TIME (finish);
	elapsed = finish - start;

	printf ("File created successfully!\nCode took %f seconds to complete.\n", elapsed);

	return 0;
}
