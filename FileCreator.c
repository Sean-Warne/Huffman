#include <stdio.h>
#include <stdlib.h>

int main (void)
{
	int i, j;

	char* alphabet = "abcdefghijklmnopqrstuvwxyz";
	int englishLetterPercent [26] = {8, 2, 3, 4, 13, 2, 2, 6, 7, 0, 1, 4, 2, 7, 8, 2, 0, 6, 6, 9, 3, 1, 2, 0, 2, 0};
	int*   englishLetterFreq = (int *) malloc(sizeof (int) * 100);	

	FILE *f;

	float filesize = 1.0;
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
	for (i = 0; i < iterations; i++) {
		fprintf (f, "%c", alphabet[englishLetterFreq[rand() % 99]]);
	}

	/* write ending character to file */
	fprintf (f, "%c", 'A');

	fclose(f);

	return 0;
}
