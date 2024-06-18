#include <stdio.h>
int main (){
    int *arr;
    int a[5]={0,1,2,3,4};
    arr =a;
    int b[3]={22,33,44};
    arr=a;
    printf("%d %d %d %d %d\n",a[0],a[1],a[2],a[3],a[4]);
    printf("%d %d %d %d %d\n",arr[0],arr[1],arr[2],arr[3],arr[4]);
    arr=b;
    printf("%d %d %d %d\n",arr[0],arr[1],arr[2],arr[3]);
    return 0;
}