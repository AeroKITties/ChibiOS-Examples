#include "print_lib.hpp"

// Some functions for printing ower SWO (debug only)
void SWO_PrintChar(char const c, uint8_t const portNo)
{
    volatile int timeout;

    /* Check if Trace Control Register (ITM->TCR at 0xE0000E80) is set */
    /* check Trace Control Register if ITM trace is enabled*/
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0)
    {
        return; /* not enabled? */
    }
    /* Check if the requested channel stimulus port (ITM->TER at 0xE0000E00) is enabled */
    /* check Trace Enable Register if requested port is enabled */
    if ((ITM->TER & (1ul << portNo)) == 0)
    {
        return; /* requested port not enabled? */
    }
    timeout = 5000; /* arbitrary timeout value */
    while (ITM->PORT[0].u32 == 0)
    {
        /* Wait until STIMx is ready, then send data */
        if (--timeout == 0)
        {
            return; /* not able to send */
        }
    }
    ITM->PORT[0].u8 = (uint8_t)c;
}
void SWO_PrintString(char const* s, uint8_t const portNumber)
{
    while (*s != '\0')
    {
        SWO_PrintChar(*s++, portNumber);
    }
}

void printString(char const* str)
{
    SWO_PrintString(str, 0);
}

///////////printDouble/////////////

double my_pow (double n, int p){
        if (p < 0)
            printString("p must be an integer >= 0");
        double ans = 1;
        while (p != 0){
            ans *= n;
            p--;
        }
        return ans;
}

char num_symb_to_char(int n){
	switch(n)
        {
            case 1:
                return '1';
                break;
            case 2:
                return '2';
                break;
            case 3:
                return '3';
                break;
            case 4:
                return '4';
                break;
            case 5:
                return '5';
                break;
            case 6:
                return '6';
                break;
            case 7:
                return '7';
                break;
            case 8:
                return '8';
                break;
            case 9:
                return '9';
                break;
	    case 0:
		return '0';
		break;
        }
        printString("an integer from 0 to 9 must be entered");
}

int get_digit_int(int num){
    int digit_num = 1;
    while (num / my_pow(10, digit_num) > 1){
            digit_num += 1;
    }
    return digit_num;
}

int get_digit_num(double num, int accuracy = 6){
    int num_high = int(num);
    int num_low = (num - num_high) * my_pow(10, accuracy);

    if (num_low == 0)
	    return get_digit_int(num_high);

    while (num_low % 10 == 0){
	    num_low /=10;
    }

    return get_digit_int(num_high) + get_digit_int(num_low);
}

char* int_to_string(int num){
    int digit_num = get_digit_num(num);

    char* str = (char*) malloc(sizeof(char)*digit_num);
    for (int i = 0; i < digit_num; i++){
        int n = (num / int(my_pow(10, digit_num-i-1))) % 10;
	      str[i] = num_symb_to_char(n); 
    }
    str[digit_num] = '\0';
    char last = str[digit_num];
    return str;
}

char* double_to_string(double dnum, int accuracy = 6){
    bool minus_sign = false;
    if (dnum < 0){
        minus_sign = true;
        dnum *= (-1);
    }
    int dnum_high = int(dnum);

    double dnum_low_ = dnum - int(dnum);
    int dnum_low = dnum_low_ * my_pow(10, get_digit_num(dnum_low_, accuracy));

    char* str_high = int_to_string(dnum_high);
    char* str_low = int_to_string(dnum_low);

    char* ans = (char*) malloc(sizeof(char)*(get_digit_num(dnum) + 1 + minus_sign));
    int i = 0;
    if (minus_sign){
        ans[i] = '-';
        i++;
    }
    while (str_high[i] != '\0'){
	    ans[i] = str_high[i-minus_sign];
	    i++;
    }
    ans[i++] = '.';
    int j = 0;
    while (str_low[j] != '\0'){
            ans[j+i] = str_low[j];
	    j++;
    }
    int t = i+j;
    ans[j+i] = '\0';
    return ans;

}

void printDouble(double val){
    printString(double_to_string(val));
    printString("\n");
}

////////////////////////////