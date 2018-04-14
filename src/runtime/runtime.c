#include <stdio.h>

/* PUT */

void PUTINTEGER(int* val)
{
    printf("%d\n", *val);
}

// Chars are treated as i8
void PUTCHAR(int* val)
{
    char c = (char)*val;
    printf("%c\n", c);
}

void PUTFLOAT(float* val)
{
    printf("%f\n", *val);
}

void PUTBOOL(int* val)
{
    printf("%s\n", *val? "true" : "false");
}

void PUTSTRING(char** str)
{
    printf("%s\n", *str);
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
    // Convert to bool; 0 = false, anything else = true
    *val = *val != 0;
}

void GETSTRING(char** str)
{
    scanf("%s", *str);
}
