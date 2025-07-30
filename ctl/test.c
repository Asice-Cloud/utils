#define P
#define T int
#include "vec.h"

#include <stdio.h>

int main()
{
    vec_int vi = vec_int_init();
    vec_int_push_back(&vi, 42);
    vec_int_push_back(&vi, 84);
    vec_int_push_back(&vi, 126);

    for (size_t i = 0; i < vi.size; i++)
    {
        printf("%d ", *vec_int_at(&vi, i));
    }
    printf("\n");
}