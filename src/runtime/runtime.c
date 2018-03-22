#include <stdio.h>

void PUTINTEGER(int val)
{
    printf("%d", val);
}

void PUTCHAR(char val)
{
    printf("%c", val);
}

void PUTFLOAT(float val)
{
    printf("%f", val);
}

void PUTBOOL(int val)
{
    printf("%s", val? "true" : "false");
}

void PUTSTRING(char* str)
{
    printf("%s", str);
}

int GETINTEGER()
{
    int ret;
    scanf("%d", &ret);
    return ret;
}
