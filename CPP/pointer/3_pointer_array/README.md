## 大前提： 指针是可以进行 算数运算的。
----
### 数组指针 就是指向数组的指针。
```c++
double (*p)[5];
int main ()
{
    double arry[5] = {1000.0, 2.0, 3.4, 7.0, 50.0};
    double (*p)[5] = &arry;
    for (int i = 0; i < 5; i++)
        cout << (*p)[i] << endl;
    system("pause");
    return 0;
}


```



