#include <Wire.h>               // I2C lib.
#include <Adafruit_MLX90614.h>  // INFRA sensor lib.
#include <SoftwareSerial.h>     // SW serial

SoftwareSerial kepernyo(7, 8); // RX, TX SW serial object

Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // infra object

//################# változók
const int trigPin  = 9;   // ultrahang trigger
const int echoPin  = 10;  // ultrahang echo sign
const int Relay    = 5;   // Relay
const int ir_dist  = 3;   // IR distance for hand sanitizer 
const int buzzer   = 12;  // buzzer

long duration;
int distance;
int page_counter = 0;
int kep = 0;
int infra = 1;
int minimum_range = 10;
int maximum_range = 20;
int correction = 0;
float kulso_temp;   //defa. ambience temp.
float paciens_temp;  //human temp.

void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);   // Start debug serial
 kepernyo.begin(9600); // Start  SW serial
 mlx.begin();   // start infra 
    kepernyo.print("main.t0.txt=\"CovidTron I.\""); kepernyo.print("\xFF\xFF\xFF"); // Send covidtron for nextion display

pinMode(trigPin, OUTPUT); // trigger out
pinMode(Relay, OUTPUT); // relay out
pinMode(buzzer, OUTPUT); // buzzer
pinMode(echoPin, INPUT); // echo 
pinMode(ir_dist, INPUT); // ir input

 Serial.println("Üdvözöl a CovidTron I. debug konzolja");  // welcome msg on serial
 csipcsip(); // probe buzzer and 
}

void csipcsip() {
tone(buzzer, 500,50); 
 delay(250);
noTone(buzzer); 
}

void loop() {
//################# relay HIGH if IR ON
infra = digitalRead(ir_dist);
if(infra == 0){ 
  digitalWrite(Relay,HIGH); delay(6000);digitalWrite(Relay,LOW); delay(7000);  //pumped sanitizer time, and pause time
} else { 
  digitalWrite(Relay,LOW);
   }
// counter for safe mem. overflow
//Hun: kell egy számláló, mert nem mehet a végtelenbe memory overflow miatt, ezert maximalom. 0 sem lehet, mert csak akkor 
// frissit fokepernyore
page_counter++;  
  if((page_counter == 1000) && (kep == 1)){ kep = 0;}
  if(page_counter == 1000){page_counter = 1;} // no mem. overflow
  
kulso_temp    = mlx.readAmbientTempC();
paciens_temp = mlx.readObjectTempC();

  Serial.print("Ambient = "); Serial.print( mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
  Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
  Serial.println();
  delay(500);

 // trigger OFF
digitalWrite(trigPin, LOW);
delayMicroseconds(2);

// trigger up and ping
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

// read value
duration = pulseIn(echoPin, HIGH);

// calculate distance
distance= duration*0.034/2;

// ird debugra mit mérsz / to debug serial
Serial.print("Distance: ");
Serial.println(distance); 

//msg to nextion display 
  if(distance < 10  ) {kepernyo.print("main.t0.txt=\" Too Close!\""); kepernyo.print("\xFF\xFF\xFF"); } //3
  if(distance > 20 && distance < 30 ) {kepernyo.print("main.t0.txt=\" Too Away!\""); kepernyo.print("\xFF\xFF\xFF"); }
  if(distance > 40  ) {kepernyo.print("main.t0.txt=\"CovidTron I.\""); kepernyo.print("\xFF\xFF\xFF"); }
  
 // itt döntöm el milyen értéknél mi történik / calculate TEMP 
if(distance >=10 && distance <= 20 && paciens_temp >= 25){ 
kepernyo.print("main.t0.txt=\"CovidTron I.\""); kepernyo.print("\xFF\xFF\xFF");
paciens_temp = 10.6 + round(mlx.readObjectTempC() * 10 ) * .1 ;  // Ez a höm. kalibrációs sor. Az első értéket buzeráld / calibartion with first float number
if(kep == 0){
  kepernyo.print("page main");
  kepernyo.print("\xFF\xFF\xFF");
  kep = 1;
}

Serial.print("hőmérséklet: "); // Temperature
Serial.println(paciens_temp); 
if(paciens_temp >37.5){ 
  csipcsip();
  kepernyo.print("main.t1.bco=RED");
  kepernyo.print("\xFF\xFF\xFF");
} else {
kepernyo.print("main.t1.bco=1024");
kepernyo.print("\xFF\xFF\xFF");
}
  kepernyo.print("main.t1.txt=\"" + String(paciens_temp) +"c\"");
  kepernyo.print("\xFF\xFF\xFF");
} 
else {
  kepernyo.print("main.t1.txt=\"00.0c\"");
  kepernyo.print("\xFF\xFF\xFF");

  kepernyo.print("main.t1.bco=0");
  kepernyo.print("\xFF\xFF\xFF");
  kep = 0;
} 
  } // void vége
