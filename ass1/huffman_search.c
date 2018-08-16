#include"huffman.h"
// Building BM table
void build_table(int bm_table[], char pattern[], int len){
    for(int i = 0; i != CODING_SIZE; i++){
        bm_table[i] = len;
    }
    for(int i = 0; i != len - 1; i++){
        bm_table[(int)pattern[i]] = len - 1 - i;
    }
}
// searching match pattern
int check_match(int search_buffer[], char search_pattern[], int bm_table[], int good_suffix_table[],
                     int start_index, int len, int *offset){
    for(int i = 0; i != len; i++){
        if(search_buffer[(start_index - i) % len] != search_pattern[len - 1 - i]){
            // bad matching rule
            *offset = bm_table[(int)search_buffer[start_index % len]];
            // Implement the good suffix rule
            if(*offset < good_suffix_table[len - i]){
                *offset = good_suffix_table[len - i];
            }
            return 0;
        }
    }
    *offset = good_suffix_table[0];
    return 1;
}

void build_good_suffix_table(char pattern[], int shift_table[], int suffix_table[], int len){
    int i = len, j = len + 1;
    suffix_table[i] = j;
    // processing 1: create table that store the shift when it comes to same prefix in the matched string 
    while(i != 0){
        while(j <= len && pattern[j - 1] != pattern[i - 1]){
            if(shift_table[j] == 0)
                shift_table[j] = j - i;
            j = suffix_table[j];
        }
        i--; j--;
        suffix_table[i] = j;
    }
    // processing2: expand the table that when the prefix of pattern match the sub prefix of the matched string
    j = suffix_table[0];
    for(int i = 0; i != len + 1; i++){
        if(shift_table[i] == 0)
            shift_table[i] = j;
        if(i == j)
            j = suffix_table[j];
    }
}

int search(char *searching_pattern, char *filename){
    FILE * search_file = fopen(filename, "rb");
    // repeat the process of decode
    if(search_file == NULL || searching_pattern[0] == '\0')
        return -1;
    int dfs_array_len = 0, appear_count = 0, c;
    char temp_dfs_array_len[8];
    while((c = fgetc(search_file)) != (char)' '){
        temp_dfs_array_len[appear_count++] = c;
    }
    // get orginal input size
    dfs_array_len = atoi(temp_dfs_array_len);
    // if it's an empty file
    if(dfs_array_len == 0){
        printf("0\n");
        fclose(search_file);
        return 0;
    }
    int dfs_arr[DFS_SIZE] = {0};
    for(int i = 0; i != dfs_array_len; i++){
      dfs_arr[i] = fgetc(search_file); 
    }
    D_tree *tree;
    tree = (D_tree *)malloc(sizeof(D_tree));
    tree->character = STOP_CHAR;
    tree->left = NULL;
    tree->right = NULL;
    int start_length = 0;
    deep_first_search_build_tree(tree, dfs_arr, &start_length, dfs_array_len);
    char temp_input_size[15];
    int input_size = 0;
    // index
    int count_index = 0;
    while((c = fgetc(search_file)) != (int)' ')
        temp_input_size[count_index++] = c;
    input_size = atoi(temp_input_size);
    fseek(search_file, 1024, SEEK_SET);
// Build BM table, bad matching rule
    int bm_table[CODING_SIZE] = {0};
    int search_pattern_size = (int)strlen(searching_pattern);
    int search_buffer[SEARCH_PATTERN_MAX] = {0};
    build_table(bm_table, searching_pattern, search_pattern_size);
    // use to build the reverse suffix matching like KMP
    int suffix_matching[search_pattern_size + 1];
    for(int i = 0; i != search_pattern_size + 1; i++){
        suffix_matching[i] = 0;
    }
    // good suffix offset table
    int good_suffix_table[search_pattern_size + 1];
    for(int i = 0; i != search_pattern_size + 1; i++){
        good_suffix_table[i] = 0;
    }
    build_good_suffix_table(searching_pattern, good_suffix_table, suffix_matching, search_pattern_size);
    int offset = search_pattern_size;
    int search_index = 0, match_count = 0, finish = 0;
    D_tree* search_tree = tree;
    char buffer_read[DECODE_READING_BUFF_SIZE] = {'\0'};
    int fread_return = 0;
    char output_char = 0;
    while(1){
        fread_return = fread(buffer_read, sizeof(char), DECODE_READING_BUFF_SIZE, search_file);
        if(!fread_return){
            break;
        }
        for(int i = 0; i != fread_return; i++){
            output_char = buffer_read[i];
            for(int j = 0; j != BIT_SPACE; j++){
                if((output_char & 128) == 0)
                    search_tree = search_tree -> left;
                else
                    search_tree = search_tree -> right;
                if(search_tree->character != STOP_CHAR){
                    search_buffer[search_index % search_pattern_size] = search_tree->character;
                    if(--offset < 1){
                        if(check_match(search_buffer, searching_pattern, bm_table, good_suffix_table,
                        search_index ,search_pattern_size, &offset))
                            match_count++;
                    }
                    search_index++;
                    input_size--;
                    if(input_size == 0){
                        finish = 1;
                        break;
                    }
                    search_tree = tree;
                }
                output_char <<= 1;
            }
            if(finish == 1)
                break;
        }
        if(finish == 1)
            break;
    }
    printf("%d\n", match_count);
    fclose(search_file);
    free_decode_tree(tree);
    return 0;
}
