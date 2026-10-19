int erreur_option(void) {
    write(2, "my_tar: invalid options\n", count_string("my_tar: invalid options\n"));
    return 1;
}

void erreur_stat(char* file) {
    write(2, "my_tar: ", count_string("my_tar: "));
    write(2, file, count_string(file));
    write(2, ": Cannot stat: No such file or directory\n", count_string(": Cannot stat: No such file or directory\n"));
}

void erreur_opening(char* file) {
    write(2, "my_tar: Cannot open ", count_string("my_tar: Cannot open "));
    write(2, file, count_string(file));
    write(2, "\n", 1);
}

int text_error(char* message) {
    write(2, message, count_string(message));
    return 1;
}