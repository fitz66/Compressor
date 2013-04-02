#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int left(int i) { return 2*i + 1; }
int right(int i) { return 2*i + 2; }
int parent(int i) { return (i-1) / 2; }

struct numFreq{
    int frequency;
    unsigned char symbol;
};

struct treeNode{
    struct treeNode *left;
    struct treeNode *right;
    struct numFreq data;
};

int treeArraySize = 255;
struct treeNode *treeArray[256];
char *arrayOfLabels[256];
int bitCount = 0;
int bits = 0;

void checkIfHeap(struct treeNode* treeArray[]);
void printBit(char b);
int translate(char input);
void swap(int posOfParent,int posOfChild);
void bubbleDown(int posOfParent);
void  bubbleUp(int index);
struct treeNode *pop();
struct treeNode *merge(struct treeNode *tempNode1 , struct treeNode *tempNode2);
void push(struct treeNode *combinedNode);
void traverse(struct treeNode *node, char *label);
void printTree(struct treeNode *node);

int main(int argc, char* argv[])
{
    int numChar = 256;
    FILE *in;
    char *fileContents;
    int dsize = 0;
    int arraysize = 1;
    int treeSize;
    char c;
    char *currentBit;
    int i;
    int j;
    int count[numChar];
    int numOfBits = 0;
    struct numFreq freqArray[numChar];
    struct numFreq current;
    struct treeNode *smallest;
    struct treeNode *nextSmallest;
    struct treeNode *combinedTreeNode; 
    struct treeNode *node;
   
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

    // Read in all of the data
    fileContents = (char*)malloc(arraysize * sizeof(char));
    while (!feof(in)) {
        fileContents[dsize++] = fgetc(in);
        if (dsize == arraysize) {
            arraysize *= 2;
            fileContents = (char*)realloc(fileContents, arraysize*sizeof(char));
        }
    }
    dsize--; 
    // The last char we read was bogus a EOF character
    
    for(i = 0; i < numChar; i++){
        count[i] = 0;
    }	
    for(i = 0; i < dsize; i++){
        count[(int)fileContents[i]]++;
    }
    
    for(i = 0; i < numChar; i++){
        struct numFreq current = { count[i], (unsigned char)i};    
        freqArray[i] = current;
    }

    treeArraySize = 0;
    treeArray[0] = (struct treeNode*)malloc(sizeof(struct treeNode));
    treeArray[0]->data = freqArray[0];
    treeArray[0]->left = NULL;
    treeArray[0]->right = NULL;

    for(i = 1; i < numChar; i++){
        node = (struct treeNode*)malloc(sizeof(struct treeNode));
        node->data = freqArray[i];
	node->left = NULL;
	node->right = NULL;

	push(node);
    }

    while(treeArraySize != 1){

	//pop off the two smallest values in the tree
        smallest = pop();
        nextSmallest = pop();

        //merge these two values
        combinedTreeNode = merge(smallest, nextSmallest);

        //place the merged tree back into the heap
        push(combinedTreeNode);
    }
    
/*  
    for(i = 0; i <= numChar - 1; i++){
        printf("this is index %c ", treeArray[i]->data.symbol);
        printf("___this is the frequency %d \n ", treeArray[i]->data.frequency);
    }
*/

    smallest = pop();
    nextSmallest = pop();
    combinedTreeNode = merge(smallest,nextSmallest);
    push(combinedTreeNode);

    //print out the tree
   
    printTree(treeArray[0]);
     
    traverse(treeArray[0], "");

    
    // Write out, as a ten digit number (possibly with leading zeroes) the
    // number of bits in the data stream you are outputting
   
    // Write out the "compressed" version of the data we read.
    
    for(i = 0; i < dsize; i++){  
        numOfBits += translate(fileContents[i]);
    }
    
/*
    for(i = 0; i < 256; i++){
        printf("Character %c = %s \n", i, arrayOfLabels[i]);
    }
*/    
    printf("%010d",numOfBits);

   
    for(i = 0; i < dsize; i++){
        currentBit = arrayOfLabels[fileContents[i]];
        for(j = 0; j < strlen(currentBit); j++){
            printBit((char)currentBit[j]);
	}
    }
         
    while(bitCount != 0){
       printBit('0');
    }
    return 0;
}

