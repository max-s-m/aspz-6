//Test Case 8: Use-After-Free (UAF) з лекції 6 Debugging Tools for Memory Issues in Linux
#include <stdlib.h>

int main(){
    int *ptr = malloc(sizeof(int));
    free(ptr);
    //UAF, use after free (вказівник використовується після його звільнення)
    *ptr = 10;
}
