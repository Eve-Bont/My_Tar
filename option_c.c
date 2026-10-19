int option_c() {
    int archive_open = open(argv[archive_index], O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (archive_open == -1) {
        erreur_opening(argv[archive_index]);
        free(files);
        return 1;
    }

    for(int i = 0; i < nb_files; i++) {
        struct stat info;
        if(lstat(files[i], &info) == -1) {
            erreur_stat(files[i]);
            return close_and_free(archive_open, files);
        }
        int file_open = open(files[i], O_RDONLY);
        if (file_open == -1) {
            erreur_opening(files[i]);
            return close_and_free(archive_open, files);
        }

        char header[512] = {'\0'};
        if(fill_header(header, info, files[i])) {
            return close_and_free(archive_open, files);
        }
        if (write(archive_open, header, 512) != 512 ) {
            text_error("my_tar: write error\n");
            return close_and_free(archive_open, files);
        }



        char buffer[4096];
        ssize_t size_file = read(file_open, buffer, 4096);
        if(size_file == -1) {
            text_error(/*error*/);
            return double_close_and_free(archive_open, file_open, files);
        }
        while(size_file > 0) {
            int verif += write(archive_open, buffer, size_file);
            if(verif == -1) {
                text_error("my_tar: write error\n");
                return close_and_free(archive_open, files);
            }
            while(verif != size_file) {
                verif = write(archive_open, buffer, size_file - verif);
            }
            size_file = read(file_open, buffer, 4096);
        }
        
        


        close(file_open);
    }
    
    close(archive_open);
    return 0;
}

int verificaton_write(int sortie, int entree, size) {
    int verif += write(archive_open, buffer, size_file);
    if(verif == -1) {
        text_error("my_tar: write error\n");
        return close_and_free(archive_open, files);
    }
    while(verif != size_file) {
        verif = write(archive_open, buffer, size_file - verif);
    }
}

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
int write_octal_field(unsigned long info, int size, int index_min, int index_max, char* header, char* error) {
    char* conversion = malloc(sizeof(char) * size);
    if(conversion == NULL) {
        return text_error("my_tar: Memory exhausted\n");
    }
    for(int fill = 0; fill < size; fill++) {
        conversion[fill] = '0';
    }

    int index_info = 0;
    while(info != 0){
        if (index_info >= size) {
            free(conversion);
            return text_error(error);
        }
        conversion[index_info] = (info % 8) + '0';
        info = info / 8;
        index_info++;
    }
    int index_header = index_min;
    while(index_header < index_max - index_info) {
        header[index_header] = '0';
        index_header++;
    }
    while(index_info != 0) {
        index_info--;
        header[index_header] = conversion[index_info];
        index_header++;
    }
    header[index_max] = '\0';
    free(conversion);
    return 0;
}

int write_u_g_name(int index_header, char* name, char* header, char* error) {
    for (int i = 0; name[i] != '\0'; i++) {
        if(i > 31) {
            return text_error(error);
        }
        header[index_header] = name[i];
        index_header++;
    }
    return 0;
}
int close_and_free(int archive_open, char** files) {
    close(archive_open);
    free(files);
    return 1;
}
int double_close_and_free(int archive_open, int file_open, char** files) {
    close(archive_open);
    close(file_open);
    free(files);
    return 1;
}


int name(char* file, char* header) {
    int size = count_string(file);
    if(size > 100) {
        index_split = -1;
        for(int i = 0; i < size; i++) {
            if (file[i] == '/' && i <= 155 && size - i - 1 <= 100) {
                index_split = i;
                break;
            }
        }
        if (index_split == -1) {
            return text_error("erreur de longueur du nom\n");
        }
        prefix(file, index_split, header);
        int index_header = 0;
        for (int j = index_split + 1; j < size; j++) {
            header[index_header] = file[j];
            index_header++;
        }
        return 0;
    }
    
    for (int j = 0; j < size; j++) {
        header[j] = file[j];
    }
    return 0;
}
int mode(unsigned long mode, char* header) {
    return write_octal_field(mode, 3, 100, 107, header, "");
}
int uid(unsigned long uid, char* header) {
    return write_octal_field(uid, 7, 108, 115, header, "my_tar: Numeric user ID too large\n");
}
int gid(unsigned long gid, char* header) {
    return write_octal_field(gid, 7, 116, 123, header, "my_tar: Numeric group ID too large\n");
}
int size(unsigned long size, char* header) {
    return write_octal_field(size, 11, 124, 135, header, "my_tar: File size too large\n");
}
int mtime(unsigned long mtime, char* header) {
    return write_octal_field(mtime, 11, 136, 147, header, "my_tar: Modification time too large\n");
}
int chksum(char* header) {
    int index_header = 148;
    for (int i = 0; i < 8; i++) {
        header[index_header] = ' ';
        index_header++;
    }
    int chksum = 0;
    for (int i = 0; i != 512; i++) {
        chksum += (unsigned char)header[i];
    }
    return write_octal_field(chksum, 6, 148, 154, header, "my_tar: Checksum too large\n");
}
int typeflag(mode_t type, char* header) {
    if(S_ISREG(type)) {
        header[156] = '0';
        return 0;
    }
    if(S_ISDIR(type)) {
        header[156] = '5';
        return 0;
    }
    if(S_ISLNK(type)) {
        header[156] = '2';
        return 0;
    }
    return 1;
}
int linkname(char* file, char* header) {
    int new_index = readlink(file, &header[157], 100);
    if (new_index == -1) {
        return text_error("my_tar: Cannot read symbolic link\n");
    }
    if (new_index == 100) {
        return text_error("my_tar: Symbolic link target too long\n");
    }
    header[157 + new_index] = '\0';
    return 0;
}
int magic(char* header) {
    char* magic = "ustar";
    for(int i = 0; magic[i] != '\0'; i++) {
        header[257+i] = magic[i];
    }
    header[262] = '\0';
    return 0;
}
int version(char* header) {
    header[263] = '0';
    header[264] = '0';
    return 0;
}
int uname(uid_t info, char* header) {
    struct passwd *user = getpwuid(info);
    if(user == NULL) {
        return text_error("my_tar: Cannot find user for UID\n");
    }
    return write_u_g_name(265, user->pw_name, header, "my_tar: User name too long\n");
}
int gname(gid_t info, char* header) {
    struct group *group = getgrgid(info);
    if(group == NULL) {
        return text_error("my_tar: Cannot find group for GID\n");
    }
    return write_u_g_name(297, group->gr_name, header, "my_tar: Group name too long\n");
}
int devmajor(unsigned long devmajor, char* header) {
    return write_octal_field(devmajor, 7, 329, 336, header, "my_tar: Device major number too large\n");
}
int devminor(unsigned long devminor, char* header) {
    return write_octal_field(devminor, 7, 337, 344, header, "my_tar: Device major number too large\n");
}
void prefix(char* file, int index_split, char* header) {
    int index_header = 345;
    for(int i = 0; i < index_split; i++) {
        header[index_header] = file[i];
        index_header++;
    }
}