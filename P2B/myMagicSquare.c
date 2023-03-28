///////////////////////////////////////////////////////////////////////////////
// Copyright 2020 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission, CS 354 Spring 2022, Deb Deppeler
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Main File:        myMagicSquare.c
// This File:        myMagicSquare.c
// Other Files:      N/A
// Semester:         CS 354 Fall 2022
// Instructor:       deppeler
//
// Author:           Cinthya Nguyen
// Email:            cnguyen37@wisc.edu
// CS Login:         cinthya
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   Fully acknowledge and credit all sources of help,
//                   including Peer Mentors, Instructors, and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the ideas and help they provided.
//
// Online sources:   Avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   any information you find.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure that represents a magic square
typedef struct {
    int size;           // dimension of the square
    int **magic_square; // pointer to heap allocated magic square
} MagicSquare;

/*
 * Prompts the user for the magic square's size, reads it,
 * checks if it's an odd number >= 3 (if not display the required
 * error message and exit), and returns the valid number.
 */
int getSize() {

    int i;
    printf("Enter magic square's size (odd integer >=3)\n");
    scanf("%d", &i);

    if (i < 3 && i % 2 == 1) {
        printf("Magic square size must be >= 3.\n");
        exit(1);
    } else if (i % 2 == 0) {
        printf("Magic square size must be odd.\n");
        exit(1);
    }

    return i;
}

/*
 * Makes a magic square of size n using the alternate Siamese magic square
 * algorithm from assignment and returns a pointer to the completed
 * MagicSquare struct.
 *
 * n the number of rows and columns
 */
MagicSquare *generateMagicSquare(int n) {

    MagicSquare *temp = malloc(sizeof(MagicSquare));
    temp->size = n;

    int *testMalloc1 = calloc(n, sizeof(int *));
    if (testMalloc1 == NULL) {
        printf("invalid\n");
        exit(1);
    } else { // allocate array of *int
        temp->magic_square = calloc(n, sizeof(int *));
    }

    int *testMalloc2 = calloc(n , sizeof(int));
    if (testMalloc2 == NULL) {
        printf("invalid\n");
        exit(1);
    } else {
        for (int i = 0; i < n; i++) { // allocate array of ints
            *(temp->magic_square + i) = calloc(n , sizeof(int));
        }
    }

    int area = n*n;
    int i = 1;
    int row = 0;
    int col = n/2;

    while (i <= area) {
        if (row == -1 && col == n) {
            row += 2;
            col = n - 1;
        }

        if (row == -1) {
            row = n - 1;
        }

        if (col == n) {
            col = 0;
        }

        if ( *(*(temp->magic_square + row) + col) != 0) {
            row += 2;
            col--;
            continue;
        } else {
            *(*(temp->magic_square + row) + col) = i;
            i++;
        }

        row--;
        col++;
    }

    free(testMalloc1);
    free(testMalloc2);

    return temp;
}

/*
 * Opens a new file (or overwrites the existing file) and writes the square
 * in the specified format.
 *
 * magic_square the magic square to write to a file
 * filename the name of the output file
 */
void fileOutputMagicSquare(MagicSquare *magic_square, char *filename) {

    // Open the file and check if it opened successfully.
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Can't open file for reading.\n");
        exit(1);
    }

    fprintf(fp, "%d\n", magic_square->size);

    for (int i = 0; i < magic_square->size; i++) { // Print magic square.
        for (int j = 0; j < magic_square->size; j++) {
            fprintf(fp, "%d ", *(*(magic_square->magic_square + i) + j));
        }
        fprintf(fp, "\n");
    }

    // Close the file.
    if (fclose(fp) != 0) {
        printf("Error while closing the file.\n");
        exit(1);
    }

}

/*
 * Generates a magic square of the user specified size and
 * output the square to the output filename
 *
 * argc: the number of command line args (CLAs)
 * argv: the CLA strings, includes the program name
 */
int main(int argc, char **argv) {
    // Check input arguments to get output filename
    if (argc != 2) { // Check if number of command-line arguments is correct.
        printf("Usage: ./myMagicSquare <output_filename>\n");
        exit(1);
    }

    // Get magic square's size from user
    MagicSquare square;
    int size = getSize();

    // Generate the magic square
    square = *generateMagicSquare(size);

    // Output the magic square
    fileOutputMagicSquare(&square, *(argv + 1));

    // Free memory.
    for (int i = 0; i < size; i++) {
        free(*(square.magic_square + i));
    }
    
    free(square.magic_square);

    return 0;
}


//     F22 deppeler myMagicSquare.c
