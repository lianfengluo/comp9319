#include "bwtsearch.h"

/* _merge_sort 
 *
 * arr1 is the orginal array
 * arr2 is the extra array being used to sort
 * start is the starting position to sort
 * end is the ending position to sort
 */
static void _merge_sort(int *arr1, int *arr2, int start, int end) {
    if (start + 1 == end)
        return;
    int mid = (start + end) / 2;
    _merge_sort(arr2, arr1, start, mid);
    _merge_sort(arr2, arr1, mid, end);
    int i = start, j = mid, k = start;
    while (i != mid && j != end) {
        if (arr2[i] > arr2[j]) 
            arr1[k++] = arr2[j++];
        else
            arr1[k++] = arr2[i++];
    }
    while (i != mid)
        arr1[k++] = arr2[i++];
    while (j != end)
        arr1[k++] = arr2[j++];
}

/* merge_sort
 * 
 * arr is the orginal arr needed to be sorted
 * size is the size of the arr.
 */
static void merge_sort(int *arr, int size) {
    int arr2[size];
    memcpy(arr2, arr, sizeof(int) * size);
    _merge_sort(arr, arr2, 0, size);
}

/* search_exist
 *
 * delimiter is the delimiter
 * encoded_path is the path of encoded file stream
 * temp_index_path is the path where stores temp index file
 * search_pattern is the search pattern
 */
int search_exist(char delimiter, FILE* encoded_file, char *temp_index_path, char *search_pattern, FILE* index_file) {
    fseek(encoded_file, 0, SEEK_END);
    int file_size = ftell(encoded_file);
    rewind(encoded_file);
    fseek(index_file, 0, SEEK_END);
    int size_of_delimiter = (int)(ftell(index_file) / 4);
    int mapping_index[CHAR_LENGTH] = {0};
    int c_table[CHAR_LENGTH] = {0};
    int num_of_char = 0;
    if (file_size == 0) {
        ;
    } else if (file_size <= 6 * 1024 * 1024) { // small than 6MB situation
        // handling the small file which can store all the info and load the read buff
        // do not build the index file
        char *read_buff = (char *)malloc(file_size * sizeof(char));
        int lower_index, upper_index, search_index, step_size, real_chunks_nums;
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
            int output_queue[MAX_RECORD];
            int queue_index = 0;
            for (int i = lower_index; i <= upper_index; i++) {
                int curr_index = i;
                int id = 0;
                while (1) {
                    int c = read_buff[curr_index];
                    int occ = occ_function_sm(occurrence_table, curr_index + 1, c,
                                    step_size, read_buff, mapping_index, num_of_char);
                    curr_index = c_table[c - 1] + occ - 1;
                    if (lower_index <= curr_index && curr_index <= upper_index)
                        break;
                    if (c == delimiter) {
                        fseek(index_file, (occ - 1) * sizeof(int), SEEK_SET);
                        if (!(fread(&id, sizeof(int), 1, index_file))) {
                            fprintf(stderr, "Err!\n");
                            return -1;
                        }
                        if (id + 1 > size_of_delimiter)
                            output_queue[queue_index++] = 1;
                        else
                            output_queue[queue_index++] = id + 1;
                        break;
                    }
                }
            }
            merge_sort(output_queue, queue_index);
            for (int i = 0; i != queue_index; i++)
                printf("%d\n", output_queue[i]);
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
        int lower_index, upper_index, search_index;
        get_search_index(search_pattern, &search_index, &lower_index, &upper_index, c_table);
        occurrence_md_get_bound(&lower_index, &upper_index, &search_index, read_buff,
                                &step_size, search_index_file, c_table, search_pattern, 
                                mapping_index, num_of_char);
        if (upper_index >= lower_index) {
            // check on true of false
            int output_queue[MAX_RECORD];
            int queue_index = 0;
            for (int i = lower_index; i <= upper_index; i++) {
                int curr_index = i;
                int id = 0;
                while (1) {
                    int c = read_buff[curr_index];
                    int occ = occ_function_md(read_buff, curr_index + 1, c,
                                              step_size, search_index_file, mapping_index, num_of_char);
                    curr_index = c_table[c - 1] + occ - 1;
                    if (lower_index <= curr_index && curr_index <= upper_index)
                        break;
                    if (c == delimiter) {
                        fseek(index_file, (occ - 1) * sizeof(int), SEEK_SET);
                        if (!(fread(&id, sizeof(int), 1, index_file))) {
                            fprintf(stderr, "Err!\n");
                            return -1;
                        }
                        if (id + 1 > size_of_delimiter)
                            output_queue[queue_index++] = 1;
                        else
                            output_queue[queue_index++] = id + 1;
                        break;
                    }
                }
            }
            merge_sort(output_queue, queue_index);
            for (int i = 0; i != queue_index; i++)
                printf("%d\n", output_queue[i]);
        }
        fclose(search_index_file);
        free(read_buff);
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
        char *read_buff = (char *)malloc(sizeof(char) * REST_MEMORY);
        fseek(encoded_file, 0, SEEK_SET);
        if(!(fread(read_buff, sizeof(char), REST_MEMORY, encoded_file)))
            return -1;
        // use to get the lower bound and upper bound
        occurrence_lg_get_bound(&lower_index, &upper_index, &search_index, encoded_file,
                                &step_size, search_index_file, c_table, search_pattern, 
                                mapping_index, num_of_char, read_buff);
        if (upper_index >= lower_index) {
            // check on true of false
            int output_queue[MAX_RECORD];
            int queue_index = 0;
            for (int i = lower_index; i <= upper_index; i++) {
                int curr_index = i;
                int id = 0;
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
                        fseek(index_file, (occ - 1) * sizeof(int), SEEK_SET);
                        if (!(fread(&id, sizeof(int), 1, index_file))) {
                            fprintf(stderr, "Err!\n");
                            return -1;
                        }
                        if (id + 1 > size_of_delimiter)
                            output_queue[queue_index++] = 1;
                        else
                            output_queue[queue_index++] = id + 1;
                        break;
                    }
                }
            }
            merge_sort(output_queue, queue_index);
            for (int i = 0; i != queue_index; i++)
                printf("%d\n", output_queue[i]);
        }
        free(read_buff);
        fclose(search_index_file);
    }
    return 0;
}
