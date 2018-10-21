#include "bwtsearch.h"

/* get_index_file_path
 *
 * encoded_filename is the name of encoded file
 * index_dir is the dir where stores temp index file
 */
void get_index_file_path(char *encoded_filename, char *index_dir, char *temp_index_path, char *suffix_name) {
    strcpy(temp_index_path, index_dir);
    temp_index_path[strlen(temp_index_path)] = '/';
    strcat(temp_index_path, encoded_filename);
    strcat(temp_index_path, suffix_name);
}
/* occ_function_sm
 *
 * occurrence_table is the occurrence table that have the occurrence
 * location is the location you want the set the count
 * character is the character you want to count
 * step_size the step of each index
 * file is the input file stream
 * mapping_index is the table map the large size index to small size
 * num_of_char is the num of character after deduction
 * return the number of times that the character occur before location in the file
 */
int occ_function_sm(int *occurrence_table, int location, char character, int step_size, char* read_buff, 
    int *mapping_index, int num_of_char) {
    int chunk_location = location / step_size;
    int occurrence = 0;
    int start_place = 0;
    if(chunk_location > 0) {
        occurrence = occurrence_table[(chunk_location - 1) * num_of_char + mapping_index[(int)character]];
        start_place = chunk_location * step_size;
    }
    for(int i = start_place; i < location; ++i) {
        if (read_buff[i] == character)
            ++occurrence;
    }
    return occurrence;
}
/* occ_function_md
 *
 * read_buff is bwt file stream buff
 * location is the location you want the set the count
 * character is the character you want to count
 * step_size the step of each index
 * file is the input index stream
 * mapping_index is the table map the large size index to small size
 * return the number of times that the character occur before location in the file
 * num_of_char is the num of character after deduction
 */
int occ_function_md(char *read_buff, int location, char character, int step_size, FILE* file, 
    int *mapping_index, int num_of_char) {
    int chunk_location = location / step_size;
    int occurrence = 0;
    int start_place = 0;
    if(chunk_location > 0){
        fseek(file, ((chunk_location - 1) * num_of_char + mapping_index[(int)character] + CHAR_LENGTH) * sizeof(int) , SEEK_SET);
        if (!(fread(&occurrence, sizeof(int), 1, file))) {
            fprintf(stderr, "Error reading\n");
        }
        start_place = chunk_location * step_size;
    }
    // the size of block we need to read
    for(int i = start_place; i < location; ++i) {
        if (read_buff[i] == character)
            ++occurrence;
    }
    return occurrence;
}

/* occ_function_lg
 *
 * encoded_file is the encoded file stream
 * location is the location you want the set the count
 * character is the character you want to count
 * step_size the step of each index
 * file is the input index stream
 * mapping_index is the table map the large size index to small size
 * return the number of times that the character occur before location in the file
 * num_of_char is the num of character after deduction
 */
int occ_function_lg(FILE *encoded_file, int location, char character, int step_size, FILE* index_file, 
    int *mapping_index, int num_of_char) {
    int chunk_location = location / step_size;
    int occurrence = 0;
    // if the search pattern is not is memory
    if(chunk_location > 0){
        fseek(index_file, ((chunk_location - 1) * num_of_char + mapping_index[(int)character] + CHAR_LENGTH) * sizeof(int),
            SEEK_SET);
        if(!(fread(&occurrence, sizeof(int), 1, index_file))) {
            fprintf(stderr, "Error reading!\n");
        }
        // start_place = chunk_location * step_size;
        fseek(encoded_file, (chunk_location * step_size), SEEK_SET);
        location %= step_size;
    } else {
        fseek(encoded_file, 0, SEEK_SET);
    }
    // the size of block we need to read
    char rest_buffer[location];
    if ((fread(rest_buffer, sizeof(char), sizeof(char) * location, encoded_file))) {
        for(int i = 0; i < location; ++i){
            if(rest_buffer[i] == character)
                ++occurrence;
        }
    }
    return occurrence;
}

