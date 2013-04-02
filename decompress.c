/* This file contains a program which will read compressed data, either from a
 * given file or from stdin, and then print out that same data, but
 * uncompressed.  I will be testing your code on my Linux machine, and I will
 * test it by running files through your compression program and then my
 * decompression program, and then verifying that I get the same file back that
 * I put in.  A command line like:
 * 
 * ./yourcompression mobydick.txt | ./decompress | diff - mobydick.txt
 *
 * should produce no output.  That would mean that the output of your
 * compressor, run on the file "mobydick.txt", decompresses to output that is
 * exactly the same as the input file.  
 * 
 * Furthermore, 
 *
 * ./yourcompression mobydick.txt > mobydick.txt.zipped
 * 
 * should produce a file "mobydick.txt.zipped" that is smaller than the
 * original.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* The nodes that make up our decoding tree */
struct treenode {
    struct treenode *left;
    struct treenode *right;
    char data;
};

typedef struct treenode node;


/* This function is for debugging only - if you think that the decompress
 * program is reading in the wrong tree, then you can use this program to print
 * out the tree that the decompress program is reading in */
void printTree(node *tree, int depth) {
    int i;
    for (i = 0; i < depth; i++) fprintf(stderr, " ");
    fprintf(stderr, "(data: %d\n", tree->data);
    if (tree->left != NULL) {
        for (i = 0; i < depth; i++) fprintf(stderr, " ");
        fprintf(stderr, "left:\n");
        printTree(tree->left, depth + 2);
    }
    if (tree->right != NULL) {
        for (i = 0; i < depth; i++) fprintf(stderr, " ");
        fprintf(stderr, "right:\n");
        printTree(tree->right, depth + 2);
    }
    for (i = 0; i < depth; i++) fprintf(stderr, " ");
    fprintf(stderr, ")\n");
}

/* This function reads a decompression tree from the given input file and then
 * returns a pointer to that tree. */
node *readTree(FILE *in)
{
    char c; 
    node* n;

    // Things had better start with a '('
    c = fgetc(in);
    if (c != '(') {
        fprintf(stderr, "Bad start of tree: %c\n", c);
        exit(1);
    }

    // Make a new node to hold the data you are about to read
    n = (node*)calloc(1, sizeof(node));
    n->left = NULL;
    n->right = NULL;

    // A loop that exits via a "return" statement
    for (;;) {
        c = fgetc(in);

        switch (c) {
            // We read a number
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                n->data = n->data * 10 + (c - '0');
                break;

            // We read the beginning of a subtree
            case '(':
                ungetc(c, in);
                if (n->left == NULL) {
                    n->left = readTree(in);
                } else if (n->right == NULL) {
                    n->right = readTree(in);
                } else {
                    fprintf(stderr, "error in reading tree - missing a ')'\n");
                    exit(1);
                }
                break;

            // We read the end of our subtree
            case ')':
                return n;

            // I have no idea what just happened - ERROR
            default:
                fprintf(stderr, "Bad character in the lookup table: %c\n", c);
                exit(1);
        }
    }
}


/* This function will translate a file using the given Huffman tree.  The file
 * to be translated is read from "in", the tree to do the translation is stored
 * in "table", and the total number of bits to translate is "nbits". */
void translate(FILE *in, node *table, long nbits)
{
    unsigned char data;
    int i;
    node *curr;

    curr = table;
    data = fgetc(in);

    // While we haven't reached the end of the file
    while (!feof(in)) {
        // fprintf(stderr, "Read: %c (%d)\n", data, data); // Debug output
        // Read a byte, process all 8 bits
        for (i = 0; i < 8; i++) {
            if (nbits == 0) {
                fprintf(stderr, 
                        "Error: zero bits remaining but not at a leaf\n");
                printTree(curr, 0);
                exit(1);
            }

            //fprintf(stderr, "%d\n", data & (1 << 7)); // Debug output
            if ((data & (1 << 7)) != 0) { // Read the leftmost bit
                curr = curr->right;
            } else {
                curr = curr->left;
            }
            data <<= 1; // Shift the data over by one

            nbits--;
            // If we have reached a leaf in the tree, print things out.
            if (curr->right == NULL && curr->left == NULL) {
                //printf("Printing %c (%d)\n", curr->data, curr->data); // Debug output
                printf("%c", curr->data);
                curr = table;
                if (nbits == 0) {
                    return;
                }
            } else if (curr->right == NULL || curr->left == NULL) {
                fprintf(stderr, "Bad tree - can't have only one child\n");
                exit(1);
            }
        }

        data = fgetc(in);
    }

    // Hopefully this doesn't happen.
    if (nbits != 0) {
        fprintf(stderr, "Reached the end of the file with %ld bits remaining\n", nbits);
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    FILE *in;
    long nbits = 0;
    node *table;

    int i;
    char c;

    // If the user passes in an argument, try and use that as the input file
    if (argc > 1) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            perror(argv[1]);
            exit(errno);
        }
    } else {
        in = stdin;
    }

    // Read in the translation table (should be a tree with the leaves as
    // decimal numbers from 0-255, represented in ascii with parentheses)
    table = readTree(in);

    //printTree(table, 0); // Debug output

    // Now read in a ten-character long integer - the max being 4294967295
    // You should have no commas
    for (i = 0; i < 10; i++) {
        fscanf(in, "%c", &c);
        nbits = nbits * 10 + (c - '0');
    }

    // Now the remainder of the file is binary data

    // Translate the compressed file into an uncompressed file by reading in
    // the data byte by byte and consulting the translation table.  Print the
    // resulting table out on stdout
    translate(in, table, nbits);

    return 0;
}

