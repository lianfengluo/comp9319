#include<stdio.h>
#include<libgen.h>
#include<stdlib.h>
#include<string.h>
#define READING_BUFFER 131072
#define WRITING_BUFFER 131072
#define WRITING_INDEX_BUFF 65536
#define READING_INDEX_BUFF 65536
#define TOTAL_CHAR 97
#define CHAR_LENGTH 127
#define CHAR_LENGTH_MAX_INDEX 128


void create_filename(char *, char *, char *, char *);
int encoder(char, FILE *, char *, int, char *, char *);
