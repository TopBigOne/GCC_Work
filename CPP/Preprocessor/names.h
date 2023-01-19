//
// Created by dev on 2022/12/29.
//

#ifndef PREPROCESSOR_NAMES_H // 如果没有定义 PREPROCESSOR_NAMES_H ，那就执行以下的  define；
#define PREPROCESSOR_NAMES_H

#define  SLEN 32
typedef struct names_st {
    char first[SLEN];
    char last[SLEN];
} names;

void get_namse(names);

void show_names(const names *);

char *s_gets(char *st, int n);

#endif //PREPROCESSOR_NAMES_H
