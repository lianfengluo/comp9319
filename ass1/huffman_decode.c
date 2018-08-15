#include"huffman.h"
// use deep first search to retrieve the tree
void deep_first_search_build_tree(D_tree* node_ptr, int dfs_code[], int* length, int fixed_len){
    if(dfs_code[*length] == (int)'1'){
        (*length)++;
        node_ptr->character = dfs_code[(*length)++];
        node_ptr->left = NULL;
        node_ptr->right = NULL;
    }
    else if(dfs_code[*length] == (int)'0'){
        (*length)++;
        if(*length != fixed_len){
            D_tree* new_node_ptr; 
            new_node_ptr = (D_tree *)malloc(sizeof(D_tree));
            new_node_ptr->character = STOP_CHAR;
            new_node_ptr->left = NULL;
            new_node_ptr->right = NULL;
            node_ptr->left = new_node_ptr;
            node_ptr->right = NULL;
            // explot left
            deep_first_search_build_tree(node_ptr->left, dfs_code, length, fixed_len);
        }
        if(*length != fixed_len){
            D_tree* new_node_ptr; 
            new_node_ptr = (D_tree *)malloc(sizeof(D_tree));
            new_node_ptr->character = STOP_CHAR;
            new_node_ptr->left = NULL;
            new_node_ptr->right = NULL;
            node_ptr->right = new_node_ptr;
            // explot right
            deep_first_search_build_tree(node_ptr->right, dfs_code, length, fixed_len);
        }
    }
}
void free_decode_tree(D_tree* tree){
    if(tree->left && tree->right){
        free_decode_tree(tree->left);
        free_decode_tree(tree->right);
    }
    free(tree);
}
// decode main function
int decode(char *src_name, char *decode_file_name){
    FILE* ROutput = fopen(src_name, "rb");
    if(ROutput == NULL)
        return -1;
    int dfs_array_len = 0;
    char temp_dfs_array_len[8];
    int c;
    int appear_count = 0;
    while((c = fgetc(ROutput)) != (int)' '){
        temp_dfs_array_len[appear_count++] = c;
    }
    // get orginal input size
    dfs_array_len = atoi(temp_dfs_array_len);
    // if it's an empty file
    if(dfs_array_len == 0){
        FILE* WOrign = fopen(decode_file_name, "wb");
        fclose(WOrign);
        fclose(ROutput);
        return 0;
    }
    int dfs_arr[DFS_SIZE] = {0};
    for(int i = 0; i != dfs_array_len; i++){
      dfs_arr[i] = fgetc(ROutput); 
    }
    D_tree *tree;
    tree = (D_tree *)malloc(sizeof(D_tree));
    tree->character = STOP_CHAR;
    tree->left = NULL;
    tree->right = NULL;
    int start_length = 0;
    char temp_input_size[15];
    int input_size = 0;
    // index
    int count_index = 0;
    while((c = fgetc(ROutput)) != (int)' ')
        temp_input_size[count_index++] = c;
    input_size = atoi(temp_input_size);
    deep_first_search_build_tree(tree, dfs_arr, &start_length, dfs_array_len);
    // output orginal file
    FILE* WOrign = fopen(decode_file_name, "wb");
    if(WOrign == NULL){
        fclose(ROutput);
        return -1;
    }
    fseek(ROutput, 1024, SEEK_SET);
    // input_size is the input size
    int finish = 0;
    D_tree* search_tree = tree;
    while((c=fgetc(ROutput))!=EOF){
        for(int i = 0; i != BIT_SPACE; i++){
            if((c & 128) == 0){
                search_tree = search_tree->left; 
            } else {
                search_tree = search_tree->right;
            }
            if(search_tree->character != STOP_CHAR){
                fputc(search_tree->character, WOrign);
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
    fclose(WOrign);
    fclose(ROutput);
    free_decode_tree(tree);
    return 0;
}