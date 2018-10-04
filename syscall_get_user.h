#pragma once

#include <linux/list.h>

#define MAX_NAME_LEN 50

struct Person {
    char name[MAX_NAME_LEN];
    char surname[MAX_NAME_LEN];
    char phone[MAX_NAME_LEN];
    char email[MAX_NAME_LEN];
    char age[MAX_NAME_LEN];
};

