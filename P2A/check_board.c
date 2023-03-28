////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Deb Deppeler
// Posting or sharing this file is prohibited, including any changes/additions.
//
// We have provided comments and structure for this program to help you get 
// started.  Later programs will not provide the same level of commenting,
// rather you will be expected to add same level of comments to your work.
// 09/20/2021 Revised to free memory allocated in get_board_size function.
// 01/24/2022 Revised to use pointers for CLAs
//
////////////////////////////////////////////////////////////////////////////////
// Main File:        check_board.c
// This File:        check_board.c
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

char *DELIM = ",";  // commas ',' are a common delimiter character for data strings

/* COMPLETED (DO NOT EDIT):       
 * Read the first line of input file to get the size of that board.
 * 
 * PRE-CONDITION #1: file exists
 * PRE-CONDITION #2: first line of file contains valid non-zero integer value
 *
 * fptr: file pointer for the board's input file
 * size: a pointer to an int to store the size
 */
void get_board_size(FILE *fptr, int *size) {
    char *line1 = NULL;
    size_t len = 0;

    if (getline(&line1, &len, fptr) == -1) {
        printf("Error reading the input file.\n");
        free(line1);
        exit(1);
    }

    char *size_chars = NULL;
    size_chars = strtok(line1, DELIM);
    *size = atoi(size_chars);

    // Free memory allocated for reading first link of file.
    free(line1);
    line1 = NULL;
}


/*
 * Returns 1 if and only if the board is in a valid Sudoku board state.
 * Otherwise, returns 0.
 * 
 * A valid row or column contains only blanks or the digits 1-size, 
 * with no duplicate digits, where size is the value 1 to 9.
 * 
 * Note: p2A requires only that each row and each column are valid.
 * 
 * board: heap allocated 2D array of integers 
 * size:  number of rows and columns in the board
 */
int valid_board(int **board, int size) {

    // Check size is between 1-9.
    if (size < 1 || size > 9) {
        return 0;
    }

    // Check rows for duplicates.
    for (int i = 0; i < size; i++) { // i represents row.
        for (int j = 0; j < size; j++) { // j represents column.

            int ref = *(*(board + i) + j);
            if (ref < 0 || ref > size) { // Ensures number is between 0-n.
                return 0;
            }

            for (int k = j + 1; k < size; k++) {
                int num = *(*(board + i) + k);

                if (num != 0 && ref == num) { // Number is a duplicate.
                    return 0;
                }
            }
        }
    }

    // Check columns for duplicates.
    for (int i = 0; i < size; i++) { // i represents column.
        for (int j = 0; j < size; j++) { // j represents row.
            int ref = *(*(board + j) + i);

            for (int k = j + 1; k < size; k++) {
                int num = *(*(board + k) + i);

                if (num != 0 && ref == num) { // Number is a duplicate.
                    return 0;
                }
            }
        }
    }

    return 1;
}


/*
 * This program prints "valid" (without quotes) if the input file contains
 * a valid state of a Sudoku puzzle board wrt to rows and columns only.
 *
 * A single CLA which is the name of the file that contains board data 
 * is required.
 *
 * argc: the number of command line args (CLAs)
 * argv: the CLA strings, includes the program name
 */
int main(int argc, char **argv) {

    // Check if number of command-line arguments is correct.
    if (argc != 2) {
        printf("Usage: ./check_board <input_filename>\n");
        exit(1);
    }

    // Open the file and check if it opened successfully.
    FILE *fp = fopen(*(argv + 1), "r");
    if (fp == NULL) {
        printf("Can't open file for reading.\n");
        exit(1);
    }

    // Declare local variables.
    int size;

    // Calls get_board_size to read first line of file as the board size.
    get_board_size(fp, &size);

    // Dynamically allocate a 2D array for given board size.
    int **m;

    // Allocate array of *int.
    int *testMalloc1 = malloc(sizeof(int *) * size);
    if (testMalloc1 == NULL) {
        printf("invalid\n");
        exit(1);
    } else {
        m = malloc(sizeof(int *) * size);
    }

    // Allocate arrays of int.
    int *testMalloc2 = malloc(sizeof(int) * size);
    if (testMalloc2 == NULL) {
        printf("invalid\n");
        exit(1);
    } else {
        for (int i = 0; i < size; i++) { // allocate array of ints for each row
            *(m + i) = malloc(sizeof(int) * size);
        }
    }

    // Read the rest of the file line by line.
    // Tokenize each line wrt the delimiter character
    // and store the values in your 2D array.
    char *line = NULL;
    size_t len = 0;
    char *token = NULL;
    for (int i = 0; i < size; i++) {
        if (getline(&line, &len, fp) == -1) {
            printf("Error while reading line %i of the file.\n", i + 2);
            exit(1);
        }

        token = strtok(line, DELIM);

        for (int j = 0; j < size; j++) {
            // Initialize your 2D array.
            *(*(m + i) + j) = atoi(token);
            token = strtok(NULL, DELIM);
        }
    }

    // Call the function valid_board and print the appropriate
    // output depending on the function's return value.
    if (valid_board(m, size) == 1) {
        printf("valid\n");
    } else {
        printf("invalid\n");
    }

    // Free dynamically allocated memory.
    for (int i = 0; i < size; i++) {
        free(*(m + i));
        *(m + i) = NULL;
    }

    free(m);
    free(line);
    free(testMalloc1);
    free(testMalloc2);

    m = NULL;
    line = NULL;
    testMalloc1 = NULL;
    testMalloc2 = NULL;

    // Close the file.
    if (fclose(fp) != 0) {
        printf("Error while closing the file.\n");
        exit(1);
    }

    return 0;
}

// s22

