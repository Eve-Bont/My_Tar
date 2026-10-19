#include "function.h"

int count_string(char* name) {
    int i = 0, count = 0;
    while (name[i] != '\0') {
        count++;
        i++;
    }
    return count;
}

int verification_options(int argc, char* argument, char* option, int* option_f, int* archive_index, int* i) {
    for (int j = 1; argument[j] != '\0'; j++) {
        if (argument[j] == 'f') {
            if (*option_f == 1 || *i + 1 >= argc || argument[j+1] != '\0') {
                return erreur_option();
            }
            *option_f = 1;
            *archive_index = *i + 1;
            (*i)++;

        } else if (argument[j] == 'c' || argument[j] == 'r' || argument[j] == 't' || argument[j] == 'u' || argument[j] == 'x') {
            if (*option != '\0') {
                return erreur_option();
            }
            *option = argument[j];

        } else {
            return erreur_option();
        }
    }
    return 0;
}

int verification_file(char* argument, int* nb_files) {
    struct stat info;
    if (stat(argument, &info) == -1) {
        return erreur_stat(argument);
    }
    (*nb_files)++;
    return 0;
}

int verification_option_exist(char option) {
    if (option == '\0') {
        write(2, "my_tar: missing mode\n", 21);
        return 1;
    }
    return 0;
}

int verification_options_combined(char option, int option_f) {
    if ((option == 'r' || option == 'u') && option_f == 0) {
        write(2, "my_tar: option -f required\n", 27);
        return 1;
    }
    return 0;
}

int fill_tabl_files(char** files, char** argv, int argc, int archive_index) {
    int index = 0;
    int end_options = 0;
    for (int i = 1; i < argc; i++) {
        if (i == archive_index) {
            continue;
        }

        if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == '\0') {
            end_options = 1;
            continue;
        }

        if (argv[i][0] == '-' && argv[i][1] != '\0' && end_options == 0) {
            continue;
        } else {
            files[index] = argv[i];
            index++;
        }
    }
    return index;
}