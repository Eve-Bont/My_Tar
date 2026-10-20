int option_r() {
    int archive_open = open(archive, O_RDONLY);
    if (archive_open == -1) {
        erreur_opening(archive);
        return 1;
    }
    if(fstat(archive_open, &info) == -1 || S_ISREGt(info.st_mode)) {
        erreur_stat(archive);
        close(archive_open);
        return 1;
    }
    char header[512];
    ssize_t read_header = read(archive_open, header, 512);
    if(read_header != 512) {

    }
}