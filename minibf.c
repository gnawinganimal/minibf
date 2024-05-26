#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/***********
 * STRINGS *
 ***********/

struct str {
    char*  ptr;
    size_t len;
    size_t cap;
};

struct str str_new() {
    struct str s;
    s.ptr = malloc(sizeof(char) * 4);
    s.len = 0;
    s.cap = 4;
    return s;
}

void       str_resize(struct str* s) {
    char* ptr = malloc(sizeof(char) * s->cap * 2);
    memcpy(ptr, s->ptr, s->cap);
    free(s->ptr);
    s->ptr = ptr;
    s->cap = s->cap * 2;
}

void       str_resize_n(struct str* s, size_t n) {
    char* ptr = malloc(sizeof(char) * s->cap * n);
    memcpy(ptr, s->ptr, s->cap);
    free(s->ptr);
    s->ptr = ptr;
    s->cap = s->cap * n;
}

char       str_push(struct str* s, char c) {
    if (s->len >= s->cap)
        str_resize(s);

    s->ptr[s->len] = c;
    s->len++;
}

size_t      str_cat(struct str* s, char* src, size_t n) {
    while (s->len + n - 1 >= s->cap)
        str_resize(s);

    memcpy(s->ptr + s->len, src, n);
    s->len = s->len + n;
}

char        str_get(struct str* s, size_t i) {
    return s->ptr[i];
}

void        str_clear(struct str* s) {
    if (s->ptr)
        free(s->ptr);

    s->ptr = NULL;
    s->len = 0;
    s->cap = 0;
}

int         str_read_from(struct str* s, int fd) {
    char       buf[4096];
    size_t     n;
    
    while (n = read(fd, buf, 4096)) {
        str_cat(s, buf, n);
    }
}

/********
* LISTS *
*********/

struct list {
    struct node* head;
};

struct node {
    size_t       item;
    struct node* next;
};

struct list list_new() {
    struct list list;
    list.head = NULL;
    return list;
}

void        list_push(struct list* list, size_t item) {
    struct node* node = malloc(sizeof(struct node));
    node->item = item;
    node->next = list->head;
    list->head = node;
}

size_t      list_get(struct list* list) {
    return list->head->item;
}

size_t      list_pop(struct list* list) {
    struct node* node = list->head;
    size_t       item = node->item;
    list->head        = node->next;
    free(node);
    return item;
}

/*********
 * TAPES *
 *********/

 struct tape {
    char*  ptr;
    size_t len;
    size_t ofs;

    char   flags;
};

struct tape tape_new(size_t len) {
    struct tape tape;
    tape.ptr   = calloc(sizeof(char), len);
    tape.len   = len;
    tape.ofs   = 0;
    tape.flags = 0;
    return tape;
}

char        tape_get_cur(struct tape* tape) {
    return tape->ptr[tape->ofs];
}

char*       tape_ref_cur(struct tape* tape) {
    return  tape->ptr + tape->ofs;
}

size_t      tape_get_ofs(struct tape* tape) {
    return  tape->ofs;
}

size_t*     tape_ref_ofs(struct tape* tape) {
    return &tape->ofs;
}

/**********
 * SEARCH *
 **********/ 

size_t
search_from(struct str* s, size_t i, char c) {
    while (str_get(s, i) != c) {
        i++;
    }

    return i;
}

/********
 * EXEC *
 ********/

int
exec_src(struct str* src, struct tape* tape, struct list* stack) {
    char   c;        // current 'char' instruction to execute
    size_t i    = 0; // current instruction index

    while (i < src->len) {
        c = str_get(src, i);

        switch (c) {
            case '+':
                (*tape_ref_cur(tape))++;
                break;
            case '-':
                (*tape_ref_cur(tape))--;
                break;
            case '>':
                (*tape_ref_ofs(tape))++;
                break;
            case '<':
                (*tape_ref_ofs(tape))--;
                break;
            case '.':
                putchar(tape_get_cur(tape));
                break;
            case ',':
                (*tape_ref_cur(tape)) = (char) getchar();
                break;
            case '[':
                list_push(stack, i + 1);
                if (!tape_get_cur(tape)) {
                    i = search_from(src, i + 1, ']');
                    continue;
                } else {
                    break;
                }
            case ']':
                if (tape_get_cur(tape)) {
                    i = list_get(stack);
                    continue;
                } else {
                    list_pop(stack);
                    break;  
                }
        }

        i++;
    }
    
    return 0;
}

int
main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "minibf: no file path specified\n");
        return 1;
    }

    char* path = argv[1];
    int   file = open(path, O_RDONLY);
    if (file < 0) {
        fprintf(stderr, "minibf: could not open file '%s'\n", path);
        return 1;
    }

    struct str src = str_new();
    if (str_read_from(&src, file) < 0) {
        fprintf(stderr, "minibf: could not read file '%s'\n", path);
    }
    
    struct tape tape = tape_new(1000);
    struct list stack = list_new();
    exec_src(&src, &tape, &stack);

    return 0;
}
