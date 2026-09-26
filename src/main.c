#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "check.h"
#include "codegen.h"
#include "diag.h"
#include "interp.h"
#include "parser.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: cannot open %s\n", path);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "error: cannot measure %s\n", path);
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0) {
        fprintf(stderr, "error: cannot measure %s\n", path);
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buffer = malloc((size_t)size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "error: out of memory reading %s\n", path);
        fclose(f);
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)size, f);
    fclose(f);
    if (read != (size_t)size) {
        fprintf(stderr, "error: short read on %s\n", path);
        free(buffer);
        return NULL;
    }

    buffer[read] = '\0';
    return buffer;
}

static void print_tree(const Node *node) {
    switch (node->type) {
    case NODE_NUMBER:
        printf("%ld", node->as.number);
        break;
    case NODE_BOOL:
        printf("%s", node->as.boolean ? "true" : "false");
        break;
    case NODE_NAME:
        printf("%.*s", node->as.name.length, node->as.name.start);
        break;
    case NODE_UNARY:
        printf("(%s ", op_symbol(node->as.unary.op));
        print_tree(node->as.unary.operand);
        printf(")");
        break;
    case NODE_BINARY:
        printf("(%s ", op_symbol(node->as.binary.op));
        print_tree(node->as.binary.left);
        printf(" ");
        print_tree(node->as.binary.right);
        printf(")");
        break;
    case NODE_CALL:
        printf("(%.*s", node->as.call.callee.length, node->as.call.callee.start);
        for (int i = 0; i < node->as.call.arg_count; i++) {
            printf(" ");
            print_tree(node->as.call.args[i]);
        }
        printf(")");
        break;
    }
}

static int evaluate(const char *src) {
    diag_set_path("<command line>");

    Parser p;
    parser_init(&p, src);
    Node *tree = parser_parse(&p);
    if (tree == NULL) {
        return 1;
    }
    if (!check_expression(src, tree)) {
        ast_free(tree);
        return 1;
    }

    print_tree(tree);
    long value = interp_eval(src, tree);
    if (tree->ty == TYPE_BOOL) {
        printf("\n= %s\n", value ? "true" : "false");
    } else {
        printf("\n= %ld\n", value);
    }

    ast_free(tree);
    return 0;
}

static bool front_end(const char *path, char **src, Program *prog) {
    *src = read_file(path);
    if (*src == NULL) {
        return false;
    }
    diag_set_path(path);

    Parser p;
    parser_init(&p, *src);
    *prog = parser_parse_program(&p);
    if (!p.had_error && check_program(*src, prog)) {
        return true;
    }

    program_free(prog);
    free(*src);
    return false;
}

static char *default_output(const char *path) {
    size_t length = strlen(path);
    char *out = xrealloc(NULL, length + 5);
    memcpy(out, path, length + 1);
    if (length > 3 && strcmp(path + length - 3, ".iz") == 0) {
        out[length - 3] = '\0';
    } else {
        strcat(out, ".out");
    }
    return out;
}

static bool build_binary(const char *src, const Program *prog, const char *path, const char *output) {
    const char *cc = getenv("CC");
    if (cc == NULL || cc[0] == '\0') {
        cc = "cc";
    }

    int fds[2];
    if (pipe(fds) != 0) {
        fprintf(stderr, "error: cannot create a pipe to %s: %s\n", cc, strerror(errno));
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "error: cannot start %s: %s\n", cc, strerror(errno));
        close(fds[0]);
        close(fds[1]);
        return false;
    }

    if (pid == 0) {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        int null = open("/dev/null", O_WRONLY);
        if (null >= 0) {
            dup2(null, STDOUT_FILENO);
            dup2(null, STDERR_FILENO);
            close(null);
        }
        execlp(cc, cc, "-std=c11", "-O2", "-w", "-x", "c", "-", "-o", output, (char *)NULL);
        _exit(127);
    }

    close(fds[0]);
    signal(SIGPIPE, SIG_IGN);
    FILE *to_cc = fdopen(fds[1], "w");
    if (to_cc == NULL) {
        close(fds[1]);
    } else {
        codegen_emit(to_cc, src, prog);
        fclose(to_cc);
    }

    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            fprintf(stderr, "error: lost track of %s: %s\n", cc, strerror(errno));
            return false;
        }
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && to_cc != NULL) {
        return true;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
        fprintf(stderr, "error: cannot run the C compiler '%s', set CC to one that exists\n", cc);
    } else {
        fprintf(stderr, "error: the C compiler '%s' rejected the generated code\n"
                        "       run 'izvor emit %s' to see it\n", cc, path);
    }
    return false;
}

static void usage(void) {
    fprintf(stderr,
            "usage: izvor run <file.iz>\n"
            "       izvor build <file.iz> [-o <output>]\n"
            "       izvor emit <file.iz>\n"
            "       izvor -e \"<expression>\"\n");
}

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "-e") == 0) {
        return evaluate(argv[2]);
    }

    const char *command = "run";
    const char *path = NULL;
    const char *output = NULL;

    if (argc == 2 && argv[1][0] != '-') {
        path = argv[1];
    } else if (argc == 3 && (strcmp(argv[1], "run") == 0 || strcmp(argv[1], "emit") == 0 ||
                             strcmp(argv[1], "build") == 0)) {
        command = argv[1];
        path = argv[2];
    } else if (argc == 5 && strcmp(argv[1], "build") == 0 && strcmp(argv[3], "-o") == 0) {
        command = argv[1];
        path = argv[2];
        output = argv[4];
    } else {
        usage();
        return 1;
    }

    char *src;
    Program prog;
    if (!front_end(path, &src, &prog)) {
        return 1;
    }

    int status = 0;
    if (strcmp(command, "run") == 0) {
        interp_run(src, &prog);
    } else if (strcmp(command, "emit") == 0) {
        codegen_emit(stdout, src, &prog);
    } else {
        char *owned = output == NULL ? default_output(path) : NULL;
        if (!build_binary(src, &prog, path, output != NULL ? output : owned)) {
            status = 1;
        }
        free(owned);
    }

    program_free(&prog);
    free(src);
    return status;
}
