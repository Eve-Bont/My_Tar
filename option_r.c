int option_r() {
    int archive_open = open(archive, O_RDONLY);
    if (archive_open == -1) {
        error_opening(archive);
        return 1;
    }
    if(fstat(archive_open, &info) == -1) {
        error_stat(archive);
        close(archive_open);
        return 1;
    }
    if(!S_ISREG(info.st_mode) || info.st_size % 512 != 0 || info.st_size < 1024) {
        close(archive_open);
        return text_error("my_tar: invalid archive\n");
    }

    for (off_t index_archive = 0; index_archive < info.st_size; index_archive += 512) {
        char header[512];
        if(read_all(archive_open, header, 512)) {
            close(archive_open);
            return text_error("my_tar: invalid archive\n");
        }
        if (verify_header_0(header) != 512) {
            if(verify_checksum(header)) {
                close(archive_open);
                return text_error("my_tar: invalid archive\n");
            }
        } else {
            if(verify_end_block(archive_open, info.st_size, index_archive + 512)) {
                close(archive_open);
                return text_error("my_tar: invalid archive\n");
            }
        }
    }
    close(archive_open);
    return 0;
}

/*
FUNCTIONS
*/

int read_all(int descriptor, char* buffer, size_t size) {
    size_t total = 0;
    while(total < size) {
        ssize_t reading = read(descriptor, buffer + total, size - total);
        if (reading <= 0) {
            return 1;
        }
        total += reading;
    }
    return 0;
}

/*
HEADER
*/

int verify_end_block(int archive_open, off_t archive_size, off_t current_position) {
    char end_block[512];
    if (current_position + 1024 > archive_size) {
        return 1;
    }
    if (read_all(archive_open, end_block, 512)) {
        return 1;
    }
    for (int i = 0; i < 512; i++) {
        if (end_block[i] != '\0') {
            return 1;
        }
    }
    if (current_position + 1024 != archive_size) {
        return 1;
    }
    return 0;
}

int verify_header_0(char* header) {
    int count_0 = 0;
    for (int index_header = 0; index_header < 512; index_header++) {
        if(header[index_header] == '\0') {
            count_0++;
        }
    }
    return count_0;
}

int verify_checksum(char* header) {
    unsigned long restored_checksum = 0;
    for (int i = 148; i < 154; i++) {
        if (header[i] < '0' || header[i] > '7') {
            return 1;
        }
        restored_checksum = restored_checksum * 8 + (header[i] - '0');
    }
    unsigned long calculated_checksum = 0;
    for (int i = 0; i < 512; i++) {
        if (i >= 148 && i <= 155) {
            calculated_checksum += ' ';
        } else {
            calculated_checksum += (unsigned char)header[i];
        }
    }
    return restored_checksum != calculated_checksum;
}