/* occ_function_lg_uniq
 *
 * similar the last function but will get the current character
 */
int occ_function_lg_with_char(FILE *encoded_file, int location, char *character, int step_size, FILE* index_file, 
    int *mapping_index, int num_of_char) {
    int chunk_location = location / step_size;
    int occurrence = 0;
    location %= step_size;
    char rest_buffer[location];
    if(chunk_location > 0 && location != 0){
        fseek(encoded_file, (chunk_location * step_size), SEEK_SET);
    } else if (chunk_location > 0) {
        fseek(encoded_file, (chunk_location * step_size - 1), SEEK_SET);
    } else {
        fseek(encoded_file, 0, SEEK_SET);
    }
    // the size of block we need to read
    if (location == 0) {
        *character = fgetc(encoded_file);
    } else {
        if (!(fread(rest_buffer, sizeof(char), location, encoded_file))) {
            fprintf(stderr, "Err read \n");
            return -1;
        }
        *character = rest_buffer[location - 1];
    }
    // if the search pattern is not is memory
    if(chunk_location > 0){
        fseek(index_file, ((chunk_location - 1) * num_of_char + mapping_index[(int)(*character)] + CHAR_LENGTH) * sizeof(int),
            SEEK_SET);
        if(!(fread(&occurrence, sizeof(int), 1, index_file))) {
            fprintf(stderr, "Error reading!\n");
        }
    }
    for(int i = 0; i < location; ++i){
        if(rest_buffer[i] == *character)
            ++occurrence;
    }
    return occurrence;
}


/* cal_c_table
 *
 * c_table calculate the _table
 * mapping index downsizing the char space
 * num_of_char it will contain
 */
void cal_c_table(int *c_table, int *mapping_index, int *num_of_char) {
    for (int i = 9; i != CHAR_LENGTH; ++i) {
        c_table[i] += c_table[i - 1];
        if (c_table[i] != c_table[i - 1]) {
            mapping_index[i] = (*num_of_char)++;
        }
    }
}
/* get_search_index
 *
 * search_pattern is the search pattern for us
 * search_index is the last index of our search pattern
 * lower_index is the index first appear the last element of search pattern
 * upper_index is the index first appear the last element of search pattern
 * c_table is the counting table
 */
void get_search_index(char *search_pattern, int *search_index, int *lower_index, int *upper_index, int *c_table) {
    *search_index = (int)strlen(search_pattern) - 1;
    // starting index of search pattern last element
    *lower_index = c_table[(int)search_pattern[*search_index] - 1];
    // ending index of search pattern which will not get
    *upper_index = c_table[(int)search_pattern[*search_index]] - 1;
}
/* occurrence_sm_get_bound
 *
 * lower_index is the index first appear the last element of search pattern
 * upper_index is the index first appear the last element of search pattern
 * search_index is the last index of our search pattern
 * occurrence_table is the table that recording occurrence
 * step_size is the step of each indexes
 * read_buff is the buff we use to read the bwt text
 * c_table is the counting table
 * search_pattern is the pattern used for search
 */
void occurrence_sm_get_bound(int *lower_index, int *upper_index, int *search_index, int *occurrence_table, 
                    int *step_size, char *read_buff, int *c_table, char *search_pattern, int *mapping_index, int num_of_char) {
    int occurrence_1, occurrence_2;
    while (*lower_index <= *upper_index && *search_index >= 1) {
        int c = search_pattern[--(*search_index)];
        occurrence_1 = occ_function_sm(occurrence_table, *lower_index, c,
                                        *step_size, read_buff, mapping_index, num_of_char);
        occurrence_2 = occ_function_sm(occurrence_table, *upper_index + 1, c,
                                        *step_size, read_buff, mapping_index, num_of_char);
        *lower_index = c_table[c - 1] + occurrence_1;
        *upper_index = c_table[c - 1] + occurrence_2 - 1;
    }
}
/* occurrence_md_get_bound
 *
 * lower_index is the index first appear the last element of search pattern
 * upper_index is the index first appear the last element of search pattern
 * search_index is the last index of our search pattern
 * read_buff is the buff that will the original info
 * step_size is the step of each indexes
 * bwt_file is the bwt file stream
 * c_table is the counting table
 * search_pattern is the pattern used for search
 */
