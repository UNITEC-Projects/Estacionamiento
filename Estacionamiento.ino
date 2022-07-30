#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN  10

LiquidCrystal_I2C LCD(0x27, 16, 2); // Crear objeto del módulo I2C

MFRC522 RFID(SS_PIN, RST_PIN); // Crear objeto del RFID
byte LecturaUID[4]; // Arreglo para almacenar el UID leído
byte Usuario1[5]= {0x73, 0x52, 0x2C, 0x1C, 0x00}; // UID y estado del usuario 1
byte Usuario2[5]= {0x53, 0x84, 0x0D, 0x94, 0x00}; // UID y estado del usuario 2
byte Usuario3[5]= {0xE2, 0xB5, 0x63, 0xFA, 0x00}; // UID y estado del usuario 3

Servo servoMotor;
int servoPin = 6;
int servoCerrado = 0;
int servoAbierto= 90;

int retraso = 5000;

void setup() {
  Serial.begin(9600); // Inicializa comunicacion por monitor serie a 9600 bps
  
  /* Configuración inicial del LCD con I2C */
  LCD.init();      // Inicializa el modulo I2C
  LCD.backlight(); // Enciende la Luz del Fondo del LCD
  LCD.clear();     // Borra el contenido del LCD

  /* Configuración inicial del servomotor */
  servoMotor.attach(servoPin); // Servo asociado al pin asignado a la variable servoPin
  servoMotor.write(servoCerrado);         // Servo colocado a 0 grados

  /* Configuración inicial del RFID */
  RFID.PCD_Init();  // Inicializa modulo RFID
  SPI.begin();      // Inicializa bus SPI

  bienvenida();
}

void loop() {
  
  /* Si no hay una tarjeta presente retorna al loop esperando por una tarjeta */
  if ( ! RFID.PICC_IsNewCardPresent() ) 
    return;

  /* Si no puede obtener datos de la tarjeta retorna al loop esperando por otra tarjeta */
  if ( ! RFID.PICC_ReadCardSerial() ) 
    return;

  /* Almacena el UID leído */
  for (byte i = 0; i < 4; i++) {
    Serial.print(RFID.uid.uidByte[i], HEX);
    LecturaUID[i] = RFID.uid.uidByte[i];
  }

  /* Compara el UID leído los UID registrados */
  if (compararUID(LecturaUID, Usuario1)) {
    if ( nuevoIngreso(Usuario1) ) {
      Usuario1[4] = 0xFF;
      accesoConcedido();
    } else {
      Usuario1[4] = 0x00;
      salidaConcedida();
    }
  } else if (compararUID(LecturaUID, Usuario2)) {
    if ( nuevoIngreso(Usuario2) ) {
      Usuario2[4] = 0xFF;
      accesoConcedido();
    } else {
      Usuario2[4] = 0x00;
      salidaConcedida();
    }
  } else if (compararUID(LecturaUID, Usuario3)) {
    if ( nuevoIngreso(Usuario3) ) {
      Usuario3[4] = 0xFF;
      accesoConcedido();
    } else {
      Usuario3[4] = 0x00;
      salidaConcedida();
    }
  } else {
    accesoDenegado();
  }
  
  RFID.PICC_HaltA(); // Detiene comunicacion con tarjeta    
}

/** Muestra en el LCD el texto de acceso concedido y levanta la pluma */
void accesoConcedido() {
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Acceso concedido");
  LCD.setCursor(1,1);
  LCD.print("Puede ingresar");
  
  servoMotor.write(servoAbierto);
  delay(retraso);
  servoMotor.write(servoCerrado);
  
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

/** Muestra en el LCD el texto de bienvenida */
void bienvenida() {
  LCD.setCursor(3,0);
  LCD.print("Bienvenido");
  LCD.setCursor(0,1);
  LCD.print("Presenta tu pase");
}

/** Muestra en el LCD el texto de salida concedida y levanta la pluma */
void salidaConcedida() {
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Salida concedida");
  LCD.setCursor(1,1);
  LCD.print("Vuelva pronto");
  
  servoMotor.write(servoAbierto);
  delay(retraso);
  servoMotor.write(servoCerrado);
  
  LCD.clear();
  bienvenida();
}

/** 
 * Compara el UID de la tarjeta leída con el UID de un usuario registrado. 
 * Si el UID leído corresponde al de un usuario registrado retorna true,
 * de lo contrario retornará false.
 */
boolean compararUID(byte lectura[],byte usuario[]) {
  for (byte i = 0; i < 4; i++) {
    if (lectura[i] != usuario[i])
      return false;
    }
  return true;
}

/**  */
boolean nuevoIngreso(byte usuario[]) {
  if (usuario[4] == 0x00) {
    Serial.println("\t Nuevo ingreso");
    return true;
  } else {
    Serial.println("\t Salida");
    return false;
  }
}
