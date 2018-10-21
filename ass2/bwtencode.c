#include "bwtencode.h"

/* create_filename
 * 
 * orginal_file_name is the orginal file name
 * index_dir is the path to temporary dir
 * temp_index_path is the path the index
 */
void create_filename(char *orginal_file_name, char *index_dir, char *temp_index_path, char *suffix) {
    strcpy(temp_index_path, index_dir);
    temp_index_path[strlen(temp_index_path)] = '/';
    strcat(temp_index_path, orginal_file_name);
    strcat(temp_index_path, suffix);
}
static void usage(char *name) {
    fprintf(stderr, "Command: %s delimiter tmp_folder orginal_file_path bwt_file_path\n"
                    "    delimiter is the character \'\\n\' or acsii from 32 to 126.\n"
                    "    tmp_foler is the folder that to be used to store index.\n"
                    "    orginal_file is the orginal file path\n"
                    "    bwt_file_path is the output path of encoded bwt file.\n", name);
}


// total 97 chars
int main(int argc, char **argv){
    if (argc != 5) {
        usage(argv[0]);
        return -1;
    }
    char delimiter;
    if (strlen(argv[1]) == 2)
        delimiter = (char)'\n';
    else
        delimiter = argv[1][0];
    char tmp_dir[strlen(argv[2]) + 1];
    strcpy(tmp_dir, argv[2]);
    char orginal_file_path[strlen(argv[3]) + 1];
    memset(orginal_file_path, '\0', (strlen(argv[3]) + 1));
    strcpy(orginal_file_path, argv[3]);
    char encoded_file_path[strlen(argv[4]) + 1];
    memset(encoded_file_path, '\0', strlen(argv[4]) + 1);
    strcpy(encoded_file_path, argv[4]);
    char index_file_path[strlen(encoded_file_path) + 5];
    memset(index_file_path, '\0', strlen(encoded_file_path) + 5);
    strcpy(index_file_path, encoded_file_path);
    strcat(index_file_path, ".aux");
    FILE *org_file = fopen(orginal_file_path, "rb");
    if (!org_file) {
        fprintf(stderr, "Invalid file name\n");
        return -1;
    }
    fseek(org_file, 0, SEEK_END);
    int file_size = (int)ftell(org_file);
    fseek(org_file, 0, SEEK_SET);
    // maximum 50 M
    encoder(delimiter, org_file, index_file_path, file_size, encoded_file_path, tmp_dir);
    fclose(org_file);
    return 0;
}