void occurrence_md_get_bound(int *lower_index, int *upper_index, int *search_index, char *read_buff, 
       int *step_size, FILE *index_file, int *c_table, char *search_pattern, int *mapping_index, int num_of_char) {
    int occurrence_1, occurrence_2;
    while (*lower_index <= *upper_index && *search_index >= 1) {
        int c = search_pattern[--(*search_index)];
        occurrence_1 = occ_function_md(read_buff, *lower_index, c,
                                    *step_size, index_file, mapping_index, num_of_char);
        occurrence_2 = occ_function_md(read_buff, *upper_index + 1, c,
                                    *step_size, index_file, mapping_index, num_of_char);
        *lower_index = c_table[c - 1] + occurrence_1;
        *upper_index = c_table[c - 1] + occurrence_2 - 1;
    }
}

/* occurrence_lg_get_bound
 *
 * lower_index is the index first appear the last element of search pattern
 * upper_index is the index first appear the last element of search pattern
 * search_index is the last index of our search pattern
 * bwt_file is the encoded file
 * step_size is the step of each indexes
 * bwt_file is the bwt file stream
 * c_table is the counting table
 * search_pattern is the pattern used for search
 */

void occurrence_lg_get_bound(int *lower_index, int *upper_index, int *search_index, FILE *bwt_file, 
       int *step_size, FILE *index_file, int *c_table, char *search_pattern, int *mapping_index, int num_of_char,
       char *read_buff) {
    int occurrence_1, occurrence_2;
    while (*lower_index <= *upper_index && *search_index >= 1) {
        char c = search_pattern[--(*search_index)];
        if (*lower_index < REST_MEMORY && read_buff) {
            occurrence_1 = occ_function_md(read_buff, *lower_index, c,
                                    *step_size, index_file, mapping_index, num_of_char);
        } else {
            occurrence_1 = occ_function_lg(bwt_file, *lower_index, c,
                                        *step_size, index_file, mapping_index, num_of_char);
        }
        if (*upper_index + 1 < REST_MEMORY && read_buff) {
            occurrence_2 = occ_function_md(read_buff, *upper_index + 1, c,
                                    *step_size, index_file, mapping_index, num_of_char);
        } else {
            occurrence_2 = occ_function_lg(bwt_file, *upper_index + 1, c,
                                        *step_size, index_file, mapping_index, num_of_char);
        }
        *lower_index = c_table[c - 1] + occurrence_1;
        *upper_index = c_table[c - 1] + occurrence_2 - 1;
    }
}


/* sm_file_statistic
 *
 * read_buff is the buffer of orginal txt
 * encoded_file is the file have been encoded
 * file_size is the bwt file size
 * step_size is the step size of index
 * occurrence_table
 * num_of_char is how many number it contains in the orginal file
 * mapping_index
 */
void sm_file_statistic(char *read_buff, int file_size, int step_size,
                        int *occurrence_table, int num_of_char, int *mapping_index, int real_chunks_nums) {
    int chunk_count = -1, step_count = step_size;
    for (int i = 0; i != file_size; ++i) {
        int c = mapping_index[(int)read_buff[i]];
        if (step_count == step_size) {
            step_count = 0;
            // move to next chunk
            if (++chunk_count == real_chunks_nums)
                break;
            // occurrence_table[chunk_count] = (int *)malloc(num_of_char * sizeof(int));
            if (chunk_count != 0) {
                for (int j = chunk_count * num_of_char; j != (chunk_count + 1) * num_of_char; ++j)
                    occurrence_table[j] = occurrence_table[j - num_of_char];
            }
        }
        ++step_count;
        ++occurrence_table[chunk_count * num_of_char + c];
    }
}

/* search_sm_integrate
 * preprocessing the small case 2 MB case
 * 
 * ...
 * return occurrence_table
 */
