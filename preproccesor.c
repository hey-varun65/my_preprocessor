#include "preprocessor.h"

Macro macros[MAX_MACROS];
int macro_count = 0;

char *remove_comments(const char *source)
{
        int i = 0, j = 0;
        int state = 0;

        char *result = malloc(strlen(source) + 1);

        if (result == NULL)
                return NULL;

        while (source[i] != '\0')
        {
                if (state == 0)
                {
                        if (source[i] == '"')
                        {
                                result[j++] = source[i++];
                                state = 3;
                        }
                        else if (source[i] == '\'')
                        {
                                result[j++] = source[i++];
                                state = 4;
                        }
                        else if (source[i] == '/' && source[i + 1] == '/')
                        {
                                i += 2;
                                state = 1;
                        }
                        else if (source[i] == '/' && source[i + 1] == '*')
                        {
                                i += 2;
                                state = 2;
                        }
                        else
                        {
                                result[j++] = source[i++];
                        }
                }
                else if (state == 1)
                {
                        if (source[i] == '\n')
                        {
                                result[j++] = '\n';
                                i++;
                                state = 0;
                        }
                        else
                                i++;
                }
                else if (state == 2)
                {
                        if (source[i] == '*' && source[i + 1] == '/')
                        {
                                i += 2;
                                state = 0;
                        }
                        else
                        {
                                if (source[i] == '\n')
                                        result[j++] = '\n';

                                i++;
                        }
                }
                else if (state == 3)
                {
                        result[j++] = source[i];

                        if (source[i] == '\\' && source[i + 1] != '\0')
                                result[j++] = source[++i];
                        else if (source[i] == '"')
                                state = 0;

                        i++;
                }
                else if (state == 4)
                {
                        result[j++] = source[i];

                        if (source[i] == '\\' && source[i + 1] != '\0')
                                result[j++] = source[++i];
                        else if (source[i] == '\'')
                                state = 0;

                        i++;
                }
        }

        result[j] = '\0';

        return result;
}
void define_macro(char *line)
{
        char name[MAX_NAME] = "";
        char value[MAX_VALUE] = "";
        char params[10][MAX_NAME];
        int count = 0;
        int function = 0;

        char *p = line + 7;

        while (isspace((unsigned char)*p))
                p++;

        int i = 0;

        while (isalnum((unsigned char)*p) || *p == '_')
                name[i++] = *p++;

        name[i] = '\0';

        if (name[0] == '\0')
                return;

        /* Function-like macro */

        if (*p == '(')
        {
                function = 1;
                p++;

                while (*p && *p != ')')
                {
                        while (isspace((unsigned char)*p))
                                p++;

                        i = 0;

                        while (isalnum((unsigned char)*p) || *p == '_')
                                params[count][i++] = *p++;

                        params[count][i] = '\0';

                        if (i > 0)
                                count++;

                        while (isspace((unsigned char)*p))
                                p++;

                        if (*p == ',')
                                p++;
                        else
                                break;
                }

                if (*p == ')')
                        p++;
        }

        while (isspace((unsigned char)*p))
                p++;

        strncpy(value, p, MAX_VALUE - 1);
        value[strcspn(value, "\n")] = '\0';

        /* Find existing macro */

        int index = -1;

        for (i = 0; i < macro_count; i++)
        {
                if (strcmp(macros[i].name, name) == 0)
                {
                        index = i;
                        break;
                }
        }

        if (index == -1)
        {
                if (macro_count >= MAX_MACROS)
                        return;

                index = macro_count++;
        }

        strcpy(macros[index].name, name);
        strcpy(macros[index].value, value);

        macros[index].function_like = function;
        macros[index].param_count = count;

        for (i = 0; i < count; i++)
                strcpy(macros[index].params[i], params[i]);
}

/* Replace parameter names in function macro body */

static void replace_parameter(
                const char *body,
                char params[][MAX_NAME],
                char args[][MAX_VALUE],
                int count,
                char *result)
{
        int i = 0, j = 0;

        while (body[i] && j < MAX_LINE - 1)
        {
                if (isalpha((unsigned char)body[i]) || body[i] == '_')
                {
                        char word[MAX_NAME];
                        int k = 0;

                        while ((isalnum((unsigned char)body[i]) ||
                                                body[i] == '_') && k < MAX_NAME - 1)
                                word[k++] = body[i++];

                        word[k] = '\0';

                        int found = 0;

                        for (int n = 0; n < count; n++)
                        {
                                if (strcmp(word, params[n]) == 0)
                                {
                                        int len = strlen(args[n]);

                                        if (j + len < MAX_LINE - 1)
                                        {
                                                strcpy(result + j, args[n]);
                                                j += len;
                                        }

                                        found = 1;
                                        break;
                                }
                        }

                        if (!found)
                        {
                                strcpy(result + j, word);
                                j += strlen(word);
                        }
                }
                else
                {
                        result[j++] = body[i++];
                }
        }

        result[j] = '\0';
}

