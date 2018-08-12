#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define BIT_SPACE 8
#define OUTPUT_BUFF_SIZE 8
#define CODING_SIZE 256
#define CODED_LENGTH 255
#define DECODE_SIZE 40960
#define CODE_LENGTH 255
#define STOP_CHAR 128
#define DFS_SIZE 866
#define SEARCH_PATTERN_MAX 256

// encode head field
typedef struct node {
    int character;
    char code[CODED_LENGTH];
    int height;
    struct node *left;
    struct node *right;
} nnode;
typedef struct decode_tree {
    int character;
    char code[CODED_LENGTH];
    struct decode_tree *left;
    struct decode_tree *right;
} D_tree;

nnode* build_tree(int [], nnode** , int);
// void deep_first_search_build_tree(D_tree*, int [], int* , int);
void deep_first_search_build_tree(D_tree*, char [], int* , int);
void _sort_probablity(int *, int *, size_t, size_t, int*, int*);
void sort_probablity(int *, int*);
void sort_huffman(int *, nnode**, int);
void exploit_tree(nnode *, char [CODING_SIZE][CODED_LENGTH], int[], char[], int*);
int encode(char *, char *);

// decode field
int find_code(char [], D_tree, int);
void getbinay(int , char [], int *);
int decode(char *, char *);
void free_decode_tree(D_tree*);

// search field
int search(char *, char *);
void build_table(int [], char [], int);
int check_match(int [], char *, int , int);
// int check_match(int [], char [], int [], int, int, int *);