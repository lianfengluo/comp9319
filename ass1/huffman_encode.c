#include "huffman.h"

void _sort_probablity(int *arr1, int *arr2, size_t start, size_t end,
                                    int *rank1, int *rank2)
{
    if (start + 1 == end)
        return;
    size_t mid = (start + end) / 2;
    _sort_probablity(arr2, arr1, start, mid, rank2, rank1);
    _sort_probablity(arr2, arr1, mid, end, rank2, rank1);
    size_t i = start, j = mid, k = start;
    while (i != mid && j != end)
    {
        if (arr2[i] < arr2[j]){
            arr1[k] = arr2[j];
            rank1[k] = rank2[j];
            j++;
        }
        else{
            arr1[k] = arr2[i];
            rank1[k] = rank2[i];
            i++;
        }
        k++;
    }
    while (i != mid){
        arr1[k] = arr2[i];
        rank1[k] = rank2[i];
        k++;
        i++;
    }
    while (j != end){
        arr1[k] = arr2[j];
        rank1[k] = rank2[j];
        k++;
        j++;
    }
}
void sort_probablity(int* rank1, int *arr1)
{
    int arr2[CODING_SIZE];
    int rank2[CODING_SIZE];
    memcpy(arr2, arr1, CODING_SIZE * sizeof(int));
    memcpy(rank2, rank1, CODING_SIZE * sizeof(int));
    _sort_probablity(arr1, arr2, 0, CODING_SIZE, rank1, rank2);
}

void sort_huffman(int *arr, nnode **node_ptr, int size)
{ 
    for(int i = size - 2; i != size; i++){
        for(int j  = 0; j != i; j++){
            if(arr[j] < arr[i]){
                int temp = arr[j];
                arr[j] = arr[i];
                arr[i] = temp;
                nnode* temp_node = node_ptr[j];
                node_ptr[j] = node_ptr[i];
                node_ptr[i] = temp_node;
            } 
            else if (arr[i] == arr[j] && node_ptr[i]->height > node_ptr[j]->height) {
                nnode *temp_node = node_ptr[j];
                node_ptr[j] = node_ptr[i];
                node_ptr[i] = temp_node;
            }
        }
    }
}

nnode* build_tree(int frequency[], nnode** node_ptr, int char_appear){
    for(int i = char_appear - 1; i != 0; i--){
        nnode *new_node;
        new_node = (nnode *) malloc(sizeof(nnode));
        new_node->left = node_ptr[i - 1];
        new_node->right = node_ptr[i];
        new_node->character = STOP_CHAR;
        if (node_ptr[i - 1]->height < node_ptr[i]->height){
            new_node->height = node_ptr[i]->height + 1;
        }
        else
            new_node->height = node_ptr[i - 1]->height + 1;
        node_ptr[i - 1] = new_node;
        frequency[i - 1] += frequency[i];
        frequency[i] = 0;
        if(i != 1)
            sort_huffman(frequency, node_ptr, i);
    }
    if(char_appear == 1){
        nnode *new_node;
        new_node = (nnode *) malloc(sizeof(nnode));
        new_node->left = node_ptr[0];
        new_node->right = NULL;
        new_node->character = STOP_CHAR;
        new_node->height = 1;
        node_ptr[0] = new_node;
    }
    nnode *tree = node_ptr[0];
    return tree;
}

void exploit_tree(nnode *tree, char code_space[CODING_SIZE][CODED_LENGTH], int length[CODING_SIZE],
     char dfs_arr[], int *count_path){
    if(!tree->right||!tree->left){
        dfs_arr[*count_path] = '1';
        (*count_path)++;
        strcpy(code_space[tree->character], tree->code);
        dfs_arr[*count_path] = (char)tree->character;
        (*count_path)++;
        length[tree->character] = strlen(tree->code);
    } else {
        dfs_arr[*count_path] = '0';
        (*count_path)++;
    }
    if(tree->left){
        strcpy(tree->left->code, tree->code);
        tree->left->code[strlen(tree->left->code)] = '0';
        exploit_tree(tree->left, code_space, length, dfs_arr, count_path);
    }
    if(tree->right){
        strcpy(tree->right->code, tree->code);
        tree->right->code[strlen(tree->right->code)] = '1';
        exploit_tree(tree->right, code_space, length, dfs_arr, count_path);
    }
}

