#include"huffman.h"

int check_match(int search_buffer[], char *search_pattern, int start_index, int size){
    for(int i = 0; i != size; i++){
        // printf("%c", search_buffer[(start_index + i) % size]);
        if(search_buffer[(start_index - i) % size] != search_pattern[size - 1 - i])
            return 0;
    }
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
    int null_c = STOP_CHAR;
    int search_pattern_size = (int)strlen(searching_pattern);
    char out_buff[DECODE_SIZE * 2]={'\0'};
    char current_code[CODE_LENGTH] = {'\0'};
    int index = 0;      // index is the get code index
    int decode_index = 0;
    int decode_char;
    int found;
    int index_p1 = 0;
    int search_buffer[SEARCH_PATTERN_MAX] = {0};
    // use for as search buff index
    int search_index = 0;
    int match_count = 0;
    while((c=fgetc(search_file))!=EOF){
        getbinay(c, out_buff, &index);
        if(index > DECODE_SIZE){
            index_p1 = index + 1;
            while(1){
                found = 0;
                for(int i = 0; decode_index + i != index_p1; i++){
                    current_code[i] = out_buff[decode_index + i];
                    if ((decode_char = find_code(current_code, *tree, i)) != null_c)
                    {
                        // search place
                        search_buffer[search_index % search_pattern_size] = decode_char;
                        if(check_match(search_buffer, searching_pattern, 
                        search_index, search_pattern_size))
                            match_count++;
                        search_index++;
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
                // printf("%c", decode_char);
                search_buffer[search_index % search_pattern_size] = decode_char;
                // printf("\nsearch mod%d\n", search_index % search_pattern_size);
                if(check_match(search_buffer, searching_pattern, 
                search_index, search_pattern_size))
                    match_count++;
                search_index++;
                input_size--;
                decode_index += i;
                break;
            }
        }
    }
    printf("%d\n", match_count);
    fclose(search_file);
    return 0;
}