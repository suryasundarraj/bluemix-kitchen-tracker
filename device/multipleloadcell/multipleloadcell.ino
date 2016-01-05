#include<stdio.h>
#include<math.h>
#include "HX711.h"

#define calibration_factor -395050 
#define DOUT   4
#define CLK    2
#define DOUT1  5
#define CLK1   3

const char *g_container1 = "001";
const char *g_container2 = "002";

HX711 scale(DOUT, CLK);
HX711 scale1(DOUT1, CLK1);

void setup() {
  Serial1.begin(9600);
  scale1.set_scale(calibration_factor); 
  scale.set_scale(calibration_factor);
  scale1.tare();
  scale.tare(); 
}

void loop() {
  String cmd = "{\"g_container1\":\"";
  cmd += dataOne();
  cmd += "\",\"g_container2\":\"";
  cmd += dataTwo();
  cmd += "\"}";
  Serial1.println(cmd);
  delay(15000);
}

char *dataOne(){
  char l_buf[50];
  float l_weight_in_kgs_0 = scale.get_units()  * 0.453592;
  if(l_weight_in_kgs_0 >= 0.01){
    ftoa(l_weight_in_kgs_0,l_buf,2);
    return l_buf;
  }
  else{
    return "0";
  }
}

char *dataTwo(){
  char l_buf[50];
  float l_weight_in_kgs_1 = scale1.get_units() * 0.453592;
  if(l_weight_in_kgs_1 >= 0.01){
    ftoa(l_weight_in_kgs_1,l_buf,2);
    return l_buf;
  }
  else{
    return "0";
  }
}

void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
 
    reverse(str, i);
    str[i] = '\0';
    return i;
}

void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
 
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot
        fpart = fpart * pow(10, afterpoint);
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