int encode(char* input_file_name, char* output_file){
    int count = 0;
    int input;
    FILE *input_file = fopen(input_file_name, "rb");
    if(input_file == NULL){
        return -1;
    }
    int arr[CODING_SIZE] = {0};
    while (1){
        if (EOF == (input = fgetc(input_file)))
            break;
        arr[input]++;
        count++;
    };
    int rank[CODING_SIZE];
    int char_appear = 0;
    for (int i = 0; i != CODING_SIZE; i++)
    {
        rank[i] = i;
        if (arr[i] != 0)
            char_appear++;
    }
    // if is a null file
    if(char_appear > 1){
        sort_probablity(rank, arr);
    }
    else if(char_appear == 1){
        for(int i = 0; i != CODING_SIZE; i++){
            if(arr[i] != 0){
                rank[0] = rank[i];
                rank[i] = 0;
                arr[0] = arr[i];
                arr[i] = 0;
            }
        }
    }
    // Just for fun
    char rest_space_note[] = "Our tutor is the best. Raymond is a great lecturer and interesting.\n";
    int statistic_space = 1024;
    // write the dictionary
    FILE* out_f = fopen(output_file, "wb");
    if(out_f == NULL)
        return -1;
    if(char_appear == 0){
        fputc('0', out_f);
        statistic_space--;
        while(statistic_space != 0){
            fputc(rest_space_note[(1024 - statistic_space) % 68], out_f);
            statistic_space--;
        }
        fclose(out_f);
        return -1;
    }
    nnode** node_ptr;
    node_ptr = (nnode **) malloc(sizeof(nnode*) * char_appear);
    for(int i = 0; i != char_appear; i++){
        node_ptr[i] = (nnode *)malloc(sizeof(nnode));
        node_ptr[i]->character = rank[i];
        node_ptr[i]->height = 0;
        node_ptr[i]->left = NULL;
        node_ptr[i]->right = NULL;
    }
    nnode* tree = build_tree(arr, node_ptr, char_appear);
    // just for initialization
    char code_space[CODING_SIZE][CODED_LENGTH];
    for(int i = 0; i != CODING_SIZE; i++){
        memset(code_space[i], 0, CODED_LENGTH);
    }
    int length[CODING_SIZE] = {0};
    char dfs_arr[DFS_SIZE] = {'\0'};
    int count_dfs_char = 0;
    if(char_appear == 1){
        strcpy(code_space[rank[0]], "0");
        length[rank[0]] = 1;
        dfs_arr[0] = '0';
        dfs_arr[1] = '1';
        dfs_arr[2] = rank[0];
        count_dfs_char = 3;
    }
    else{
        exploit_tree(tree, code_space, length, dfs_arr, &count_dfs_char);
    }
    for(int i = 0; i != char_appear; i++)
        free(node_ptr[i]);
    free(node_ptr);
    // write the dfs array length
    fprintf(out_f, "%d ",(int)count_dfs_char);
    // minus space
    statistic_space--;
    fwrite(dfs_arr, sizeof(char), count_dfs_char, out_f);
    statistic_space -= count_dfs_char;
    // minis the appear times
    while(count_dfs_char != 0){
        count_dfs_char /= 10;
        statistic_space--;
    }
    fprintf(out_f, "%d ", count);
    // minus space 1 bype
    statistic_space--;
    // minus the count
    // check the output length of count;
    int temp_count = count;
    while(temp_count!=0){
        temp_count /= 10;
        statistic_space--;
    }
    while(statistic_space != 0){
        fputc(rest_space_note[(1024 - statistic_space) % 68], out_f);
        statistic_space--;
    }

    fseek(input_file, 0, SEEK_SET);
    char buffer[OUTPUT_BUFF_SIZE + 1] = {'\0'};
    int output_char;
    int mini_count = 0;
    while(1){
        if ((output_char = fgetc(input_file)) == EOF)
            break;
        for(int j = 0; j != length[output_char]; j++){
            buffer[mini_count++] = code_space[output_char][j];
            if(mini_count == OUTPUT_BUFF_SIZE){
                long c = strtol(buffer, 0, 2);
                fputc(c, out_f);
                mini_count = 0;
            }
        }
    }
    if(mini_count % OUTPUT_BUFF_SIZE != 0){
        if(mini_count % BIT_SPACE != 0){
            while(mini_count % BIT_SPACE != 0){
                buffer[mini_count++] = '0';
            }
            buffer[mini_count] = '\0';
        }
        long c = strtol(buffer, 0, 2);
        fputc(c, out_f);
    }
    // printf("%d\n", count);
    fclose(out_f);
    fclose(input_file);

    return 0;
}