option_t(char* archive) {
    int archive_open = open(archive, O_RDONLY);
    if(archive_open == -1) {
        error_opening(archive);
        return 1;
    }
    
}