int *search_sm_integrate(char *read_buff, int file_size, FILE *encoded_file, int *num_of_char, 
    int *mapping_index, int *lower_index, int *upper_index, int *search_index, int *c_table, char *search_pattern, 
    int *step_size, int *real_chunks_nums) {
    for (int i = 0; i != file_size; i++) {
        int c = read_buff[i];
        c_table[c]++;
    }
    // filling in the mapping_index with save somespace of the occurrence
    // counting the number of character
    cal_c_table(c_table, mapping_index, num_of_char);
    // minux the c_table space
    // allocate our memory
    int space_can_be_use = 15 * 1024 * 1024 - file_size - START_UP_SIZE;
    int max_chunks_nums = space_can_be_use / ((*num_of_char) * sizeof(int));
    *step_size = (file_size / max_chunks_nums == 0) ? 1 : ((file_size / max_chunks_nums) + 1);
    *real_chunks_nums = file_size / *step_size;
    int *occurrence_table = (int *)malloc((*real_chunks_nums) * (*num_of_char) * sizeof(int *));
    fseek(encoded_file, 0, SEEK_SET);
    sm_file_statistic(read_buff, file_size, *step_size, occurrence_table, *num_of_char, mapping_index, *real_chunks_nums);
    get_search_index(search_pattern, search_index, lower_index, upper_index, c_table);
    // use to get the lower bound and upper bound
    occurrence_sm_get_bound(lower_index, upper_index, search_index, occurrence_table,
                            step_size, read_buff, c_table, search_pattern, mapping_index, *num_of_char);
    return occurrence_table;
}
/* restore_data_sm 
 * restore the median size file data
 * 
 * ...same as above variable name
 * return occurrence_table
 */

int *restore_data_sm(int *c_table, FILE* search_index_file, int *mapping_index, int *num_of_char, int file_size, 
    int *step_size, int *real_chunks_nums) {
    if (!(fread(c_table, sizeof(int), CHAR_LENGTH, search_index_file))) {
        fprintf(stderr, "Error reading!\n");
    }
    cal_c_table(c_table, mapping_index, num_of_char);
    int max_chunks_nums = (file_size - sizeof(int) * CHAR_LENGTH) / ((*num_of_char) * sizeof(int));
    *step_size = (int)(file_size / max_chunks_nums ) + 1;
    *real_chunks_nums = (int)file_size / (*step_size);
    int *occurrence_table = (int *)malloc((*real_chunks_nums) * (*num_of_char) * sizeof(int));
    if (!(fread(occurrence_table, sizeof(int), (*real_chunks_nums) * (*num_of_char), search_index_file))) {
        fprintf(stderr, "Error reading!\n");
    }
    return occurrence_table;
}

/* restore_data_md_lg
 * restore the median and large size file data
 * just retrieve the c_table and num_of_char
 * ...same as above variable name
 */
void restore_data_md_lg(int *c_table, FILE* search_index_file, int *mapping_index, int *num_of_char, int file_size, 
    int *step_size, int *real_chunks_nums) {
    if (!(fread(c_table, sizeof(int), CHAR_LENGTH, search_index_file))) {
        fprintf(stderr, "Error reading!\n");
    }
    cal_c_table(c_table, mapping_index, num_of_char);
    int max_chunks_nums = (file_size - sizeof(int) * CHAR_LENGTH) / ((*num_of_char) * sizeof(int));
    *step_size = (file_size / max_chunks_nums ) + 1;
    *real_chunks_nums = file_size / (*step_size);
}


/* storing_message_sm
 *
 * ...same as above
 * return occurrence_table
 */
