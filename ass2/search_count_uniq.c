#include "bwtsearch.h"
/* search_count_uniq
 *
 * delimiter is the delimiter
 * encoded_path is the path of encoded file stream
 * temp_index_path is the path where stores temp index file
 * search_pattern is the search pattern
 * index_file is the id index file
 */
int search_count_uniq(char delimiter, FILE* encoded_file, char *temp_index_path, char *search_pattern) {
    fseek(encoded_file, 0, SEEK_END);
    int file_size = ftell(encoded_file);
    fseek(encoded_file, 0, SEEK_SET);
    int mapping_index[CHAR_LENGTH] = {0};
    int c_table[CHAR_LENGTH] = {0};
    int num_of_char = 0;
    if (file_size == 0) {
        printf("0\n");
    } else if (file_size <= 6 * 1024 * 1024) { // small than 6MB situation
        // handling the small file which can store all the info and load the read buff
        // do not build the index file
        char *read_buff = (char *)malloc(file_size * sizeof(char));
        int lower_index, upper_index, search_index, step_size, real_chunks_nums, occurrence = 0;
        int *occurrence_table;
        if(!(fread(read_buff, sizeof(char), file_size, encoded_file)))
            return -1;
        if (file_size <= 3 * 1024 * 1024) { // 3MB situation
            // not storing
            occurrence_table = search_sm_integrate(read_buff, file_size, encoded_file, &num_of_char, 
            mapping_index, &lower_index, &upper_index, &search_index, c_table, search_pattern, 
            &step_size, &real_chunks_nums);
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
            get_search_index(search_pattern, &search_index, &lower_index, &upper_index, c_table);
            occurrence_sm_get_bound(&lower_index, &upper_index, &search_index, occurrence_table,
                                &step_size, read_buff, c_table, search_pattern, mapping_index, num_of_char);
        }
        if (upper_index >= lower_index) {
            for (int i = lower_index; i <= upper_index; ++i) {
                int curr_index = i;
                while (1) {
                    int c = read_buff[curr_index];
                    int occ = occ_function_sm(occurrence_table, curr_index + 1, c,
                                    step_size, read_buff, mapping_index, num_of_char);
                    curr_index = c_table[c - 1] + occ - 1;
                    if (lower_index <= curr_index && curr_index <= upper_index)
                        break;
                    if (c == delimiter) {
                        ++occurrence;
                        break;
                    }
                }
            }
        }
        printf("%d\n", occurrence);
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
        int lower_index, upper_index, search_index, occurrence = 0;
        get_search_index(search_pattern, &search_index, &lower_index, &upper_index, c_table);
        occurrence_md_get_bound(&lower_index, &upper_index, &search_index, read_buff,
                                &step_size, search_index_file, c_table, search_pattern, 
                                mapping_index, num_of_char);
        if (upper_index >= lower_index) {
            // starting position of delimiter occurrence
            for (int i = lower_index; i <= upper_index; ++i) {
                int curr_index = i;
                while (1) {
                    int c = read_buff[curr_index];
                    int occ = occ_function_md(read_buff, curr_index + 1, c,
                                              step_size, search_index_file, mapping_index, num_of_char);
                    curr_index = c_table[c - 1] + occ - 1;
                    if (lower_index <= curr_index && curr_index <= upper_index)
                        break;
                    if (c == delimiter) {
                        ++occurrence;
                        break;
                    }
                }
            }
        }
        printf("%d\n", occurrence);
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
        int lower_index, upper_index, search_index;
        get_search_index(search_pattern, &search_index, &lower_index, &upper_index, c_table);
        int occurrence = 0;
        // use to get the lower bound and upper bound
        char *read_buff = (char *)malloc(sizeof(char) * REST_MEMORY);
        fseek(encoded_file, 0, SEEK_SET);
        if(!(fread(read_buff, sizeof(char), REST_MEMORY, encoded_file)))
            return -1;
        occurrence_lg_get_bound(&lower_index, &upper_index, &search_index, encoded_file,
                                &step_size, search_index_file, c_table, search_pattern, 
                                mapping_index, num_of_char, read_buff);
        if (upper_index >= lower_index) {
            // check on true of false
            for (int i = lower_index; i <= upper_index; ++i) {
                int curr_index = i;
                char c = '\0';
                int occ = 0;
                while (1) {
                    if (curr_index < REST_MEMORY) {
                        c = read_buff[curr_index];
                        occ = occ_function_md(read_buff, curr_index + 1, c,
                              step_size, search_index_file, mapping_index, num_of_char);
                    } else {
                        occ = occ_function_lg_with_char(encoded_file, curr_index + 1, &c,
                                    step_size, search_index_file, mapping_index, num_of_char);
                    }
                    curr_index = c_table[c - 1] + occ - 1;
                    if (lower_index <= curr_index && curr_index <= upper_index)
                        break;
                    if (c == delimiter) {
                        ++occurrence;
                        break;
                    }
                }
            }
        }
        free(read_buff);
        printf("%d\n", occurrence);
        fclose(search_index_file);
    }
    return 0;
}