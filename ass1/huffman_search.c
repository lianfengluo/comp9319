#include"huffman.h"

void build_table(int bm_table[], char pattern[], int len){
    for(int i = 0; i != len - 1; i++){
        bm_table[(int)pattern[i]] = len - 1 - i;
    }
    for(int i = 0; i != CODING_SIZE; i++){
        if(bm_table[i] == 0){
            bm_table[i] = len;
        }
    }
}

int check_match(int search_buffer[], char search_pattern[], int bm_table[], int start_index,
                         int len, int *offset){
    for(int i = 0; i != len; i++){
        if(search_buffer[(start_index - i) % len] != search_pattern[len - 1 - i]){
            *offset = bm_table[(int)search_buffer[start_index % len]];
            return 0;
        }
    }
    *offset = 1;
    return 1;
}
int search(char *searching_pattern, char *filename){
    FILE * search_file = fopen(filename, "rb");
    // repeat the process of decode
    if(search_file == NULL)
        return -1;
    int dfs_array_len = 0;
    char temp_dfs_array_len[8];
    int c;
    int appear_count = 0;
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
    char dfs_arr[DFS_SIZE] = {'\0'};  //store the original ascii
    if(!fread(dfs_arr, sizeof(char), (size_t)dfs_array_len, search_file)){
        printf("error\n");
        return -1;
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
// Build BM table
    int bm_table[CODING_SIZE] = {0};
    int search_pattern_size = (int)strlen(searching_pattern);
    int search_buffer[SEARCH_PATTERN_MAX] = {0};
    build_table(bm_table, searching_pattern, search_pattern_size);
    int offset = search_pattern_size;
    // use for as search buff index
    int search_index = 0;
    int match_count = 0;
    int finish = 0;
    D_tree* search_tree = tree;
    while((c=fgetc(search_file))!=EOF){
        for(int i = 0; i != BIT_SPACE; i++){
            if((c & 128) == 0){
                search_tree = search_tree->left; 
            } else {
                search_tree = search_tree->right;
            }
            if(search_tree->character != STOP_CHAR){
                search_buffer[search_index % search_pattern_size] = search_tree->character;
                if(--offset < 1){
                    if(check_match(search_buffer, searching_pattern, bm_table, 
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
            c <<= 1;
        }
        if(finish)
            break;
    }
    printf("%d\n", match_count);
    fclose(search_file);
    free_decode_tree(tree);
    return 0;
}
