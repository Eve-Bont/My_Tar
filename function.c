#include "function.h"
/*
OPTIONS VALIDATION
*/

int verification_options(int argc, char* argument, char* option, int* option_f, int* archive_index, int* i) {
    for (int j = 1; argument[j] != '\0'; j++) {
        if (argument[j] == 'f') {
            if (*option_f == 1 || *i + 1 >= argc || argument[j+1] != '\0') {
                return error_option();
            }
            *option_f = 1;
            *archive_index = *i + 1;
            (*i)++;

        } else if (argument[j] == 'c' || argument[j] == 'r' || argument[j] == 't' || argument[j] == 'u' || argument[j] == 'x') {
            if (*option != '\0') {
                return error_option();
            }
            *option = argument[j];

        } else {
            return error_option();
        }
    }
    return 0;
}

int verification_file(char* argument, int* nb_files) {
    struct stat info;
    if (stat(argument, &info) == -1) {
        error_stat(argument);
        return 1;
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

/*
ARCHIVE VALIDATION
*/

int validate_archive_format(int archive_open, struct stat* info) {
    if (fstat(archive_open, &info) == -1) {
        return 1;
    }
    if (!S_ISREG(info->st_mode) || info->st_size < 1024 || info->st_size % 512 != 0) {
        return 1;
    }
    return 0;
}

int validate_archive(int archive_open, struct stat info) {
    off_t position = 0;
    char header[512];

    while (position < info.st_size) {
        if (read_all(archive_open, header, 512)) {
            return 1;
        }
        position += 512;
        if (verify_bloc_0(header)) {
            return verify_archive_end(archive_open, position, info);
        }
        if (verify_archive_file(archive_open, header, &position, info)) {
            return 1;
        }
    }
    return 1;
}

/*
FILLING ARCHIVE
*/

int fill_header(char* header, struct stat info, char* file) {
    if (name(file, header) || 
        mode(info.st_mode & 0777, header) || 
        uid(info.st_uid, header) || 
        gid(info.st_gid, header) || 
        size((S_ISDIR(info.st_mode) || S_ISLNK(info.st_mode)? 0 : info.st_size), header) || 
        mtime(info.st_mtime, header) || 
        typeflag(info.st_mode, header) || 
        (S_ISLNK(info.st_mode)? linkname(file, header) : 0) || 
        magic(header) || 
        version(header) || 
        uname(info.st_uid, header) || 
        gname(info.st_gid, header) || 
        devmajor(0, header) || 
        devminor(0, header) || 
        chksum(header)) {
        return 1;
    }
    return 0;
}

int fill_archive_header(char* file, int archive_open) {
    struct stat info;
    if(lstat(file, &info) == -1) {
        error_stat(file);
        return 1;
    }
    char header[512] = {'\0'};
    if(fill_header(header, info, file)) {
        return 1;
    }
    if (write_all(archive_open, header, 512)) {
        text_error("my_tar: write error\n");
        return 1;
    }
    return 0;
}

int fill_archive(char* file, int archive_open) {
    struct stat info;
    if(lstat(file, &info) == -1) {
        error_stat(file);
        return 1;
    }
    if(S_ISREG(info.st_mode)) {
        int file_open = open(file, O_RDONLY);
        if (file_open == -1) {
            error_opening(file);
            return 1;
        }
        char buffer[4096];
        ssize_t size_file;
        while((size_file = read(file_open, buffer, 4096)) > 0) {
            if (write_all(archive_open, buffer, size_file)) {
                text_error("my tar: write error\n");
                close(file_open);
                return 1;
            }
        }
        if(size_file == -1) {
            text_error("my tar: read error\n");
            close(file_open);
            return 1;
        }
        close(file_open);
    }
    return 0;
}

int padding(char* file, int archive_open) {
    struct stat info;
    if(lstat(file, &info) == -1) {
        error_stat(file);
        return 1;
    }
    int size_end = 0;
    if(S_ISREG(info.st_mode)) {
        size_end = (512 - (info.st_size % 512)) % 512;
    }
    if (size_end != 0) {
        char buffer[511] = {'\0'};
        if (write_all(archive_open, buffer, size_end)) {
            return 1;
        }
    }
    return 0;
}

int fill_archive_end(int archive_open) {
    char buffer_end[1024] = {'\0'};
    if(write_all(archive_open, buffer_end, 1024)) {
        return 1;
    }
    return 0;
}

/*
OTHERS
*/

int count_string(char* name) {
    int i = 0, count = 0;
    while (name[i] != '\0') {
        count++;
        i++;
    }
    return count;
}

int read_all(int archive_open, char* buffer, size_t size) {
    size_t total = 0;
    while (total < size) {
        ssize_t reading = read(archive_open, buffer + total, size - total);
        if (reading <= 0) {
            return 1;
        }
        total += reading;
    }
    return 0;
}

int write_all(int descriptor, char* buffer, size_t size) {
    size_t total = 0;
    while(total < size) {
        ssize_t written = write(descriptor, buffer + total, size - total);
        if (written <= 0) {
            return 1;
        }
        total += written;
    }
    return 0;
}

int get_file_info(char* header, unsigned long* info, int index_begin, int index_end) {
    *info = 0;
    for (int i = index_begin; i < index_end; i++) {
        if (header[i] < '0' || header[i] > '7') {
            return 0;
        }
        *info = *info * 8 + (header[i] - '0');
    }
    if (header[index_end] != '\0') {
        return 0;
    }
    return 1;
}