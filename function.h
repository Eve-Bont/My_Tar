#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

int count_string(char* name);
int verification_options(int argc, char* argument, char* option, int* option_f, int* archive_index, int* i);
int verification_file(char* argument, int* nb_files);
int verification_option_exist(char option);
int verification_options_combined(char option, int option_f);