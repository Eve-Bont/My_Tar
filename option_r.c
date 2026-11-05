int option_r(char* archive, char** files, int nb_files) {
    int archive_open = open(archive, O_RDWR);
    struct stat info;
    if (archive_open == -1) {
        error_opening(archive);
        return 1;
    }
    if (validate_archive_format(archive_open, &info) || 
        validate_archive(archive_open, info) ||
        lseek(archive_open, -1024, SEEK_END) == -1) {
        close(archive_open);
        return text_error("my_tar: invalid archive\n");
    }
    for (int i = 0; i < nb_files; i++) {
        if( fill_archive_header(files[i], archive_open) || 
            fill_archive(files[i], archive_open) || 
            padding(files[i], archive_open)) {
            close(archive_open);
            return 1;
        }
    }
    if(fill_archive_end(archive_open)) {
        close(archive_open);
        return 1;
    }
    close(archive_open);
    return 0;
}

/*
FUNCTIONS
*/

int verify_bloc_0(char* bloc) {
    for (int i = 0; i < 512; i++) {
        if (bloc[i] != '\0') {
            return 0;
        }
    }
    return 1;
}

int verify_archive_end(int archive_open, off_t position, struct stat info) {
    char second_block[512];
    if (position > info.st_size || info.st_size - position != 512) {
        return 1;
    }
    if (read_all(archive_open, second_block, 512)) {
        return 1;
    }
    if (!verify_bloc_0(second_block)) {
        return 1;
    }
    return 0;
}

int verify_archive_file(int archive_open, char* header, off_t* position, struct stat info) {
    unsigned long size_file = 0;
    if (!verify_checksum(header) || !get_file_size(header, &size_file)) {
        return 1;
    }

    unsigned long padding = (512 - (size_file % 512)) % 512;
    unsigned long skip = size_file + padding;
    if (*position > info.st_size || skip > (unsigned long)(info.st_size - *position)) {
        return 1;
    }
    if (lseek(archive_open, (off_t)skip, SEEK_CUR) == -1) {
        return 1;
    }
    *position += (off_t)skip;
    return 0;
}

int verify_checksum(char* header) {
    unsigned long restored = 0;
    unsigned long calculated = 0;

    for (int i = 148; i < 154; i++) {
        if (header[i] < '0' || header[i] > '7') {
            return 0;
        }
        restored = restored * 8 + (header[i] - '0');
    }
    if (header[154] != '\0' || header[155] != ' ') {
        return 0;
    }
    for (int i = 0; i < 512; i++) {
        if (i >= 148 && i <= 155) {
            calculated += ' ';
        } else {
            calculated += (unsigned char)header[i];
        }
    }
    return restored == calculated;
}