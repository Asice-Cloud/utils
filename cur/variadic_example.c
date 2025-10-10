#include <stdio.h>
#include <stdarg.h>

// 例子1: 计算多个整数的和
int sum(int count, ...) {
    va_list args;
    va_start(args, count);  // 从count后开始
    
    int total = 0;
    for (int i = 0; i < count; i++) {
        int value = va_arg(args, int);  // 获取下一个int参数
        total += value;
    }
    
    va_end(args);
    return total;
}

// 例子2: 打印多个不同类型的参数
void print_values(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    for (const char *p = format; *p != '\0'; p++) {
        switch (*p) {
            case 'i': {  // 整数
                int i = va_arg(args, int);
                printf("Integer: %d\n", i);
                break;
            }
            case 'f': {  // 浮点数
                double d = va_arg(args, double);  // float被提升为double
                printf("Float: %.2f\n", d);
                break;
            }
            case 's': {  // 字符串
                char *s = va_arg(args, char*);
                printf("String: %s\n", s);
                break;
            }
            case 'c': {  // 字符
                int c = va_arg(args, int);  // char被提升为int
                printf("Char: %c\n", c);
                break;
            }
        }
    }
    
    va_end(args);
}

// 例子3: 找出多个数中的最大值
int max_value(int count, ...) {
    if (count <= 0) return 0;
    
    va_list args;
    va_start(args, count);
    
    int max = va_arg(args, int);  // 第一个值作为初始最大值
    
    for (int i = 1; i < count; i++) {
        int current = va_arg(args, int);
        if (current > max) {
            max = current;
        }
    }
    
    va_end(args);
    return max;
}

// 例子4: 自定义的printf简化版
void my_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    for (const char *p = format; *p != '\0'; p++) {
        if (*p != '%') {
            putchar(*p);
            continue;
        }
        
        // 处理%后的格式符
        switch (*++p) {
            case 'd': {
                int i = va_arg(args, int);
                printf("%d", i);
                break;
            }
            case 's': {
                char *s = va_arg(args, char*);
                printf("%s", s);
                break;
            }
            case 'c': {
                int c = va_arg(args, int);
                putchar(c);
                break;
            }
            case '%':
                putchar('%');
                break;
            default:
                putchar('%');
                putchar(*p);
                break;
        }
    }
    
    va_end(args);
}

int main() {
    printf("=== 可变参数函数演示 ===\n\n");
    
    // 测试sum函数
    printf("1. 计算多个数的和:\n");
    printf("sum(3, 10, 20, 30) = %d\n", sum(3, 10, 20, 30));
    printf("sum(5, 1, 2, 3, 4, 5) = %d\n", sum(5, 1, 2, 3, 4, 5));
    printf("\n");
    
    // 测试print_values函数
    printf("2. 打印不同类型的值:\n");
    print_values("ifsc", 42, 3.14, "Hello", 'A');
    printf("\n");
    
    // 测试max_value函数
    printf("3. 找最大值:\n");
    printf("max_value(4, 10, 5, 25, 15) = %d\n", max_value(4, 10, 5, 25, 15));
    printf("max_value(6, 100, 200, 50, 300, 150, 250) = %d\n", 
           max_value(6, 100, 200, 50, 300, 150, 250));
    printf("\n");
    
    // 测试自定义printf
    printf("4. 自定义printf:\n");
    my_printf("Hello %s! You have %d messages.\n", "World", 5);
    my_printf("Character: %c, Percent: %%\n", 'X');
    
    return 0;
}