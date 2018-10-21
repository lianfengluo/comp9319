#include "bwtsearch.h"
/* search_range
 *
 * delimiter is the delimiter
 * encoded_path is the path of encoded file stream
 * temp_index_path is the path where stores temp index file
 * search_pattern is the search pattern
 */
int search_range(char delimiter, FILE* encoded_file, char *temp_index_path, char *search_pattern, FILE* index_file) {
    fseek(encoded_file, 0, SEEK_END);
    int file_size = ftell(encoded_file);
    fseek(encoded_file, 0, SEEK_SET);
    int space_index = 0;
    while (search_pattern[space_index++] != ' ');
    char start_string[space_index];
    memset(start_string, '\0', sizeof(char) * space_index);
    for (int i = 0; i != space_index; ++i)
        start_string[i] = search_pattern[i];
    int start_pos = atoi(start_string);
    char end_string[strlen(search_pattern) - space_index + 1];
    memset(end_string, '\0', sizeof(char) * (strlen(search_pattern) - space_index + 1));
    for (size_t i = space_index; i != strlen(search_pattern); ++i)
        end_string[i - space_index] = search_pattern[i];
    int end_pos = atoi(end_string);
    fseek(index_file, 0, SEEK_END);
    int num_of_char = 0;
    int mapping_index[CHAR_LENGTH] = {0};
    int c_table[CHAR_LENGTH] = {0};
    // make it become index
    start_pos--;
    if (file_size == 0) {
        ;
    } else if (file_size <= 6 * 1024 * 1024) { // 6MB situation
        char *read_buff = (char *)malloc(file_size * sizeof(char));
        int step_size, real_chunks_nums;
        int *occurrence_table;
        if(!(fread(read_buff, sizeof(char), file_size, encoded_file)))
            return -1;
        // not storing anything
        if (file_size <= 3 * 1024 * 1024) { // 3MB situation
            for (int i = 0; i != file_size; ++i) {
                int c = read_buff[i];
                ++c_table[c];
            }
            // filling in the mapping_index with save somespace of the occurrence
            // counting the number of character
            cal_c_table(c_table, mapping_index, &num_of_char);
            // minux the c_table space
            // allocate our memory 
            int space_can_be_use = 15 * 1024 * 1024 - file_size - START_UP_SIZE;
            int max_chunks_nums = space_can_be_use / (num_of_char * sizeof(int));
            step_size = (file_size / max_chunks_nums == 0) ? 1 : ((file_size / max_chunks_nums) + 1);
            real_chunks_nums = file_size / step_size;
            occurrence_table = (int *)malloc((real_chunks_nums * num_of_char) * sizeof(int *));
            sm_file_statistic(read_buff, file_size, step_size, occurrence_table, num_of_char, mapping_index, real_chunks_nums);
        } else {
            FILE *search_index_file = fopen(temp_index_path, "rb");
            if (search_index_file) {
                occurrence_table = restore_data_sm(c_table, search_index_file, mapping_index, &num_of_char, file_size,
                                                &step_size, &real_chunks_nums);
                fclose(search_index_file);
            } else {
                // the first time create the index
                occurrence_table = storing_message_sm(c_table, temp_index_path, &num_of_char, &step_size,
                                                    &real_chunks_nums, mapping_index, file_size, read_buff);
            }
        }
        // starting index of search pattern last element
        for (int i = start_pos; i != end_pos; ++i) {
            int cur_index = c_table[(int)delimiter - 1] + i;
            char record[MAX_RECORD] = {'\0'};
            int record_index = 0;
            int c = read_buff[cur_index];
            while (c != delimiter) {
                int occ = occ_function_sm(occurrence_table, cur_index + 1, c,
                                step_size, read_buff, mapping_index, num_of_char);
                cur_index = c_table[c - 1] + occ - 1;
                record[record_index++] = c;
                c = read_buff[cur_index];
            }
            for (int j = record_index - 1; j != -1; j--) {
                printf("%c", record[j]);
            }
            printf("\n");
        }
        free(occurrence_table);
        free(read_buff);
    } else if (file_size <= 12 * 1024 * 1024) { //less than 12 MB
        // the circumstance that we need to store extra index
        // and also we have enough memory to get all the occurrence_table into memory
        // calculate the chunk size
        char *read_buff;
        int real_chunks_nums, step_size;
        FILE *search_index_file = fopen(temp_index_path, "rb");
        if (search_index_file) {
            restore_data_md_lg(c_table, search_index_file, mapping_index, &num_of_char, file_size,
                                               &step_size, &real_chunks_nums);
            read_buff = (char *)malloc(sizeof(char) * file_size);
            if(!(fread(read_buff, sizeof(char), file_size, encoded_file)))
                return -1;
        } else {
            // the first time create the index
            read_buff = storing_message_md(encoded_file, c_table, temp_index_path, &num_of_char, &step_size,
                                                  &real_chunks_nums, mapping_index, file_size);
            search_index_file = fopen(temp_index_path, "rb");
        }

        // starting index of search pattern last element
        for (int i = start_pos; i != end_pos; ++i) {
            int cur_index = c_table[(int)delimiter - 1] + i;
            char record[MAX_RECORD] = {'\0'};
            int record_index = 0;
            int c = read_buff[cur_index];
            while (c != delimiter) {
                int occ = occ_function_md(read_buff, cur_index + 1, c,
                                              step_size, search_index_file, mapping_index, num_of_char);

                cur_index = c_table[c - 1] + occ - 1;
                record[record_index++] = c;
                c = read_buff[cur_index];
            }
            for (int j = record_index - 1; j != -1; j--) {
                printf("%c", record[j]);
            }
            printf("\n");
        }
        free(read_buff);
        fclose(search_index_file);
    } else {
        // file more than 12MB
        int real_chunks_nums, step_size;
        FILE *search_index_file = fopen(temp_index_path, "rb");
        if (search_index_file) {
            restore_data_md_lg(c_table, search_index_file, mapping_index, &num_of_char, file_size,
                                               &step_size, &real_chunks_nums);
        } else {
            // the first time create the index
            storing_message_lg(encoded_file, c_table, temp_index_path, &num_of_char, &step_size,
                                            &real_chunks_nums, mapping_index, file_size);
            search_index_file = fopen(temp_index_path, "rb");
        } 
        char *read_buff = (char *)malloc(sizeof(char) * REST_MEMORY);
        fseek(encoded_file, 0, SEEK_SET);
        if(!(fread(read_buff, sizeof(char), REST_MEMORY, encoded_file)))
            return -1;
        for (int i = start_pos; i != end_pos; ++i) {
            int cur_index = c_table[(int)delimiter - 1] + i;
            char record[MAX_RECORD] = {'\0'};
            int record_index = 0;
            char c = '\0';
            int occ = 0;
            if (cur_index < REST_MEMORY) {
                c = read_buff[cur_index];
            } else {
                occ = occ_function_lg_with_char(encoded_file, cur_index + 1, &c,
                                            step_size, search_index_file, mapping_index, num_of_char);
            }
            while (c != delimiter) {
                if (cur_index < REST_MEMORY) {
                    occ = occ_function_md(read_buff, cur_index + 1, c,
                                            step_size, search_index_file, mapping_index, num_of_char);
                }
                cur_index = c_table[c - 1] + occ - 1;
                record[record_index++] = c;
                if (cur_index < REST_MEMORY) {
                    c = read_buff[cur_index];
                } else {
                    occ = occ_function_lg_with_char(encoded_file, cur_index + 1, &c,
                                            step_size, search_index_file, mapping_index, num_of_char);
                }
            }
            for (int j = record_index - 1; j != -1; j--) {
                printf("%c", record[j]);
            }
            printf("\n");
        }
        free(read_buff);
        fclose(search_index_file);
    }
    return 0;
}
