#include "HX711.h"
#define calibration_factor -395050 
#define DOUT  4
#define CLK  2
#define DOUT1  5
#define CLK1  3

HX711 scale(DOUT, CLK);
HX711 scale1(DOUT1, CLK1);

void setup() {
  Serial.begin(9600);
  Serial.println("Readings:");
  scale1.set_scale(calibration_factor); 
  scale.set_scale(calibration_factor);
  scale1.tare();
  scale.tare(); 
}

void loop() {
  dataOne();
  delay(0);
  dataTwo();
  Serial.println("\n\n");
}

void dataOne(){
  float l_weight_in_kgs_0 = scale.get_units()  * 0.453592;
  Serial.print("Rice : ");
  Serial.print(l_weight_in_kgs_0, 2); 
  Serial.println(" kgs");
}

void dataTwo(){
  float l_weight_in_kgs_1 = scale1.get_units() * 0.453592;
  Serial.print("Wheat : ");
  Serial.print(l_weight_in_kgs_1, 2); 
  Serial.print(" kgs"); 
}

