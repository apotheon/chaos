#include "compiler.h"

struct stat dir_stat = {0};

unsigned short indent_length = 4;

void compile(char *module, enum Phase phase_arg, char *bin_file) {
    printf("Starting compiling...\n");
    ASTNode* ast_node = ast_root_node;

    if (stat(__KAOS_BUILD_DIRECTORY__, &dir_stat) == -1) {
        printf("Creating build directory...\n");
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
            _mkdir(__KAOS_BUILD_DIRECTORY__);
        #else
            mkdir(__KAOS_BUILD_DIRECTORY__, 0700);
        #endif
    }

    char c_file_path[PATH_MAX];
    sprintf(c_file_path, "%s%smain.c", __KAOS_BUILD_DIRECTORY__, __KAOS_PATH_SEPARATOR__);

    printf("Compiling Chaos code into %s\n", c_file_path);

    FILE *c_fp = fopen(c_file_path, "w");
    if (c_fp == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    const char *c_file_base =
        "#include <stdio.h>\n\n"
        "int main() {\n";

    unsigned short indent = indent_length;

    fprintf(c_fp, "%s", c_file_base);

    transpile_node(ast_node, module, c_fp, indent);

    fprintf(c_fp, "}\n");

    fclose(c_fp);

    printf("Compiling the C code into machine code...\n");

    char bin_file_path[PATH_MAX];
    if (bin_file != NULL) {
        sprintf(bin_file_path, "%s%s%s", __KAOS_BUILD_DIRECTORY__, __KAOS_PATH_SEPARATOR__, bin_file);
    } else {
        sprintf(bin_file_path, "%s%smain", __KAOS_BUILD_DIRECTORY__, __KAOS_PATH_SEPARATOR__);
    }

    char c_compiler_path[PATH_MAX];
    #if defined(__clang__)
        sprintf(c_compiler_path, "clang");
    #elif defined(__GNUC__) || defined(__GNUG__)
        sprintf(c_compiler_path, "gcc");
    #endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    STARTUPINFO info={sizeof(info)};
    PROCESS_INFORMATION processInfo;
    DWORD status;

    char cmd[PATH_MAX];
    sprintf(cmd, "/c %s -o %s %s", c_compiler_path, bin_file_path, c_file_path);
    if (CreateProcess("C:\\WINDOWS\\system32\\cmd.exe", cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo)) {

        WaitForSingleObject(processInfo.hProcess, INFINITE);

        GetExitCodeProcess(processInfo.hProcess, &status);

        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);

        if (status != 0) {
            printf("Compilation of %s is failed!\n", c_file_path);
            exit(status);
        }
    } else {
        printf("CreateProcess() failed!");
    }
#else
    pid_t pid;

    if ((pid = fork()) == -1)
        perror("fork error");
    else if (pid == 0)
        execlp(c_compiler_path, c_compiler_path, "-o", bin_file_path, c_file_path, NULL);

    int status;
    pid_t wait_result;

    while ((wait_result = wait(&status)) != -1)
    {
        // printf("Process %lu returned result: %d\n", (unsigned long) wait_result, status);
        if (status != 0) {
            printf("Compilation of %s is failed!\n", c_file_path);
            exit(status);
        }
    }
#endif

    printf("Finished compiling.\n\n");

    char bin_file_path_final[PATH_MAX + 4];
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    if (!string_ends_with(bin_file_path, __KAOS_WINDOWS_EXE_EXT__)) {
        sprintf(bin_file_path_final, "%s%s", bin_file_path, __KAOS_WINDOWS_EXE_EXT__);
    } else {
        sprintf(bin_file_path_final, "%s", bin_file_path);
    }
#else
    sprintf(bin_file_path_final, "%s", bin_file_path);
#endif

    printf("Binary is ready on: %s\n", bin_file_path_final);
}

ASTNode* transpile_node(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent) {
    if (ast_node == NULL) {
        return ast_node;
    }

    if (ast_node->depend != NULL) {
        eval_node(ast_node->depend, module);
    }

    if (ast_node->right != NULL) {
        eval_node(ast_node->right, module);
    }

    if (ast_node->left != NULL) {
        eval_node(ast_node->left, module);
    }

    fprintf(c_fp, "%*c", indent, ' ');
    switch (ast_node->node_type)
    {
        case AST_PRINT_STRING:
            fprintf(c_fp, "printf(\"%s\\n\");", ast_node->value.s);
            break;
        default:
            break;
    }
    fprintf(c_fp, "\n");

    return ast_node;
}
