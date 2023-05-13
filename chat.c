#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_PATH_LEN 256
#define MAX_FILES 256
#define MAX_VALUE_LEN 8192
#define MAX_LINE_LEN 8192

typedef struct {
    char user_name[MAX_VALUE_LEN];
    char model_name[MAX_VALUE_LEN];
    char case_code[MAX_VALUE_LEN];
    char delimiter[MAX_VALUE_LEN];
    char suffix[MAX_VALUE_LEN];
    char system[MAX_LINE_LEN];
} Params;

typedef struct {
    char user_name[MAX_VALUE_LEN];
    char model_name[MAX_VALUE_LEN];
    char system[MAX_LINE_LEN];
    char conversation[MAX_LINE_LEN];
} Prompt;

int get_files(const char *base_path, const char *to_search, char paths[][MAX_PATH_LEN], int *num_paths) {
    DIR *dir;
    struct dirent *entry;
    char search_extension = to_search[0] == '.' ? 1 : 0;
    char full_path[MAX_PATH_LEN];

    *num_paths = 0;
    if (!(dir = opendir(base_path))) return -1;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            if (!search_extension) {
                snprintf(full_path, MAX_PATH_LEN, "%s/%s/%s", base_path, entry->d_name, to_search);
                if (access(full_path, F_OK) != -1) {
                    strncpy(paths[*num_paths], entry->d_name, MAX_PATH_LEN);
                    (*num_paths)++;
                }
            }

        } else if (search_extension) {
            char *extension = strrchr(entry->d_name, '.');
            if (extension && strcmp(extension, to_search) == 0) {
                size_t length = extension - entry->d_name;
                strncpy(paths[*num_paths], entry->d_name, length);
                paths[*num_paths][length] = '\0';
                (*num_paths)++;
            }
        }
    }
    closedir(dir);
    return 0;
}

int get_selection(const char *prompt, char items[][MAX_PATH_LEN], int num_items) {
    int selection;
    char input[MAX_PATH_LEN];

    while (1) {
        printf("%s", prompt);
        fgets(input, MAX_PATH_LEN, stdin);
        input[strcspn(input, "\n")] = '\0';

        if (sscanf(input, "%d", &selection) == 1 &&
            selection >= 1 && selection <= num_items)
            return selection - 1;

        for (int i = 0; i < num_items; i++)
            if (strcmp(input, items[i]) == 0) return i;

        printf("Please enter a valid ID or name.\n");
    }
}

void sort_strings(char items[][MAX_PATH_LEN], int num_items) {
    char temp[MAX_PATH_LEN];
    int i, j;

    for (i = 0; i < num_items - 1; i++) {
        for (j = i + 1; j < num_items; j++) {
            if (strcmp(items[i], items[j]) > 0) {
                strcpy(temp, items[i]);
                strcpy(items[i], items[j]);
                strcpy(items[j], temp);
            }
        }
    }
}

int init(char *model_file_path, char *params_file_path, char *prompt_file_path) {
    int num_models = 0;
    char models[MAX_FILES][MAX_PATH_LEN];
    if (get_files("./models/", "params.txt", models, &num_models) != 0) return 1;
    sort_strings(models, num_models);

    printf("\n\e[34mAvailable models:\e[0m\n");
    for (int i = 0; i < num_models; i++) {
        printf("[%d] %s\n", i + 1, models[i]);
    }

    int model_selection = get_selection("ID or model name: ", models, num_models);
    char model_path[MAX_PATH_LEN];
    snprintf(model_path, MAX_PATH_LEN, "./models/%s", models[model_selection]);
    snprintf(params_file_path, MAX_PATH_LEN, "./models/%s/params.txt", models[model_selection]);

    char bins[MAX_FILES][MAX_PATH_LEN];
    int num_bins = 0;
    char bin_dir[MAX_PATH_LEN];
    snprintf(bin_dir, MAX_PATH_LEN, "%s/", model_path);
    if (get_files(bin_dir, ".bin", bins, &num_bins) != 0) return 1;
    sort_strings(bins, num_bins);

    printf("\n\e[34mAvailable model versions:\e[0m\n");
    for (int i = 0; i < num_bins; i++) {
        printf("[%d] %s\n", i + 1, bins[i]);
    }

    int bin_selection = get_selection("ID or bin file name: ", bins, num_bins);
    snprintf(model_file_path, MAX_PATH_LEN, "%s/%s.bin", model_path, bins[bin_selection]);

    int num_prompts = 0;
    char prompts[MAX_FILES][MAX_PATH_LEN];
    if (get_files("./prompts/", ".txt", prompts, &num_prompts) != 0) return 1;
    sort_strings(prompts, num_prompts);

    printf("\n\e[34mAvailable prompt files:\e[0m\n");
    for (int i = 0; i < num_prompts; i++) {
        printf("[%d] %s\n", i + 1, prompts[i]);
    }

    int prompt_selection = get_selection("ID or prompt file name: ", prompts, num_prompts);
    snprintf(prompt_file_path, MAX_PATH_LEN, "./prompts/%s.txt", prompts[prompt_selection]);

    return 0;
}

