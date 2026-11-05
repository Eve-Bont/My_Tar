int option_u(char* archive, char** files, int nb_files) {
    int archive_open = open(archive, O_RDWR);
    struct stat info;
    if (archive_open == -1) {
        error_opening(archive);
        return 1;
    }
    if (validate_archive_format(archive_open, &info) || 
        validate_archive(archive_open, info)) {
        close(archive_open);
        return text_error("my_tar: invalid archive\n");
    }
    for(int index_file = 0; index_file < nb_files; index_file++) {
        if(lseek(archive_open, 0, SEEK_SET) == -1 || update_archive(archive_open, files[index_file])) {
            close(archive_open);
            return 1;
        }
    }
    close(archive_open);
    return 0;
}

/*
FUNCTIONS
*/

int update_archive(int archive_open, char* file) {
    unsigned long mtime_archive = 0;
    int found = search_file(archive_open, file, &mtime_archive);
    if (found == -1)
        return 1;
    if (found) {
        int newer = compare_file_mtime(mtime_archive, file);
        if (newer == -1)
            return 1;
        if (newer)
            return updating(archive_open, file);
    }
    return 0;
}

int search_file(int archive_open, char* file, unsigned long *mtime_archive) {
    int found = 0;

    while (1) {
        char header[512];
        if (read_all(archive_open, header, 512))
            return -1;
        if (verify_bloc_0(header))
            return found;
        int result = check_file(header, file, mtime_archive);
        if(result == -1)
            return -1;
        if(result)
            found = 1;
        if (jump_file(archive_open, header))
            return -1;
    }
}

int check_file(char* header, char* file, unsigned long *mtime_archive) {
    char name_file[257] = {'\0'};
    construct_name(header, name_file);
    if (!compare_file_name(name_file, file))
        return 0;
    if (!get_file_info(header, mtime_archive, 136, 147))
        return -1;
    return 1;
}

void construct_name(char* header, char* name_file) {
    int i = 0;
    int j = 0;
    while (j < 155 && header[345 + j] != '\0') {
        name_file[i] = header[345 + j];
        i++;
        j++;
    }
    if (i > 0)
        name_file[i++] = '/';

    j = 0;
    while (j < 100 && header[j] != '\0') {
        name_file[i] = header[j];
        i++;
        j++;
    }
    name_file[i] = '\0';
}

int compare_file_name(char* string1, char* string2) {
    int size_string1 = count_string(string1);
    if(size_string1 != count_string(string2)) {
        return 0;
    }
    for (int i = 0; i < size_string1; i++) {
        if(string1[i] != string2[i]) {
            return 0;
        }
    }
    return 1;
} 

int compare_file_mtime(unsigned long mtime_file, char* file) {
    struct stat info;
    if (lstat(file, &info) == -1) {
        error_stat(file);
        return -1;
    }
    return info.st_mtime > mtime_file;
}

int updating(int archive_open, char* file) {
    if (lseek(archive_open, -1024, SEEK_END) == -1)
        return 1;
    if (fill_archive_header(file, archive_open) ||
        fill_archive(file, archive_open) ||
        padding(file, archive_open) ||
        fill_archive_end(archive_open))
        return 1;
    return 0;
}