#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define BIT_SPACE 8
#define OUTPUT_BUFF_SIZE 8
#define CODING_SIZE 256
#define CODED_LENGTH 255
#define CODE_LENGTH 255
#define STOP_CHAR 257
#define DFS_SIZE 866
#define SEARCH_PATTERN_MAX 256
#define ENCODE_READING_BUFF_SIZE 100000
#define ENCODE_WRITING_BUFF_SIZE 50000
#define DECODE_READING_BUFF_SIZE 100000
#define DECODE_WRITING_BUFF_SIZE 50000

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
void deep_first_search_build_tree(D_tree*, int [], int* , int);
void _sort_probablity(int *, int *, size_t, size_t, int*, int*);
void sort_probablity(int *, int*);
void sort_huffman(int *, nnode**, int);
void exploit_tree(nnode *, char [CODING_SIZE][CODED_LENGTH], int[], char[], int*);
int encode(char *, char *);

int decode(char *, char *);
void free_decode_tree(D_tree*);

int search(char *, char *);
void build_table(int [], char [], int);
void build_good_suffix_table(char [], int [], int [], int);
int check_match(int [], char [], int [], int[], int, int, int *);

