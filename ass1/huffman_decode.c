#include"huffman.h"
// void deep_first_search_build_tree(D_tree* node_ptr, int dfs_code[], int* length, int fixed_len){
void deep_first_search_build_tree(D_tree* node_ptr, char dfs_code[], int* length, int fixed_len){
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
int find_code(char code[], D_tree tree, int size){
    D_tree* tree_ptr = &tree;
    for(int i = 0; i != size; i++){
        if(code[i] == '0'){
            tree_ptr = tree_ptr->left;
        } else {
            tree_ptr = tree_ptr->right;
        }
    }
    return tree_ptr->character;
}
void getbinay(int val, char buf[], int *index){
    int i = 7;
    for(; i!=-1 ; --i, val /= 2)
        buf[(*index)+i] = "01"[val % 2];	
    (*index)+=8;
}
void free_decode_tree(D_tree* tree){
    if(tree->left && tree->right){
        free_decode_tree(tree->left);
        free_decode_tree(tree->right);
    }
    free(tree);
}

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
    char dfs_arr[DFS_SIZE] = {'\0'};  //store the original ascii
    if(!fread(dfs_arr, sizeof(char), (size_t)dfs_array_len, ROutput)){
        printf("error\n");
        return -1;
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
    char out_buff[DECODE_SIZE * 2] = {'\0'};
    char current_code[CODE_LENGTH] = {'\0'};
    int index = 0;      // index is the get code index
    int decode_index = 0;
    int decode_char;
    int found;
    int index_p1 = 0;
    int null_c = STOP_CHAR;
    // printf("%c, %c \n", tree->left->character, tree->right->character);
    while((c=fgetc(ROutput))!=EOF){
        getbinay(c, out_buff, &index);
        if(index > DECODE_SIZE){
            index_p1 = index + 1;
            while(1){
                found = 0;
                for(int i = 0; decode_index + i < index_p1; i++){
                    current_code[i] = out_buff[decode_index + i];
                    if ((decode_char = find_code(current_code, *tree, i)) != null_c){
                        fputc(decode_char, WOrign);
                        input_size--;
                        decode_index += i;
                        found = 1;
                        if(decode_index == index)
                            found = 0;
                        break;
                    }
                }
                if(input_size == 0){
                    break;
                }
                if (!found) {
                    for (int i = 0; decode_index + i != index_p1; i++){
                        out_buff[i] = out_buff[decode_index + i];
                    }
                    index = index - decode_index;
                    decode_index = 0;
                    break;
                }
            }
        }
    }
    index_p1 = index + 1;
    while(input_size != 0){
        for(int i = 0; decode_index + i != index_p1; i++){
            current_code[i] = out_buff[decode_index + i];
            if ((decode_char = find_code(current_code, *tree, i)) != null_c)
            {
                fputc(decode_char, WOrign);
                input_size--;
                decode_index += i;
                break;
            }
        }
    }
    fclose(WOrign);
    fclose(ROutput);
    free_decode_tree(tree);

    return 0;
}