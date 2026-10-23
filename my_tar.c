#include "function.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        write(2, "my_tar: missing options\n", 24);
        return 1;
    }

    char option = '\0';
    int option_f = 0;
    int archive_index = -1;
    int end_options = 0;
    int nb_files = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == '\0') {
            end_options = 1;
            continue;
        }
        if (argv[i][0] == '-' && argv[i][1] != '\0' && end_options == 0) {
            if (verification_options(argc, argv[i], &option, &option_f, &archive_index, &i)) {
                return 1;
            }
        } else {
            if (verification_file(argv[i], &nb_files)) {
                return 1;
            }
        }
    }

    if(verification_option_exist(option) || verification_options_combined(option, option_f)) {
        return 1;
    }

    char** files = malloc(sizeof(char*) * nb_files);
    if (files == NULL) {
        text_error("erreur malloc\n");
        return 1;
    }
    if (fill_tabl_files(files, argv, argc, archive_index) != nb_files) {
        free(files);
        return 1;
    }

    int archive_open = 1;
    if (option_f == 1) {
        if (option == 'c') {
            if(option_c(files, argv[archive_index], nb_files)) {
                free(files);
                return 1;
            }
        } else if (option == 'r') {
            
        } else if (option == 't') {
            
        } else if (option == 'u') {
            
        } else if (option == 'x') {
            
        }
    } else {
        if (option == 'c') {
            
        } else if (option == 'r') {
            
        } else if (option == 'x') {
            
        }
    }


    free(files);
    return 0;
}