#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>

int factorial(int n); 
int matmul(int p, int q, int s, int a[p*q], int b[q*s], int c[p*s]);
int matadd(int p, int q, int a[p*q], int b[p*q]);
int primeBetween(int m, int n);
int smallernuminmatrix(int p, int q, int a[p*q], int s);
int numdivisibleinmatrix(int p, int q, int a[p*q], int s);
int primeofmat(int p, int q, int a[p*q]);
int prime(int n);
static int choice = 0;
static int p, q, s, d, l;
static int a[2500], b[2500], c[2500];

module_param(choice, int, S_IRUGO);
module_param(d, int, S_IRUGO);
module_param(p, int, S_IRUGO);
module_param(q, int, S_IRUGO);
module_param(s, int, S_IRUGO);
module_param_array(a, int, NULL, S_IRUGO);
module_param_array(b, int, NULL, S_IRUGO);
module_param_array(c, int, NULL, S_IRUGO);

static int p02_init(void)
{
        switch (choice)
        {
        case 1:
                {
                printk(KERN_ALERT "\n%d! = %d", d, factorial(d));
                break;
                }
        case 2:
                {
                matadd(p, q, a, b);
                break;
                }
        case 3:
                {
                matmul(p, q, s, a, b, c);
                break;
                }        
        case 4: 
                {
                primeBetween(p, s);
                break;
                }
        case 5:
                {
                smallernuminmatrix(p, q, a, s);
                break;
                }
        case 6:
                {
                numdivisibleinmatrix(p, q, a, s);
                break;
                }
        case 7:
                {
                primeofmat(p, q, a);
                break;
                }
        }
        return 0;
}

int factorial(int n)
{
        int gt = 2;
        int i = 3;

        for(i=3; i<=n; i++)
        {
                gt *= i;
        }

        return gt;
}

int matadd(int p, int q, int a[p*q], int b[p*q]) {
    int i, j;
    for (i = 0; i < p; i++) {
        for (j = 0; j < q; j++) {
            *(c + j + i * q) = *(a + j + i * q) + *(b + j + i * q);
            //c[i][j] = a[i][j] + b[i][j];
            // *(a + j + i * q) += *(b + j + i * q);
        }
   }
    printk(KERN_ALERT "\n------------------------------\nThe sum of 2 matrices is a matrix with height %d and width %d\n",p,q);
    for (i=0;i<p;i++){
        for (j=0;j<q;j++){
            printk(KERN_ALERT "%-7d", *(c + (q * i) + j));
        }
        printk(KERN_ALERT "\n");
    }
    return 0;
}
 
int matmul(int p, int q, int s, int a[p*q], int b[q*s], int c[p*s]) {
    int i, j;
    for (i=0;i<p;i++){
        for (j=0;j<s;j++){
            for (l=0;l<q;l++){
                //c[i][j] = c[i][j] + a[i][l] * b[l][j];
                *(c + (s * i) + j) = *(c + (s * i) + j) + *(a + (q * i) + l) * *(b + (l * s) + j);
            }
        }
    }
    printk(KERN_ALERT "\n------------------------------\nThe product of 2 matrices is a matrix with height %d and width %d\n",p,s);
    for (i=0;i<p;i++){
        for (j=0;j<s;j++){
            printk(KERN_ALERT "%-7d", *(c + (s * i) + j));
        }
        printk(KERN_ALERT "\n");
    }
    return 0;
}

int primeBetween(int num1, int num2) {
    int flag_var, i, j;
    for (i = num1 + 1; i < num2; ++i) {
        flag_var = 0;
        for (j = 2; j <= i / 2; ++j) {
            if (i % j == 0) {
                flag_var = 1;
                break;
                }
            }
      if (flag_var == 0) printk(KERN_ALERT "%d\n",i);
    }
    return 0;
}

int smallernuminmatrix(int p, int q, int a[p*q], int s) {
    int i, j, count = 0;

    for (i = 0; i < p; i++) {
        for (j = 0; j < q; j++) {
            if (a[j + q * i] < s) {
                count++;
            }
        }
    }

    printk(KERN_ALERT "There are %d numbers smaller than %d in this matrix", count, s);
    return 0;
}

int numdivisibleinmatrix(int p, int q, int a[p*q], int s) {
    int i, j, count = 0;

    for (i = 0; i < p; i++) {
        for (j = 0; j < q; j++) {
            if (a[j + q * i] % s == 0) {
                count++;
            }
        }
    }
    printk(KERN_ALERT "There are %d numbers divisible by %d in this matrix", count, s);
    return 0;    
}

int primeofmat(int p, int q, int a[p*q]) {
    int i, j, count = 0;

    for (i = 0; i < p; i++) {
        for (j = 0; j < q; j++) {
            count += prime(a[j + q * i]);
        }
    }
    printk(KERN_ALERT "There are %d prime numbers in this matrix", count);
    return 0;   
}

int prime(int n) {
    if (n <= 1) {
        return 0;
    }
    int i;
    for (i = 2; i*i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

void display(int p, int q, int a[p][q]) {
    int i, j;
    for (i = 0; i < p; i++) {
        for (j = 0; j < q; j++) {
            printk(KERN_ALERT "%d ", a[i][j]);
        }
        printk("\n");
    }
}

static void p02_exit(void)
{
    printk(KERN_ALERT "Goodbye\n");
}

module_init(p02_init);
module_exit(p02_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nhom");