void swap(int posOfParent, int posOfChild)
{
    int leftChild;
    int rightChild;
    int parent;
    struct treeNode *temp;

    temp = treeArray[posOfParent];
    treeArray[posOfParent] = treeArray[posOfChild];
    treeArray[posOfChild] = temp;
}

void bubbleDown(int posOfParent){
    int parent;//frequency of parent
    int leftChild;//frequency of leftCHild
    int rightChild;//frequency of rightChilde

    parent = treeArray[posOfParent]->data.frequency;

    if(right(posOfParent) > treeArraySize){ 
        if(left(posOfParent) > treeArraySize) return;

        // Compare current node to the left child, but not the right
        leftChild = treeArray[left(posOfParent)]->data.frequency;
	if (leftChild < parent) 
            swap(posOfParent, left(posOfParent));
	return;
    }

    leftChild = treeArray[left(posOfParent)]->data.frequency;
    rightChild = treeArray[right(posOfParent)]->data.frequency;
    
    if(parent < leftChild && parent < rightChild) return;
    
    if(leftChild < rightChild){
        swap(posOfParent, left(posOfParent));
	bubbleDown(left(posOfParent));
    } else{
	swap(posOfParent, right(posOfParent));
	bubbleDown(right(posOfParent));
    }
}

void bubbleUp(int posOfChild){
    int parentFreq;
    int child;
    int posOfParent;
    struct treeNode *temp;

    if (posOfChild == 0) return;

    child = treeArray[posOfChild]->data.frequency;
    posOfParent = parent(posOfChild);
    parentFreq = treeArray[posOfParent]->data.frequency;
    
    if (child < parentFreq){
        temp = treeArray[posOfParent];
        treeArray[posOfParent] = treeArray[posOfChild];
        treeArray[posOfChild] = temp;

        bubbleUp(posOfParent);
    }
}

void push(struct treeNode *combinedNode){
    treeArraySize++;
    treeArray[treeArraySize] = combinedNode;
    bubbleUp(treeArraySize);
}

struct treeNode *pop(){
    struct treeNode *removed = treeArray[0];
    treeArray[0] = treeArray[treeArraySize];
    treeArraySize--;

    bubbleDown(0);
    return(removed);
}

struct treeNode *merge(struct treeNode *tempNode1, struct treeNode *tempNode2){
    struct treeNode *combinedNode;
    combinedNode = (struct treeNode*)malloc(sizeof(struct treeNode));
    combinedNode->left = tempNode1;
    combinedNode->right = tempNode2;
    combinedNode->data.frequency = tempNode1->data.frequency + 
                                        tempNode2->data.frequency;
    return combinedNode;
}

void traverse(struct treeNode *node, char *label){
    char *left, *right;
    if(node->left == NULL){
        arrayOfLabels[node->data.symbol] = label;
    }else{
        left = (char*)malloc(sizeof(char)*(strlen(label)+2));
	strcpy(left, label);
        left[strlen(label)] = '0';
        left[strlen(label)+1] = '\0';
	traverse(node->left,left);

        right = (char*)malloc(sizeof(char)*(strlen(label)+2));
	strcpy(right,label);
	right[strlen(label)] = '1';
	right[strlen(label) + 1] = '\0';
	traverse(node->right,right);
    }
}


int translate(char input){
    int i;
    char *answer;
    int numOfBits;
    answer = arrayOfLabels[(int)input];
    numOfBits = strlen(answer);
    return(numOfBits);
}

void printBit(char b){
    bits = bits << 1;
    
    if(b == '1') bits = bits | 1;
    bitCount++;

    if(bitCount == 8){
       printf("%c",bits);
       bits = 0;
       bitCount = 0;
   }
}

void printTree(struct treeNode *node)
{
    printf("(");
    if (node->left == NULL) {
        printf("%u", (int)node->data.symbol);
    } else {
        printTree(node->left);
        printTree(node->right);
    }
    printf(")");
}

void heckIfHeap(struct treeNode* treeArray[]){
    int i;
    int parent;
    int leftChild;
    int rightChild;

    for(i = 0; i < 125;i++){
        parent = treeArray[i]->data.frequency;
        leftChild = treeArray[left(i)]->data.frequency;
        rightChild = treeArray[right(i)]->data.frequency;
        if(parent > leftChild || parent > rightChild)
            printf("Heap fails at element %c \n", treeArray[i]->data.symbol);
    }
    printf("Heap is correct.\n");
}