int *storing_message_sm(int *c_table, char *temp_index_path, int *num_of_char,
    int *step_size, int *real_chunks_nums, int *mapping_index, int file_size, char *read_buff) {
    for (int i = 0; i != file_size; ++i) {
        int c = read_buff[i];
        ++c_table[c];
    }
    FILE *search_index_file = fopen(temp_index_path, "wb");
    fwrite(c_table, sizeof(int), CHAR_LENGTH, search_index_file);
    // filling in the mapping_index with save somespace of the occurrence
    // counting the number of character
    cal_c_table(c_table, mapping_index, num_of_char);
    // minux the c_table space
    // allocate our memory
    int space_can_be_use = (file_size - sizeof(int) * CHAR_LENGTH);
    int max_chunks_nums = space_can_be_use / ((*num_of_char) * sizeof(int));
    *step_size = (file_size / max_chunks_nums == 0) ? 1 : ((file_size / max_chunks_nums) + 1);
    *real_chunks_nums = file_size / *step_size;
    int *occurrence_table = (int *)malloc((*real_chunks_nums) * (*num_of_char) * sizeof(int));
    int chunk_count = -1, step_count = 0;
    int write_buff[*num_of_char];
    memset(write_buff, 0, sizeof(int) * (*num_of_char));
    for (int i = 0; i != file_size; ++i) {
        int c = mapping_index[(int)read_buff[i]];
        if (step_count == *step_size) {
            step_count = 0;
            // move to next chunk
            if(++chunk_count == *real_chunks_nums)
                break;
            fwrite(write_buff, sizeof(int), (*num_of_char), search_index_file);
            for (int j = 0; j != (*num_of_char); ++j)
                occurrence_table[chunk_count * (*num_of_char) + j] = write_buff[j];
        }
        ++step_count;
        ++write_buff[c];
    }
    fclose(search_index_file);
    return occurrence_table;
}
/* storing_message_md
 *
 * ...same as above
 * return read_buff
 */
char *storing_message_md(FILE *encoded_file, int *c_table, char *temp_index_path, int *num_of_char,
    int *step_size, int *real_chunks_nums, int *mapping_index, int file_size) {
    // if it's the first time
    char *read_buff = (char *)malloc(file_size * sizeof(char));
    if ((fread(read_buff, sizeof(char), file_size, encoded_file))) {
        for (int i = 0; i != file_size; ++i) {
            int c = read_buff[i];
            ++c_table[c];
        }
    }
    // store the c_table
    FILE *search_index_file = fopen(temp_index_path, "wb");
    fwrite(c_table, sizeof(int), CHAR_LENGTH, search_index_file);
    // filling in the mapping_index with save somespace of the occurrence
    // counting the number of character
    cal_c_table(c_table, mapping_index, num_of_char);
    // minus the c_table space and divide the size of num_of_char
    int max_chunks_nums = (file_size - sizeof(int) * CHAR_LENGTH) / ((*num_of_char) * sizeof(int));
    *step_size = (file_size / max_chunks_nums) + 1;
    *real_chunks_nums = file_size / (*step_size);
    int occurrence_table[*num_of_char];
    memset(occurrence_table, 0, sizeof(int) * (*num_of_char));
    int step_count = 0, chunk_count = -1;
    // create the occurrence table
    for (int i = 0; i != file_size; ++i) {
        int c = mapping_index[(int)read_buff[i]];
        if (step_count == *step_size) {
            step_count = 0;
            // move to next chunk
            if(++chunk_count == *real_chunks_nums)
                break;
            fwrite(occurrence_table, sizeof(int), (*num_of_char), search_index_file);
        }
        ++step_count;
        ++occurrence_table[c];
    }
    fclose(search_index_file);
    // store message
    return read_buff;
}
/* storing_message_lg
 *
 * ...same as above
 * return read_buff
 */
