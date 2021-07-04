#include <sysexits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"

#define ERR(...) fprintf(stderr, "ERROR: "__VA_ARGS__)
// max length of i32 as printf format
#define MAX_INT_LEN 10
// max length of float as printf format
// (10, plus period, plus 6 decimals)
#define MAX_FLOAT_LEN 17

char *PrettyFloat(char *buf, float f)
{
    // buffer used for this function should ALWAYS be of size:
    // MAX_FLOAT_LEN + 1
    sprintf(buf, "%f", f);
    char *period = strchr(buf, '.');
    bool hasDecimal = false;
    size_t i = 1;
    while (*(period + i) != '\0') {
        if (*(period + i) != '0') hasDecimal = true;
        ++i;
    }
    if (!hasDecimal)
        *period = '\0';
    return buf;
}

// TODO: replace with quicksort
void Sort(float *set, size_t size)
{
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        for (size_t i = 1; i < size; ++i) {
            if (set[i] < set[i - 1]) {
                sorted = false;
                float swap = set[i];
                set[i] = set[i - 1];
                set[i - 1] = swap;
            }
        }
    }
}

static float middle_of_range(const float *set, size_t start, size_t end)
{
    size_t size = end - start;
    if (size % 2 == 0) {
        return (set[(size / 2 - 1) + start] + set[(size / 2) + start]) / 2;
    } else {
        return set[(size / 2) + start];
    }
}

float Mean(const float *set, size_t size)
{
    float sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum += set[i];
    }
    return sum / size;
}

typedef struct {
    float Q1;
    float Q2;
    float Q3;
    float lowThresh;
    float highThresh;
    float IQR;
} CussInfo;

CussInfo GetCussInfo(const float *set, size_t size)
{
    CussInfo ret;
    if (size % 2 == 0) {
        ret.Q1 = middle_of_range(set, 0, size / 2 - 1);
        ret.Q2 = (set[size / 2 - 1] + set[size / 2]) / 2;
        ret.Q3 = middle_of_range(set, size / 2, size);
    } else {
        ret.Q1 = middle_of_range(set, 0, size / 2);
        ret.Q2 = set[size / 2];
        ret.Q3 = middle_of_range(set, size / 2, size);
    }
    ret.IQR = ret.Q3 - ret.Q1;
    ret.lowThresh  = ret.Q1 - (ret.IQR * 1.5);
    ret.highThresh = ret.Q3 + (ret.IQR * 1.5);
    return ret;
}

typedef struct {
    size_t low;
    size_t high;
} OutlierRange;

OutlierRange Outliers(const float *set, size_t size,
                      float lowThresh, float highThresh)
{
    OutlierRange ret;
    ret.low = 0;
    while (set[ret.low] < lowThresh)
        ++ret.low;
    ret.high = size;
    //>adding or subtracting one and I don't understand why
    // this is a certified C classic
    while (set[ret.high - 1] > highThresh)
        --ret.high;
    return ret;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        ERR("nothing was provided.\n");
        exit(EX_USAGE);
    }
    FILE* infile = fopen(argv[1], "r");
    if (!infile) {
        ERR("failed to open input file `%s`\n", argv[1]);
        exit(EX_NOINPUT);
    }
    float data[MAX_DATA_SIZE];
    size_t data_size = 0;
    char current_line[MAX_INT_LEN];
    while (fgets(current_line, MAX_INT_LEN, infile)) {
        if (data_size >= MAX_DATA_SIZE) {
            ERR("maximum set size of `%d` exceeded.\n"
                "increase MAX_DATA_SIZE in config.h if this is intentional.\n",
                MAX_DATA_SIZE);
            exit(EX_DATAERR);
        }
        float n = atof(current_line);
        data[data_size] = n;
        data_size++;
    }
    Sort(data, data_size);
    CussInfo info = GetCussInfo(data, data_size);
    OutlierRange outlier_range =
        Outliers(data, data_size, info.lowThresh, info.highThresh);
    printf("sorted data:\n");
    printf("---\n");
    for (size_t i = 0; i < data_size; ++i) {
        char buf[MAX_FLOAT_LEN + 1];
        printf("%s\n", PrettyFloat(buf, data[i]));
    }
    {
        printf("---\n");
        char buf[MAX_FLOAT_LEN + 1];
        printf("mean:     %s\n", PrettyFloat(buf, Mean(data, data_size)));
        printf("min:      %s\n", PrettyFloat(buf, data[0]));
        printf("Q1:       %s\n", PrettyFloat(buf, info.Q1));
        printf("median:   %s\n", PrettyFloat(buf, info.Q2));
        printf("Q3:       %s\n", PrettyFloat(buf, info.Q3));
        printf("max:      %s\n", PrettyFloat(buf, data[data_size - 1]));
        printf("---\n");
        printf("outliers:");
    }
    for (size_t i = 0; i < outlier_range.low; ++i) {
        char buf[MAX_FLOAT_LEN + 1];
        printf(" %s", PrettyFloat(buf, data[i]));
    }
    for (size_t i = outlier_range.high; i < data_size; ++i) {
        char buf[MAX_FLOAT_LEN + 1];
        printf(" %s", PrettyFloat(buf, data[i]));
    }
    printf("\n");
}
