#include "bwtencode.h"


/* substring_cmp
 * 
 * In this function we are going to use the extend bwt compare method
 * The delimiter appear before should be sort before
 * 
 * string is the input file string
 * i1 is the index 1 of the suffix array
 * i2 is the index 2 of the suffix array
 * file_size is the at most comparsion we will do
 */
static int substring_cmp(char *string, int i1, int i2, int file_size, int delimiter) {
    for (int i = 0; i != file_size; ++i) {
        int c1 = string[i1 + i];
        int c2 = string[i2 + i];
        if (c1 < c2)
            return 0;
        else if (c1 > c2)
            return 1;
        // two char is the same
        // if these two characters is delimiter
        // i1 < i2 mean the delimiter of i1 is before i2
        else if (c1 == delimiter) {
            if (i1 < i2)
                return 0;
            else
                return 1;
        }
    }
    // the comparation will never get to here
    return 0;
}
/* _merge_sort
 * Classicial merge sort, use recursion and take two array
 * 
 */
static void _merge_sort(int *arr1, int *arr2, int start, int end, char *string, int size, int delimiter) {
    if ((start + 1) == end) {
        return;
    }
    int mid = (int)((start + end) / 2);
    _merge_sort(arr2, arr1, start, mid, string, size, delimiter);
    _merge_sort(arr2, arr1, mid, end, string, size, delimiter);
    int i = start, j = mid, k = start;
    while (i != mid && j != end) {
        if (substring_cmp(string, arr2[i], arr2[j], size, delimiter)) {
            arr1[k++] = arr2[j++];
        }
        else {
            arr1[k++] = arr2[i++];
        }
    }
    while (i != mid){
        arr1[k++] = arr2[i++];
    }
    while (j != end) {
        arr1[k++] = arr2[j++];
    }
}

/* merge_sort
 * 
 * arr is the orginal arr needed to be sorted
 * read_buff is the orginal text
 * size is the size of the arr.
 * start is the position start to sort
 * end is the position end to sort
 * file_size is the file_size
 */
static void merge_sort(int *arr, char *read_buff, int len, int start, int end, int file_size, char delimiter) {
    int *arr2 = (int *)malloc(sizeof(int) * len);
    memcpy(arr2, arr, sizeof(int) * len);
    _merge_sort(arr, arr2, start, end, read_buff, file_size, (int)delimiter);
    free(arr2);
}

/* encoder 
 * use the brute force method to sort the string
 * 
 * delimiter is the delimiter which used to sparate id
 * index_file_path is the index file path
 * file_size is the orginal file_size
 * encoded_file_name is the file name that after bwt
 * tmp_folder is the folder that we will use to store the temporary information
 */
