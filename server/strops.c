#include <stdio.h>
#include <stdlib.h>

int getlen(const char *string);
int find(const char *string1, const char *string2);
int find_all(const char *string1, const char *string2);
int find_pos(const char *string1, const char *string2, int location[]);

char *append(const char *string1, const char *string2);
char *pstrip(const char *string, const char *item, int amount);
void  dstrip(char *string, const char *item, int amount);

void  psplit(const char *string, const char *delim, char *new_1, char *new_2);
void  dsplit(const char *string, const char *delim, char *new_1, char *new_2);

void  simple_replace(char *string, const char *to_replace, const char *replacement);
void  replace(char *string, const char *to_replace, const char *replacement);
void  replace_all(char *string, const char *to_replace, const char *replacement);
void  replace_pos(char *string, const char to_replace, const char replacement, const int pos[], int increment);

void  dcopy(const char *string1, char *string2);
char *pcopy(const char *string1);
char *pisolate(const char *string, const char *isol);
void  disolate(const char *string1, char *string2, const char *isol);

int getlen(const char *string) {
    int i = 0;
    while (string[i] != '\0') i++;
    return i;
}

char *append(const char *string1, const char *string2) {
    char *result = malloc(getlen(string1) + getlen(string2) + 1);
    int i = 0, j = 0;
    while (string1[i] != '\0') {
        result[i] = string1[i];
        i++;
    }
    while (string2[j] != '\0') {
        result[j + i] = string2[j];
        j++;
    }
    result[j + i] = '\0';
    return result;
}

char *pstrip(const char *string, const char *item, int amount) {
    int found = 0;
    int i     = 0;
    int len   = getlen(string);
    char *msg = malloc(len + 1);

    while (string[i] != '\0') {
        msg[i] = string[i];
        i++;
    }
    msg[len] = '\0';

    for (i = 0; i < len;) {
        if (msg[i] == *item) {
            int item_len = getlen(item);
            for (int j = i; j < len - item_len + 1; j++) {
                msg[j] = msg[j + item_len];
            }
            len -= item_len;
            msg[len] = '\0';
            found++;
            if (found == amount) break;
        } else {
            i++;
        }
    }
    return msg;
}

void dstrip(char *string, const char *item, int amount) {  //destructive
    int found    = 0;
    int i        = 0;
    int len      = getlen(string);
    int item_len = getlen(item);

    while (string[i] != '\0') {
        if (string[i] == *item) {
            for (int j = i; j < len - item_len + 1; j++) {
                string[j] = string[j + item_len];
            }
            len -= item_len;
            string[len] = '\0';
            found++;
            if (found == amount) break;
        } else {
            i++;
        }
    }
}

void psplit(const char *string, const char *delim, char *new_1, char *new_2) {  //preserves delimiter
    int i = 0, k = 0;
    while (string[i] != '\0') {
        if (string[i] == *delim) {
            for (int j = 0; j != i; j++) {
                new_1[k] = string[j];
                k++;
            }
            new_1[k] = '\0';
            k = 0;
            for (int j = i; string[j] != '\0'; j++) {
                new_2[k] = string[j];
                k++;
            }
            new_2[k] = '\0';
            return;
        }
        i++;
    }
}

void dsplit(const char *string, const char *delim, char *new_1, char *new_2) {  //removes delimiter
    int i = 0, k = 0;
    while (string[i] != '\0') {
        if (string[i] == *delim) {
            for (int j = 0; j != i; j++) {
                new_1[k] = string[j];
                k++;
            }
            new_1[k] = '\0';
            k = 0;
            for (int j = i + 1; string[j] != '\0'; j++) {
                new_2[k] = string[j];
                k++;
            }
            new_2[k] = '\0';
            return;
        }
        i++;
    }
}

int find(const char *string1, const char *string2) {  //string1 is source
    int j = 0;
    for (int i = 0; string1[i] != '\0'; i++) {
        while (string1[i] == string2[j]) {
            i++;
            j++;
            if (j == getlen(string2)) return i - j;
        }
        j = 0;
    }
    return 0;
}

int find_all(const char *string1, const char *string2) {
    int l        = 0;
    int pat_len  = getlen(string2);

    for (int i = 0; string1[i] != '\0'; i++) {
        if (string1[i] == string2[0]) {
            int k = 0;
            while (string2[k] != '\0' && string1[i + k] == string2[k]) k++;
            if (k == pat_len) l++;
        }
    }
    return l;
}

int find_pos(const char *string1, const char *string2, int location[]) {
    int j = 0, k = 0;
    for (int i = 0; string1[i] != '\0'; i++) {
        while (string1[i] == string2[j]) {
            location[k] = i;
            i++;
            j++;
            k++;
            if (j == getlen(string2)) {
                location[k] = -1;
                return 1;
            }
        }
        j = 0;
        k = 0;
    }
    return 0;
}

void simple_replace(char *string, const char *to_replace, const char *replacement) {
    int len = getlen(string);
    for (int i = 0; i < len; i++) {
        if (string[i] == *to_replace) {
            string[i] = *replacement;
        }
    }
}

void replace(char *string, const char *to_replace, const char *replacement) {
    int len = getlen(string);
    int pos[len + 1];
    char cache[len + 1];

    for (int i = 0; i <= len; i++) cache[i] = string[i];

    if (find_pos(string, to_replace, pos)) {
        int rep_len    = getlen(replacement);
        int old_len    = getlen(to_replace);
        int start      = pos[0];

        for (int i = 0; i < rep_len; i++) {
            string[start + i] = replacement[i];
        }
        int tail = start + old_len;
        int dest = start + rep_len;
        while (cache[tail] != '\0') {
            string[dest++] = cache[tail++];
        }
        string[dest] = '\0';
    }
}

void replace_all(char *string, const char *to_replace, const char *replacement) {
    while (find(string, to_replace)) {
        replace(string, to_replace, replacement);
    }
}

void replace_pos(char *string, const char to_replace, const char replacement, const int pos[], int increment) {
    for (int i = increment; string[pos[i]] != '\0'; i++) {
        if (string[pos[i]] == to_replace) {
            string[pos[i]] = replacement;
        }
    }
}

void dcopy(const char *string1, char *string2) {
    int i = 0;
    for (; string1[i] != '\0'; i++) {
        string2[i] = string1[i];
    }
    string2[i] = '\0';
}

char *pcopy(const char *string1) {
    int len      = getlen(string1);
    char *string2 = malloc(len + 1);
    for (int i = 0; i <= len; i++) {
        string2[i] = string1[i];
    }
    return string2;
}

char *pisolate(const char *string, const char *isol) {
    int len  = getlen(string);
    int pos[len + 1];
    find_pos(string, isol, pos);
    char *res = malloc(len + 1);
    int i = 0;
    for (; pos[i] != -1; i++) {
        res[i] = string[pos[i]];
    }
    res[i] = '\0';
    return res;
}

void disolate(const char *string1, char *string2, const char *isol) {
    int len = getlen(string1);
    int pos[len + 1];
    find_pos(string1, isol, pos);
    int i = 0;
    for (; pos[i] != -1; i++) {
        string2[i] = string1[pos[i]];
    }
    string2[i] = '\0';
}
