#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN  10

MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear objeto del RFID
LiquidCrystal_I2C LCD(0x27, 16, 2);   

byte LecturaUID[4]; // Arreglo para almacenar el UID leído
byte Usuario1[4]= {0x73, 0x52, 0x2C, 0x1C}; // UID del usuario 1
byte Usuario2[4]= {0x53, 0x84, 0x0D, 0x94}; // UID del usuario 2
byte Usuario3[4]= {0xE2, 0xB5, 0x63, 0xFA}; // UID del usuario 3

Servo servoMotor;
int servoPin = 6;
int retraso = 3000;

void setup() {
  /* Configuración inicial del LCD con I2C */
  LCD.init();
  LCD.backlight();
  LCD.clear();

  /* Configuración inicial del servomotor */
  Serial.begin(9600);          // inicializa comunicacion por monitor serie a 9600 bps
  servoMotor.attach(servoPin); // Servo asociado al pin 6 y lleva a 170 grados
  SPI.begin();                 // inicializa bus SPI
  mfrc522.PCD_Init();          // inicializa modulo lector

  bienvenida();
}

void loop() {
  
  /* Si no hay una tarjeta presente retorna al loop esperando por una tarjeta */
  if ( ! mfrc522.PICC_IsNewCardPresent() ) 
    return;

  /* Si no puede obtener datos de la tarjeta retorna al loop esperando por otra tarjeta */
  if ( ! mfrc522.PICC_ReadCardSerial() ) 
    return;         

  /* Almacena el UID leido */
  for (byte i = 0; i < 4; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    LecturaUID[i]=mfrc522.uid.uidByte[i];
  }

  /* Compara el UID leído los UID registrados */
  if (compararUID(LecturaUID, Usuario1)) {
    accesoConcedido();
  } else if (compararUID(LecturaUID, Usuario2)) {
    accesoConcedido();
  } else if (compararUID(LecturaUID, Usuario3)) {
    accesoConcedido();
  } else {
    accesoDenegado();
  }
  
  mfrc522.PICC_HaltA(); // detiene comunicacion con tarjeta    
}

/** Muestra en el LCD el texto de acceso concedido y levanta la pluma */
void accesoConcedido() {
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Acceso concedido");
  LCD.setCursor(4,1);
  LCD.print("Buen dia");
  
  servoMotor.write(0);
  delay(retraso);
  servoMotor.write(90);

  LCD.clear();
  bienvenida();
}

/** Muestra en el LCD el texto de acceso denegado */
void accesoDenegado() {
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Acceso denegado");
  LCD.setCursor(1,1);
  LCD.print("Pase invalido");
  delay(retraso);
  
  LCD.clear();
  bienvenida();
}

/** Muestra en el LCD texto estadar de bienvenida */
void bienvenida() {
  LCD.setCursor(3,0);
  LCD.print("Bienvenido");
  LCD.setCursor(0,1);
  LCD.print("Presenta tu pase");
}

/** 
 * Compara el UID de la tarjeta leída con el UID de un usuario registrado. 
 * Si el UID leído corresponde al de un usuario registrado retorna true,
 * de lo contrario retornará false.
 */
boolean compararUID(byte lectura[],byte usuario[]) {
  for (byte i=0; i < 4; i++) {
  if (lectura[i] != usuario[i])
    return(false);
  }
  return(true);
}
