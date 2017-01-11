Authors: Stein, Lucas
	 Walker, Jonathan
	 Warne, Sean
Data: 12/13/2016

Original (base) Code Retrieved From:
Scocco, D. (2011, December 8). Implementing Huffman Coding in C. Retrieved November 14, 2016, from http://www.programminglogic.com/implementing-huffman-coding-in-c/

Program: FileCreator.c
Description: Creates a file of a specified size (in GB) using the frequency of 
	     the letters in the English language. The text file it creates 
	     consists of a single string of the lowercase alphabet and spaces.

Program: HuffmanSingle.c
Description: The single threaded implementation of the Huffman Algorithm. Used
	     for testing purposes (slightly modified original code). 

Program: HuffmanMulti.c
Description: The multi-threaded implementation of the Huffman Algorithm. Takes
	     one argument: the number of threads and prompts the user for an
	     input filename and whether to decompress or compress the file.

Program: combine.sh
Description: Used by HuffmanMulti.c to combine the multi-part compressed output
	     files into a single file. Also removes the old output file and the
	     parts.


Compile:
# gcc FileCreator.c -o creator
# gcc HuffmanSingle.c -o huffmanSingle -lm
# gcc HuffmanMulti.c -o huffmanMulti -lm -lpthread
# chmod +x combine.sh

Note: the last command is used so that HuffmanMulti.c is able to use the system
      call that executes the bash shell script.


Execute:
# ./creator
# Enter the size of the file in GB >
# Enter the name of the output file >

# ./huffmanSingle
# Enter the name of the input file >
# Would you like to compress or decompress? > (1/0)

# ./huffmanMulti <number of threads>
# Enter the name of the input file >
# Would you like to compress or decompress? > (1/0)