int parse_value(const char *line, Params *params) {
    char key[MAX_VALUE_LEN], value[MAX_VALUE_LEN];
    int parsed = sscanf(line, " %[^ =] = \"%[^\"]\"", key, value);
    if (parsed != 2) parsed = sscanf(line, " %[^ =] = %s", key, value);
    if (parsed != 2) return -1;

    if (strcmp(key, "user_name") == 0) strcpy(params->user_name, value);
    else if (strcmp(key, "model_name") == 0) strcpy(params->model_name, value);
    else if (strcmp(key, "case") == 0) strcpy(params->case_code, value);
    else if (strcmp(key, "delimiter") == 0) strcpy(params->delimiter, value);
    else if (strcmp(key, "suffix") == 0) strcpy(params->suffix, value);
    else if (strcmp(key, "system") == 0) strcpy(params->system, value);
    return 0;
}

int parse_prompt_value(const char *line, Prompt *prompt) {
    char key[MAX_VALUE_LEN], value[MAX_VALUE_LEN];
    int parsed = sscanf(line, " %[^ =] = \"%[^\"]\"", key, value);
    if (parsed != 2) parsed = sscanf(line, " %[^ =] = %s", key, value);
    if (parsed != 2) return -1;

    if (strcmp(key, "user_name") == 0)
        strcpy(prompt->user_name, value);
    else if (strcmp(key, "model_name") == 0)
        strcpy(prompt->model_name, value);
    else if (strcmp(key, "system") == 0)
        strcpy(prompt->system, value);
    return 0;
}

int read_params(const char *file_path, Params *params) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error: cannot open file %s\n", file_path);
        return -1;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, MAX_LINE_LEN, file))
        if (parse_value(line, params) != 0) return -1;

    fclose(file);
    return 0;
}

int read_prompts(const char *file_path, Prompt *prompts) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error: cannot open file %s\n", file_path);
        return -1;
    }

    char line[MAX_LINE_LEN];
    int in_conversation = 0;
    while (fgets(line, MAX_LINE_LEN, file)) {
        if (strstr(line, "---") != NULL) {
            in_conversation = 1;
            continue;
        }
        if (in_conversation) strcat(prompts->conversation, line);
        else parse_prompt_value(line, prompts);
    }

    fclose(file);
    return 0;
}

void replace_substring(char *str, const char *substr, const char *replacement) {
    char *pos;
    int substr_len = strlen(substr);
    int replacement_len = strlen(replacement);

    while ((pos = strstr(str, substr)) != NULL) {
        memmove(pos + replacement_len, pos + substr_len, strlen(pos + substr_len) + 1);
        memcpy(pos, replacement, replacement_len);
        str += replacement_len;
    }
}

char* convert_case(char* str, char* case_choice) {
    char* converted_str = malloc(strlen(str) + 1);
    if (converted_str == NULL) {
        return NULL;
    }

    for (int i = 0; str[i]; i++) {
        if (strcmp(case_choice, "upper") == 0) {
            converted_str[i] = toupper(str[i]);
        } else if (strcmp(case_choice, "lower") == 0) {
            converted_str[i] = tolower(str[i]);
        } else if (strcmp(case_choice, "title") == 0) {
            if (i == 0 || isspace(str[i-1])) {
                converted_str[i] = toupper(str[i]);
            } else {
                converted_str[i] = tolower(str[i]);
            }
        } else {
            free(converted_str);
            return NULL;
        }
    }
    converted_str[strlen(str)] = '\0';
    return converted_str;
}

char* replace_escape_sequences(char* str) {
    char* newStr = malloc(strlen(str) + 1);
    int i, j;

    for (i = 0, j = 0; str[i] != '\0'; i++, j++) {
        if (str[i] == '\\' && str[i+1] == 'n') {
            newStr[j] = '\n';
            i++;
        } else {
            newStr[j] = str[i];
        }
    }
    newStr[j] = '\0';
    return newStr;
}

int remove_trailing_spaces(char *str) {
    int spaces_removed = 0;
    int i, len = strlen(str);
    for (i = len - 1; i >= 0 && isspace(str[i]); i--)
        spaces_removed++;
    str[i+1] = '\0';
    return spaces_removed;
}

