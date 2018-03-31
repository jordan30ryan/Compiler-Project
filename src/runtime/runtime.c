#include <stdio.h>

/* PUT */

void PUTINTEGER(int* val)
{
    printf("%d", *val);
}

// Chars are treated as i8
void PUTCHAR(int* val)
{
    char c = (char)*val;
    printf("%c", c);
}

void PUTFLOAT(float* val)
{
    printf("%f", *val);
}

void PUTBOOL(int* val)
{
    printf("%s", *val? "true" : "false");
}

void PUTSTRING(char** str)
{
    printf("%s", *str);
}

/* GET */

void GETINTEGER(int* val)
{
    scanf("%d", val);
}

// Chars are treated as i8
void GETCHAR(int* val)
{
    char a;
    scanf(" %c", &a);
    *val = (int)a;
}

void GETFLOAT(float* val)
{
    scanf("%f", val);
}

void GETBOOL(int* val)
{
    scanf("%d", val);
}

void GETSTRING(char** str)
{
    scanf("%s", *str);
}