void storing_message_lg(FILE *encoded_file, int *c_table, char *temp_index_path, int *num_of_char,
                    int *step_size, int *real_chunks_nums, int *mapping_index, int file_size) {
    // if it's the first time
    char read_buff[READ_BUFF_SIZE];
    int read_size = 0;
    while ((read_size = fread(read_buff, sizeof(char), READ_BUFF_SIZE, encoded_file))) {
        for (int i = 0; i != read_size; ++i) {
            int c = read_buff[i];
            ++c_table[c];
        }
    }
    // store the c_table
    FILE *search_index_file = fopen(temp_index_path, "wb");
    fwrite(c_table, sizeof(int), CHAR_LENGTH, search_index_file);
    // filling in the mapping_index with save somespace of the occurrence
    // counting the number of character
    cal_c_table(c_table, mapping_index, num_of_char);
    // minus the c_table space and divide the size of num_of_char
    int max_chunks_nums = (file_size - sizeof(int) * CHAR_LENGTH) / ((*num_of_char) * sizeof(int));
    *step_size = (file_size / max_chunks_nums) + 1;
    *real_chunks_nums = file_size / (*step_size);
    int occurrence_table[*num_of_char];
    memset(occurrence_table, 0, sizeof(int) * (*num_of_char));
    int step_count = 0, chunk_count = -1;
    fseek(encoded_file, 0, SEEK_SET);
    // create the occurrence table
    while ((read_size = fread(read_buff, sizeof(char), READ_BUFF_SIZE, encoded_file))) {
        for (int i = 0; i != read_size; ++i) {
            int c = mapping_index[(int)read_buff[i]];
            if (step_count == *step_size) {
                step_count = 0;
                if(++chunk_count == *real_chunks_nums)
                    break;
                fwrite(occurrence_table, sizeof(int), (*num_of_char), search_index_file);
            }
            ++step_count;
            ++occurrence_table[c];
        }
    }
    fclose(search_index_file);
}
static void usage(char *name) {
    fprintf(stderr, "Command: %s delimiter bwt_file tmp_folder flag pattern\n"
                    "    delimiter is the character \'\\n\' or acsii from 32 to 126.\n"
                    "    bwt_file is the encoded bwt file.\n"
                    "    tmp_foler is the folder that to be used to store index.\n"
                    "    flag: -i, -m, -n, -a.\n"
                    "    pattern:\n"
                    "\t-i : range. i.e. '10 20' to retrieve the record that id which is 10 to 20.\n"
                    "\t-m : search pattern. Count how many match in orginal text with duplication.\n"
                    "\t-n : search pattern. Count how many match in record in orginal text without duplication.\n"
                    "\t-a : search pattern. Display the record id that match orginal text.\n", name);
}


int main(int argc, char **argv) {
    if (argc != 6) {
        usage(argv[0]);
        return -1;
    }
    char delimiter;
    if (strlen(argv[1]) != 1)
        delimiter = '\n';
    else
        delimiter = argv[1][0];

    FILE* encoded_file = fopen(argv[2], "rb");
    char *encoded_filename = basename(argv[2]);
    size_t temp_index_filename_len = strlen(encoded_filename) + 6 + strlen(argv[3]);
    char temp_index_path[temp_index_filename_len];
    memset(temp_index_path, '\0', sizeof(char) * temp_index_filename_len);
    get_index_file_path(encoded_filename, argv[3], temp_index_path, ".ser");
    // memset(index_file_path, '\0', sizeof(char) * index_filename_len);
    char index_file_path[strlen(argv[2]) + 5];
    memset(index_file_path, '\0', strlen(argv[2]) + 5);
    strcpy(index_file_path, argv[2]);
    strcat(index_file_path, ".aux");
    FILE *index_file = fopen(index_file_path, "rb");
    if (strcmp(argv[4], "-a") == 0) // exist id
        search_exist(delimiter, encoded_file, temp_index_path, argv[5], index_file);
    else if (strcmp(argv[4], "-m") == 0) // exist count with duplicate
        search_count_duplicate(encoded_file, temp_index_path, argv[5]);
    else if (strcmp(argv[4], "-n") == 0) // exist count without duplicate only with id
        search_count_uniq(delimiter, encoded_file, temp_index_path, argv[5]);
    else if (strcmp(argv[4], "-i") == 0) // search a range of word
        search_range(delimiter, encoded_file, temp_index_path, argv[5], index_file);
    if (index_file)
        fclose(index_file);
    fclose(encoded_file);
    return 0;
}