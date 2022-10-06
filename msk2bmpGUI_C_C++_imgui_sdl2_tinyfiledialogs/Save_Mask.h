#pragma once

#define MAX_LINES 300
#define writelines(arg1, arg2) fwrite(arg2, MAX_LINES * 44, 1, arg1)

typedef char line_array_t[MAX_LINES][44];

int Save_Mask(bool isBMP, int argc, const char ** argv);
bool IsBMPFile(FILE *infile);
bool ReadBmpLines(FILE *file, line_array_t vOutput);            // same as below
void ReadMskLines(FILE *file, char vOutput[MAX_LINES][44]);
int BytesToInt(char *C, int numBytes);