int encoder(char delimiter, FILE *org_file, char *index_file_path, int file_size, char *encoded_file_name, char *tmp_folder) {
    if (file_size == 0) {
        FILE *emtpy = fopen(encoded_file_name, "wb");
        fclose(emtpy);
    }
    else if (file_size <= 26.5 * 1024 * 1024) { // when the file least than 26.5MB
        // Make the look up file
        int index_lookup_file_len = strlen(tmp_folder) + strlen(encoded_file_name) + 5;
        char index_lookup_filename[index_lookup_file_len];
        memset(index_lookup_filename, '\0', sizeof(char) * index_lookup_file_len);
        create_filename(encoded_file_name, tmp_folder, index_lookup_filename, ".lup");
        FILE *index_lookup_file = fopen(index_lookup_filename, "wb+");
        // 27MB
        char *read_buff = (char *)malloc(file_size * sizeof(char));
        int read_size = fread(read_buff, sizeof(char), sizeof(char) * file_size, org_file);
        if (!read_size)
            return -1;
        // 27 * 5 + startup = about 138MB
        int **sort_array = (int **)malloc(CHAR_LENGTH * sizeof(int *));
        int index_buff[WRITING_INDEX_BUFF] = {0};
        int index_count = 1;
        int write_index_count = 0;
        int c_table[CHAR_LENGTH] = {0};
        for (int i = 0; i != file_size; ++i) {
            if (read_buff[i] == delimiter) {
                // map the delimiter to the smallest value
                index_buff[write_index_count] = index_count++;
            }
            ++c_table[(int)read_buff[i]];
            ++write_index_count;
            if (write_index_count == WRITING_INDEX_BUFF) {
                fwrite(index_buff, sizeof(int), WRITING_INDEX_BUFF, index_lookup_file);
                write_index_count = 0;
            }
        }
        if (write_index_count != 0)
            fwrite(index_buff, sizeof(int), write_index_count, index_lookup_file);
        for (int i = 0; i != CHAR_LENGTH; ++i) {
            if (c_table[i])
                sort_array[i] = (int *)malloc(c_table[i] * sizeof(int));
            else
                sort_array[i] = NULL;
        }
        int count_index[CHAR_LENGTH] = {0};
        for (int i = 0; i != file_size; ++i) {
            int c = read_buff[i];
            int index = count_index[c];
            ++count_index[c];
            sort_array[c][index] = i;
        }
        // we don't need to sort the delimiter
        for (int i = 9; i != CHAR_LENGTH; ++i) {
            if (c_table[i] != 0 && i != delimiter){
                merge_sort(sort_array[i], read_buff, c_table[i], 0, c_table[i], file_size, delimiter);
            }
        }
        // This should keep in the memory
        // 138MB + 108MB = about 246MB
        int *index_array = (int *)malloc(sizeof(int) * file_size);
        fseek(index_lookup_file, 0, SEEK_SET);
        if(!(fread(index_array, sizeof(int), file_size, index_lookup_file))) {
            fprintf(stderr, "Error read the index\n");
        }
        fclose(index_lookup_file);
        FILE *bwt_file = fopen(encoded_file_name, "wb");
        FILE *index_file = fopen(index_file_path, "wb");
        char bwt_buff[WRITING_BUFFER];
        int bwt_count = 0;
        int code_index;
        index_count = 0;
        for (int i = 9; i != CHAR_LENGTH; ++i) {
            for (int j = 0; j != c_table[i]; ++j) {
                if (sort_array[i][j] == 0)
                    code_index = file_size - 1;
                else
                    code_index = sort_array[i][j] - 1;
                int c = read_buff[code_index];
                if (c == delimiter) {
                    // if it is a delimiter we have to look up the orgnial position
                    index_buff[index_count++] = index_array[code_index];
                    if (index_count == WRITING_INDEX_BUFF) {
                        fwrite(index_buff, sizeof(int), index_count, index_file);
                        index_count = 0;
                    }
                    // change it back to the orginal delimiter
                }
                bwt_buff[bwt_count++] = c;
                if (bwt_count == WRITING_BUFFER) {
                    fwrite(bwt_buff, sizeof(char), bwt_count, bwt_file);
                    bwt_count = 0;
                }
            }
        }
        if (bwt_count != 0)
            fwrite(bwt_buff, sizeof(char), bwt_count, bwt_file);
        if (index_count != 0)
            fwrite(index_buff, sizeof(int), index_count, index_file);
        free(index_array);
        fclose(index_file);
        fclose(bwt_file);
        // Free this two dimisional space
        for (int i = 0; i != CHAR_LENGTH; ++i) {
            if (c_table[i])
                free(sort_array[i]);
        }
        free(sort_array);
        free(read_buff);
        if (remove(index_lookup_filename))
            fprintf(stderr, "Fail to remove file %s", index_lookup_filename);
    } else {
        // larger than 26.5 MB
        char *input_string = (char *)malloc(file_size * sizeof(char));
        int read_size = fread(input_string, sizeof(char), sizeof(char) * file_size, org_file);
        if (!read_size)
            return -1;
        int c_table[CHAR_LENGTH] = {0};
        // counting the word for allocation
        for (int i = 0; i != file_size; ++i) {
            ++c_table[(int)input_string[i]];
        }
        int delimiter_size = c_table[(int)delimiter];
        free(input_string);
        int **sort_array = (int **)malloc(CHAR_LENGTH * sizeof(int *));
        // used space 50 MB + startup space
        // allocate the space
        // used space 200 MB + startup space
        for (int i = 0; i != CHAR_LENGTH; ++i) {
            if (c_table[i])
                sort_array[i] = (int *)malloc(c_table[i] * sizeof(int));
            else
                sort_array[i] = NULL;
        }
        // get the responsing space
        int count_index[CHAR_LENGTH] = {0};
        char read_buff[READING_BUFFER];
        read_size = 0;
        fseek(org_file, 0, SEEK_SET);
        int accumulator = 0;
        while ((read_size = fread(read_buff, sizeof(char), READING_BUFFER, org_file))) {
            for (int i = 0; i != read_size; ++i) {
                int c = read_buff[i];
                int index = count_index[c]++;
                sort_array[c][index] = accumulator++;
            }
        }
        int tmp_file_len0 = strlen(tmp_folder) + strlen(encoded_file_name) + 5;
        char tmp_filename0[tmp_file_len0];
        memset(tmp_filename0, '\0', sizeof(char) * tmp_file_len0);
        // creat the tmp_file than store the suffix with the delimiter sorted in the very beginning
        create_filename(encoded_file_name, tmp_folder, tmp_filename0, ".tm0");
        FILE *tmp_file0 = fopen(tmp_filename0, "wb");

        int tmp_file_len1 = strlen(tmp_folder) + strlen(encoded_file_name) + 5;
        char tmp_filename1[tmp_file_len1];
        memset(tmp_filename1, '\0', sizeof(char) * tmp_file_len1);
        // creat the tmp_file than store the suffix with the delimiter sorted in the very beginning
        create_filename(encoded_file_name, tmp_folder, tmp_filename1, ".tm1");
        FILE *tmp_file1 = fopen(tmp_filename1, "wb");
        for (int i = 9; i != CHAR_LENGTH; ++i) {
            if (c_table[i])
                fwrite(sort_array[i], sizeof(int), c_table[i], tmp_file0);       
        }
        for (int i = 0; i != CHAR_LENGTH; ++i) {
            if (c_table[i])
                free(sort_array[i]);
        }
        free(sort_array);
        // start to read
        fseek(org_file, 0, SEEK_SET);
        input_string = (char *)malloc(file_size * sizeof(char));
        read_size = fread(input_string, sizeof(char), file_size, org_file);
        if (!read_size)
            return -1;
        fclose(tmp_file0);
        tmp_file0 = fopen(tmp_filename0, "rb");
        fseek(tmp_file0, 0, SEEK_SET);
        int *char_sort_array;
        for (int i = 9; i != CHAR_LENGTH; ++i) {
            if (i == delimiter) {
                // we are not sorting the delimiter
                char_sort_array = (int *)malloc(sizeof(int) * c_table[i]);
                if (!(fread(char_sort_array, sizeof(int), c_table[i], tmp_file0))) {
                    fprintf(stderr, "Error Reading\n");
                }
                fwrite(char_sort_array, sizeof(int), c_table[i], tmp_file1);
                free(char_sort_array);
                continue;    
            }
            if (c_table[i]) {
                char_sort_array = (int *)malloc(sizeof(int) * c_table[i]);
                if (!(fread(char_sort_array, sizeof(int), c_table[i], tmp_file0))) {
                    fprintf(stderr, "Error Reading\n");
                }
                merge_sort(char_sort_array, input_string, c_table[i], 0, c_table[i], file_size, delimiter);
                fwrite(char_sort_array, sizeof(int), c_table[i], tmp_file1);
                free(char_sort_array);
            }
        }
        int read_index_buff[READING_INDEX_BUFF];
        char bwt_buff[WRITING_BUFFER];
        int bwt_count = 0;
        FILE *bwt_file = fopen(encoded_file_name, "wb");
        int *delimiter_index_array = (int *)malloc(sizeof(int) * delimiter_size);
        int delimiter_count = 0;
        fclose(tmp_file0);
        // clean the content of the file
        fclose(tmp_file1);
        tmp_file1 = fopen(tmp_filename1, "rb");
        while ((read_size = fread(read_index_buff, sizeof(int), READING_INDEX_BUFF, tmp_file1))) {
            for (int i = 0; i != read_size; ++i) {
                int index = read_index_buff[i] - 1;
                if (index == -1)
                    index = file_size - 1;
                int c = input_string[index];
                if (c == delimiter) {
                    delimiter_index_array[delimiter_count++] = index;
                }
                bwt_buff[bwt_count++] = c;
                if (bwt_count == WRITING_BUFFER) {
                    fwrite(bwt_buff, sizeof(char), WRITING_BUFFER, bwt_file);
                    bwt_count = 0;
                }
            }
        }
        if (bwt_count != 0) {
            fwrite(bwt_buff, sizeof(char), bwt_count, bwt_file);
        }
        if (delimiter_count != delimiter_size) {
            fprintf(stderr, "Error!!\n");
        }
        fclose(tmp_file1);
        if (remove(tmp_filename1))
            fprintf(stderr, "Fail to remove file %s", tmp_filename0);
        free(input_string);
        tmp_file0 = fopen(tmp_filename0, "wb");
        fwrite(delimiter_index_array, sizeof(int), delimiter_size, tmp_file0);
        fclose(tmp_file0);
        int *index_lookup_buff = (int *)malloc(file_size * sizeof(int));
        accumulator = 0;
        int delimiter_id = 1;
        fseek(org_file, 0, SEEK_SET);
        while ((read_size = fread(read_buff, sizeof(char), READING_BUFFER, org_file))) {
            for (int i = 0; i != read_size; ++i) {
                if (read_buff[i] == delimiter)
                    index_lookup_buff[accumulator] = delimiter_id++;
                ++accumulator;
            }
        }
        tmp_file0 = fopen(tmp_filename0, "rb");
        FILE *index_file = fopen(index_file_path, "wb");
        int index_buff[WRITING_INDEX_BUFF] = {0};
        int index_count = 0;
        while ((read_size = fread(read_index_buff, sizeof(int), READING_INDEX_BUFF, tmp_file0))) {
            for (int i = 0; i != read_size; ++i) {
                index_buff[index_count++] = index_lookup_buff[read_index_buff[i]];
                if (index_count == WRITING_INDEX_BUFF) {
                    fwrite(index_buff, sizeof(int), WRITING_INDEX_BUFF, index_file);
                    index_count = 0;
                }
            }
        }
        if (index_count != 0) {
            fwrite(index_buff, sizeof(int), index_count, index_file);
        }
        fclose(tmp_file0);
        free(index_lookup_buff);
        fclose(index_file);
        if (remove(tmp_filename0))
            fprintf(stderr, "Fail to remove file %s", tmp_filename0);
    }
    return 0;
}