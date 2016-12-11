#!/bin/bash

echo "Removing old output..."
rm ./output.txt
touch output.txt

echo "Combining files..."
cat part-*.txt >> output.txt

echo "Cleaning up..."
rm ./part-*.txt
