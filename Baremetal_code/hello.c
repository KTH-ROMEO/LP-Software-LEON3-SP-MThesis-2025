#include <stdio.h> 


int v[10] = {0};

int s[10];

int main() {

    int a = 10;
    int i = 0;

    a = a + 1;

    // for(i = 0; i < 10; i++)
    // {
    //     s[i] =  v[i] + a;
    //     a ++;
    // }

    printf("a = %d\n", a);

    a++;

    printf("a = %d\n", a);

    printf("Hello world script ran succesfully!\n");

    return 0;
}
