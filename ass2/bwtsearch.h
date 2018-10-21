#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<libgen.h>
#define READ_BUFF_SIZE 131072
#define WRITE_BUFF_SIZE 131072
#define MAX_RECORD 5000
#define START_UP_SIZE 2622222    // about 2.5 * 1024 * 1024
#define TOTAL_MEMORY 15 * 1024 * 1024
#define REST_MEMORY 13107200     // about 12.5 * 1024 * 1024
#define CHAR_LENGTH 127

int search_exist(char , FILE*, char *, char *, FILE*);
int search_count_duplicate(FILE*, char *, char *);
int search_count_uniq(char , FILE*, char *, char *);
int search_range(char, FILE *, char *, char *, FILE *);
void get_index_file_path(char *, char *, char*, char*);
int occ_function_sm(int *, int, char, int, char*, int*, int);
int occ_function_md(char *, int, char, int, FILE*, int*, int);
int occ_function_lg(FILE*, int, char, int, FILE* ,int*, int);
int occ_function_lg_with_char(FILE*, int, char *, int, FILE* ,int*, int);
void cal_c_table(int *, int *, int *);
void get_search_index(char *, int *, int *, int *, int *);
void sm_file_statistic(char *, int, int, int *, int , int *, int);
void occurrence_sm_get_bound(int *, int *, int *, int *, int *, char *, int *, char *, int *, int);
void occurrence_md_get_bound(int *, int *, int *, char *, int *, FILE *, int *, char *, int *, int);
void occurrence_lg_get_bound(int *, int *, int *, FILE *, int *, FILE *, int *, char *, int *, int, char *);
int *search_sm_integrate(char *, int, FILE *, int *, int *, int *, int *, int *, int *, char *,
                           int *, int *);
int *restore_data_sm(int *, FILE* , int *, int *, int , int *, int *);
void restore_data_md_lg(int *, FILE *, int *, int *, int , int *, int *);
int *storing_message_sm(int *, char *, int *, int *, int *, int *, int, char *);
char *storing_message_md(FILE *, int *, char *, int *, int *, int *, int *, int);
void storing_message_lg(FILE *, int *, char *, int *, int *, int *, int *, int);