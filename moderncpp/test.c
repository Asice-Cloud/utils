#include <stdio.h>
#include <stdlib.h>

int main()
{
	int arr[10];
	char i = 0;
	int j = 0;
	while (scanf("%s", &i) != EOF)
	{
		arr[j] = i;
		j++;
		if (getchar() == '\n')
			break;
	}
	for (j = 0; j < 10; j++)
	{
		printf("%c\n", arr[j]);
	}
}