void expand_macros(char *line, char *output)
{
        char temp[MAX_LINE];
        char result[MAX_LINE];

        strncpy(temp, line, MAX_LINE - 1);
        temp[MAX_LINE - 1] = '\0';

        /* Multiple passes support simple nested macros */

        for (int pass = 0; pass < 10; pass++)
        {
                int i = 0, j = 0;
                int changed = 0;

                while (temp[i] && j < MAX_LINE - 1)
                {
                        if (isalpha((unsigned char)temp[i]) || temp[i] == '_')
                        {
                                char word[MAX_NAME];
                                int k = 0;

                                while ((isalnum((unsigned char)temp[i]) ||
                                                        temp[i] == '_') && k < MAX_NAME - 1)
                                        word[k++] = temp[i++];

                                word[k] = '\0';

                                int found = -1;

                                for (int n = 0; n < macro_count; n++)
                                {
                                        if (strcmp(word, macros[n].name) == 0)
                                        {
                                                found = n;
                                                break;
                                        }
                                }

                                if (found == -1)
                                {
                                        int len = strlen(word);
                                        strcpy(result + j, word);
                                        j += len;
                                }
                                else if (!macros[found].function_like)
                                {
                                        int len = strlen(macros[found].value);

                                        if (j + len >= MAX_LINE - 1)
                                                break;

                                        strcpy(result + j, macros[found].value);
                                        j += len;
                                        changed = 1;
                                }
                                else
                                {
                                        int pos = i;

                                        while (isspace((unsigned char)temp[pos]))
                                                pos++;

                                        if (temp[pos] != '(')
                                        {
                                                strcpy(result + j, word);
                                                j += strlen(word);
                                                continue;
                                        }

                                        pos++;

                                        char args[10][MAX_VALUE] = {{0}};
                                        int ac = 0, ai = 0;
                                        int paren = 1;

                                        while (temp[pos] && paren > 0)
                                        {
                                                char c = temp[pos++];

                                                if (c == '(')
                                                        paren++;
                                                else if (c == ')')
                                                {
                                                        paren--;

                                                        if (paren == 0)
                                                                break;
                                                }

                                                if (c == ',' && paren == 1)
                                                {
                                                        args[ac][ai] = '\0';
                                                        ac++;
                                                        ai = 0;
                                                }
                                                else if (ac < 10 && ai < MAX_VALUE - 1)
                                                {
                                                        args[ac][ai++] = c;
                                                }
                                        }

                                        if (ac < 10)
                                        {
                                                args[ac][ai] = '\0';
                                                ac++;
                                        }

                                        if (paren != 0 ||
                                                        ac != macros[found].param_count)
                                        {
                                                int len = strlen(word);
                                                strcpy(result + j, word);
                                                j += len;
                                                continue;
                                        }

                                        char replaced[MAX_LINE] = "";

                                        replace_parameter(
                                                        macros[found].value,
                                                        macros[found].params,
                                                        args,
                                                        ac,
                                                        replaced
                                                        );

                                        int len = strlen(replaced);

                                        if (j + len >= MAX_LINE - 1)
                                                break;

                                        strcpy(result + j, replaced);
                                        j += len;

                                        i = pos;
                                        changed = 1;
                                }
                        }
                        else
                        {
                                result[j++] = temp[i++];
                        }
                }

                result[j] = '\0';

                strcpy(temp, result);

                if (!changed)
                        break;
        }

        strcpy(output, temp);
}

int process_file(const char *filename, FILE *out, int depth)
{
        if (depth > 20)
        {
                printf("Maximum include depth exceeded\n");
                return 0;
        }

        FILE *fp = fopen(filename, "r");

        if (fp == NULL)
        {
                printf("Cannot open file: %s\n", filename);
                return 0;
        }

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        rewind(fp);

        if (size < 0)
        {
                fclose(fp);
                return 0;
        }

        char *source = malloc(size + 1);

        if (source == NULL)
        {
                fclose(fp);
                return 0;
        }

        size_t bytes = fread(source, 1, size, fp);
        source[bytes] = '\0';

        fclose(fp);

        char *clean = remove_comments(source);

        free(source);

        if (clean == NULL)
                return 0;

        char *p = clean;
        char line[MAX_LINE];

        while (*p)
        {
                int i = 0;

                while (*p && *p != '\n' && i < MAX_LINE - 2)
                        line[i++] = *p++;

                if (*p == '\n')
                        line[i++] = *p++;

                line[i] = '\0';

                char *start = line;

                while (isspace((unsigned char)*start))
                        start++;

                /* Handle #include */
                if (strncmp(start, "#include", 8) == 0)
                {
                        char include_file[MAX_VALUE];

                        char *a = strchr(start, '"');
                        char *b = NULL;

                        if (a)
                        {
                                b = strchr(a + 1, '"');

                                if (b)
                                {
                                        int len = b - a - 1;

                                        if (len > 0 && len < MAX_VALUE)
                                        {
                                                strncpy(include_file, a + 1, len);
                                                include_file[len] = '\0';

                                                if (!process_file(include_file, out, depth + 1))
                                                {
                                                        free(clean);
                                                        return 0;
                                                }
                                        }
                                }
                        }
                        else
                        {
                                /* Preserve system header includes */
                                fputs(line, out);
                        }
                }
                /* Handle #define */
                else if (strncmp(start, "#define", 7) == 0)
                {
                        define_macro(start);
                }

                /* Macro expansion */

                else
                {
                        char expanded[MAX_LINE];

                        expand_macros(line, expanded);

                        fputs(expanded, out);
                }
        }

        free(clean);

        return 1;
}



int main(int argc, char *argv[])
{
        if (argc != 2)
        {
                printf("Usage: ./mypreprocessor inputfile.c\n");
                return 1;
        }

        char output_file[300];

        strcpy(output_file, argv[1]);

        char *extension = strrchr(output_file, '.');

        if (extension && strcmp(extension, ".c") == 0)
                strcpy(extension, ".i");
        else
                strcat(output_file, ".i");

        FILE *out = fopen(output_file, "w");

        if (out == NULL)
        {
                printf("Output file creation failed\n");
                return 1;
        }

        if (process_file(argv[1], out, 0))
        {
                printf("Preprocessing completed successfully!\n");
                printf("Output file: %s\n", output_file);
        }
        else
        {
                printf("Preprocessing failed!\n");
                fclose(out);
                remove(output_file);
                return 1;
        }

        fclose(out);

        return 0;
}
