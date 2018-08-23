#include"huffman.h"
/* deep_first_search_build_tree 
 * node_ptr is the empty tree root node, and ready to be built
 * dfs_code is the code that contain the message of reconstruct the tree
 * length is the dfs_code index
 * fixed_len is the total length of dfs_code
 */
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
/* free_decode_tree
 * tree is the tree be ready to be freed
 */
void free_decode_tree(D_tree* tree){
    if(tree->left && tree->right){
        free_decode_tree(tree->left);
        free_decode_tree(tree->right);
    }
    free(tree);
}
/* decode 
 * src_name is the encoded filename
 * output_file_name is the name of the file after being decoded
 */
int decode(char *src_name, char *output_file_name){
    FILE* encoded_file = fopen(src_name, "rb");
    if(encoded_file == NULL)
        return -1;
    int dfs_array_len = 0;
    char temp_dfs_array_len[8];
    int appear_count = 0, c;
    while((c = fgetc(encoded_file)) != (int)' '){
        temp_dfs_array_len[appear_count++] = c;
    }
    // get orginal input size
    dfs_array_len = atoi(temp_dfs_array_len);
    // output orginal file
    FILE *file_after_decode = fopen(output_file_name, "wb");
    if(file_after_decode == NULL){
        fclose(encoded_file);
        return -1;
    }
    // if it's an empty file
    if(dfs_array_len == 0){
        fclose(encoded_file);
        fclose(file_after_decode);
        return 0;
    }
    int dfs_arr[DFS_SIZE] = {0};
    for(int i = 0; i != dfs_array_len; i++){
        dfs_arr[i] = fgetc(encoded_file); 
    }
    char temp_input_size[15];
    int input_size = 0, count_index = 0, start_length = 0;
    while((c = fgetc(encoded_file)) != (int)' ')
        temp_input_size[count_index++] = c;
    input_size = atoi(temp_input_size);
    if(dfs_array_len == 3){
        char buffer_write[DECODE_WRITING_BUFF_SIZE] = {'\0'};
        while(input_size != 0){
            if(input_size - DECODE_WRITING_BUFF_SIZE >= 0){
                memset(buffer_write, dfs_arr[dfs_array_len - 1], sizeof(char) * DECODE_WRITING_BUFF_SIZE);
                fwrite(buffer_write, sizeof(char), DECODE_WRITING_BUFF_SIZE * sizeof(char), file_after_decode);
                input_size -= DECODE_WRITING_BUFF_SIZE;
            } else {
                memset(buffer_write, dfs_arr[dfs_array_len - 1], sizeof(char) * input_size);
                fwrite(buffer_write, sizeof(char), input_size * sizeof(char), file_after_decode);
                input_size = 0;
            }
        }
        fclose(file_after_decode);
        fclose(encoded_file);
        return 0;
    }
    D_tree *tree;
    tree = (D_tree *)malloc(sizeof(D_tree));
    tree->character = STOP_CHAR;
    tree->left = NULL;
    tree->right = NULL;
    deep_first_search_build_tree(tree, dfs_arr, &start_length, dfs_array_len);
    fseek(encoded_file, 1024, SEEK_SET);
    char buffer_read[DECODE_READING_BUFF_SIZE] = {'\0'};
    D_tree* search_tree = tree;
    // input_size is the input size
    int fread_return = 0, writing_index = 0, finish = 0;
    char output_char = 0;
    char buffer_write[DECODE_WRITING_BUFF_SIZE] = {'\0'};
    while(1){
        fread_return = fread(buffer_read, sizeof(char), DECODE_READING_BUFF_SIZE, encoded_file);
        if(!fread_return){
            break;
        }
        for(int i = 0; i != fread_return; i++){
            output_char = buffer_read[i];
            for(int j = 0; j != BIT_SPACE; j++){
                if((output_char & 128) == 0){
                    search_tree = search_tree->left;
                } else {
                    search_tree = search_tree->right;
                }
                if(search_tree->character != STOP_CHAR){
                    buffer_write[writing_index++] = (char)search_tree->character;
                    input_size--;
                    if(input_size == 0){
                        finish = 1;
                        fwrite(buffer_write, sizeof(char), writing_index * sizeof(char), file_after_decode);
                        break;
                    }
                    search_tree = tree;
                }
                if(writing_index == DECODE_WRITING_BUFF_SIZE){
                    fwrite(buffer_write, sizeof(char), writing_index * sizeof(char), file_after_decode);
                    writing_index = 0;
                }
                output_char <<= 1;
            }
            if(finish == 1)
                break;
        }
        if(finish == 1)
            break;
    }
    fclose(file_after_decode);
    fclose(encoded_file);
    free_decode_tree(tree);
    return 0;
}