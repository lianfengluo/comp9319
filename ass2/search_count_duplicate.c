#include "bwtsearch.h"
/* search_count_duplicate
 *
 * encoded_filepath is the path of encoded file
 * index_dir is the dir where stores temp index file
 * search_pattern is the search pattern
 */
int search_count_duplicate(FILE* encoded_file, char *temp_index_path, char *search_pattern) {
    fseek(encoded_file, 0, SEEK_END);
    int file_size = ftell(encoded_file);
    fseek(encoded_file, 0, SEEK_SET);
    int mapping_index[CHAR_LENGTH] = {0};
    int c_table[CHAR_LENGTH] = {0};
    int num_of_char = 0;
    if (file_size == 0) {
        printf("0\n");
    } else if (file_size <= 6 * 1024 * 1024) {
        // handling the small file which can store all the info and load the read buff
        // do not build the index file
        char *read_buff = (char *)malloc(file_size * sizeof(char));
        int lower_index, upper_index, search_index, step_size, real_chunks_nums, occurrence = 0;
        int *occurrence_table;
        if(!(fread(read_buff, sizeof(char), file_size, encoded_file)))
            return -1;
        // not storing anything
        if (file_size <= 888 * 1024) { // 888KB situation
            occurrence_table = search_sm_integrate(read_buff, file_size, encoded_file, &num_of_char, 
            mapping_index, &lower_index, &upper_index, &search_index, c_table, search_pattern, 
            &step_size, &real_chunks_nums);
        } else {
            // 888KB - 5.5 MB situation
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
        if (upper_index >= lower_index)
            occurrence = upper_index - lower_index + 1;
        free(occurrence_table);
        free(read_buff);
        printf("%d\n", occurrence);
    } else { 
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
        occurrence_lg_get_bound(&lower_index, &upper_index, &search_index, encoded_file,
                                &step_size, search_index_file, c_table, search_pattern, 
                                mapping_index, num_of_char, NULL);
        if (upper_index >= lower_index)
            occurrence = upper_index - lower_index + 1;
        printf("%d\n", occurrence);
        fclose(search_index_file);
    }
    return 0;
}