int option_t(char* archive) {
    int archive_open = open(archive, O_RDONLY);
    struct stat info;
    if(archive_open == -1) {
        error_opening(archive);
        return 1;
    }
    if( validate_archive_format(archive_open, &info) || 
        validate_archive(archive_open, info) || 
        lseek(archive_open, 0, SEEK_SET) == -1 || 
        list_archive(archive_open)) {
        close(archive_open);
        return text_error("my_tar: invalid archive\n");
    }
    close(archive_open);
    return 0;
}

/*
FUNCTIONS
*/

int list_archive(int archive_open) {
    while (1) {
        char header[512];
        if(read_all(archive_open, header, 512)) {
            return 1;
        }
        if(verify_bloc_0(header)) {
            return 0;
        }
        if (display_name(header) || jump_file(archive_open, header)) {
            return 1;
        }
    }
}

int display_name(char* header) {
    size_t index_end_name = 0;
    while(header[index_end_name] != '\0' && index_end_name < 100) {
        index_end_name++;
    }
    if (write_all(1, header, index_end_name) || write_all(1, '\n', 1)) {
        return 1;
    }
    return 0;
}

int jump_file(int archive_open, char* header) {
    unsigned long size_file = 0;
    if(!get_file_info(header, &size_file, 124, 135)) {
        return 1;
    }
    unsigned long padding = (512 - (size_file % 512)) % 512;
    if(lseek(archive_open, padding + size_file, SEEK_CUR) == -1) {
        return 1;
    }
    return 0;
}