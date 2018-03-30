#include <stdio.h>

/* PUT */

void PUTINTEGER(int* val)
{
    printf("%d", *val);
}

void PUTCHAR(char* val)
{
    printf("%c", *val);
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

void GETCHAR(char* val)
{
    scanf("%c", val);
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