char *choose_value(const char *value1, const char *value2, const char *title) {
    char *selected_value = malloc(MAX_VALUE_LEN);
    int choice = 0;

    while (1) {
        printf("\n\e[34mChoose a value for %s:\e[0m\n[1] %s (from prompt - the default)\n[2] %s (from model)\n", title, value2, value1);
        printf("Choice (1, 2, or custom value): ");
        char input[MAX_VALUE_LEN];
        fgets(input, MAX_VALUE_LEN, stdin);
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) { strcpy(selected_value, value2); break; }
        else if (sscanf(input, "%d", &choice) == 1) {
            if (choice == 1) { strcpy(selected_value, value2); break; }
            else if (choice == 2) { strcpy(selected_value, value1); break; }
        } else {
            strcpy(selected_value, input);
            break;
        }
    }

    return selected_value;
}

int main(void) {
    Params params;
    Prompt prompt;

    char model_file_path[MAX_PATH_LEN];
    char params_file_path[MAX_PATH_LEN];
    char prompt_file_path[MAX_PATH_LEN];

    if (init(model_file_path, params_file_path, prompt_file_path) != 0) return 1;
    if (read_params(params_file_path, &params) != 0) return 1;
    if (read_prompts(prompt_file_path, &prompt) != 0) return 1;

    char command[MAX_LINE_LEN];
    char system_prompt[MAX_LINE_LEN];
    char full_prompt[MAX_LINE_LEN];
    char temp[MAX_LINE_LEN];

    char user_name[MAX_VALUE_LEN];
    char model_name[MAX_VALUE_LEN];
    char user_name_params[MAX_VALUE_LEN];
    char model_name_params[MAX_VALUE_LEN];
    char user_name_prompt[MAX_VALUE_LEN];
    char model_name_prompt[MAX_VALUE_LEN];

    char prefix[MAX_VALUE_LEN];
    char suffix[MAX_VALUE_LEN];
    char stop[MAX_VALUE_LEN];

    const char *user_tag_full = "<user>";
    const char *model_tag_full = "<model>";
    const char *user_tag_partial = "{user}";
    const char *model_tag_partial = "{model}";

    strcpy(user_name_params, convert_case(params.user_name, params.case_code));
    strcpy(model_name_params, convert_case(params.model_name, params.case_code));
    strcpy(user_name_prompt, convert_case(prompt.user_name, params.case_code));
    strcpy(model_name_prompt, convert_case(prompt.model_name, params.case_code));

    strcpy(user_name, choose_value(user_name_params, user_name_prompt, "user's name"));
    strcpy(model_name, choose_value(model_name_params, model_name_prompt, "model's name"));
    strcpy(system_prompt, choose_value(params.system, prompt.system, "system"));

    // If a custom value is given
    strcpy(user_name, convert_case(user_name, params.case_code));
    strcpy(model_name, convert_case(model_name, params.case_code));

    if (strcmp(params.delimiter, "none") != 0) {
        strcpy(stop, params.delimiter);
        sprintf(prefix, " %s%s", user_name, params.suffix);
        sprintf(suffix, "%s %s%s", params.delimiter, model_name, params.suffix);
        sprintf(full_prompt, "%s\n%s%s", system_prompt, prompt.conversation, params.delimiter);
        sprintf(temp, "%s %s%s", params.delimiter, user_name, params.suffix);
        replace_substring(full_prompt, user_tag_full, temp);
    } else {
        sprintf(stop, "%s%s", user_name, params.suffix);
        sprintf(prefix, "%s", params.suffix);
        sprintf(suffix, "%s%s", model_name, params.suffix);
        sprintf(full_prompt, "%s\n%s%s%s", system_prompt, prompt.conversation,
                convert_case(user_name, params.case_code), params.suffix);
        replace_substring(full_prompt, user_tag_full, stop);
    }

    replace_substring(full_prompt, model_tag_full, suffix);
    replace_substring(full_prompt, user_tag_partial, convert_case(user_name, "title"));
    replace_substring(full_prompt, model_tag_partial, convert_case(model_name, "title"));

    int spaces_removed = remove_trailing_spaces(stop);
    remove_trailing_spaces(suffix);
    remove_trailing_spaces(full_prompt);

    if (strcmp(params.delimiter, "none") == 0) {
        remove_trailing_spaces(prefix);
        for (int i = 0; i < spaces_removed; i++) prefix[i] = ' ';
        prefix[spaces_removed] = '\0';
    }

    sprintf(command, "./src/main --model \"%s\" --prompt \"%s\" --reverse-prompt \"%s\" --in-suffix \"%s\" --in-prefix \"%s\" --interactive-first --threads 4 --n_predict -1 --repeat_last_n 256 --ctx_size 2048 --color --no-mmap",
            model_file_path, full_prompt, stop, suffix, prefix);

    char *final_command = replace_escape_sequences(command);
    printf("\n\e[32mGenerated command :\n%s\e[0m\n\n", final_command);
    system(final_command);
    return 0;
}
