/*
**
*/
#include <stdlib.h>
#include <stdio.h>

/*
**
*/
#define OPT_C (1<<0)
#define OPT_A (1<<1)
#define OPT_B (1<<2)
#define OPT_Z (1<<3)

typedef struct s_opt{
    int opt;
} t_opt;

typedef struct s_opt_list {
    char c;
    int val;
} t_opt_list;

/*
**
*/
void my_memset(char *buf, char c, int size) {
    for (int index = 0 ; index < size ; index += 1){
        buf[index] = c;
    }
}

void my_bzero(char *buf, int size){
    my_memset(buf, 0, size);
}

char *init(int size){
    char *buf = malloc(size);
    if (buf == NULL)
        exit(1);

    my_bzero(buf, size);
    return  buf;
}

int is_option(char c){
    t_opt_list opt_list[]= {
        {'c', OPT_C},
        {'a', OPT_A},
        {'b', OPT_B},
        {'z', OPT_Z},
        {0, 0}
    };
    
    for (int index = 0 ; opt_list[index].c != 0 ; index += 1) {
        if (c == opt_list[index].c) {
            return opt_list[index].val;
        }
    }

    return 0;
}

int get_opt(char *options) {
    int opt = 0;

    while (*options != 0) {
        opt |= is_option(*options);
        *options += 1;
    }
    return opt;
}

t_opt *init_opt(int ac, char **av) {
    t_opt *opt = (t_opt *)init(sizeof(opt));

    for (int index = 1 ; index < ac ; index += 1) {
        if (av[index][0] == '-') {
            opt->opt |= get_opt(av[index]);
        }
    }
    return opt;
}

int main(int ac, char **av){
    t_opt *opt = init_opt(ac, av);

    if (opt OPT_C) {
        printf("option C"); 
    }
}