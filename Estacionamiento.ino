#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN  10

LiquidCrystal_I2C LCD(0x27, 16, 2);

MFRC522 RFID(SS_PIN, RST_PIN);
byte LecturaUID[4]; // Arreglo para almacenar el UID leído
byte Usuario1[5]= {0x73, 0x52, 0x2C, 0x1C, 0x00}; // UID y estado del usuario 1
byte Usuario2[5]= {0x53, 0x84, 0x0D, 0x94, 0x00}; // UID y estado del usuario 2
byte Usuario3[5]= {0xE2, 0xB5, 0x63, 0xFA, 0x00}; // UID y estado del usuario 3

Servo servoMotor;
int servoPin = 2;
int servoCerrado = 15;
int servoAbierto= 90;

int ledVerde = 5;
int ledRojo = 6;

int lugaresDisponibles = 2;
int retraso = 5000;

void setup() {

  /* Configuración de los Leds */
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  
  /* Configuración inicial del LCD con I2C */
  LCD.init();      // Inicializa el modulo I2C
  LCD.backlight(); // Enciende la Luz del Fondo del LCD
  LCD.clear();     // Borra el contenido del LCD

  /* Configuración inicial del servomotor */
  servoMotor.attach(servoPin);    // Servo asociado al pin asignado a la variable servoPin
  servoMotor.write(servoCerrado); // Servo colocado a 0 grados

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
    if ( nuevoIngreso(Usuario1) && lugaresDisponibles > 0 ) {
      Usuario1[4] = 0xFF;
      lugaresDisponibles --;
      accesoConcedido();
    } else if ( ! nuevoIngreso(Usuario1) ) {
      Usuario1[4] = 0x00;
      lugaresDisponibles ++;
      salidaConcedida();
    }
  } else if (compararUID(LecturaUID, Usuario2)) {
    if ( nuevoIngreso(Usuario2) && lugaresDisponibles > 0 ) {
      Usuario2[4] = 0xFF;
      lugaresDisponibles --;
      accesoConcedido();
    } else if ( ! nuevoIngreso(Usuario2) ) {
      Usuario2[4] = 0x00;
      lugaresDisponibles ++;
      salidaConcedida();
    }
  } else if (compararUID(LecturaUID, Usuario3)) {
    if ( nuevoIngreso(Usuario3) && lugaresDisponibles > 0 ) {
      Usuario3[4] = 0xFF;
      lugaresDisponibles --;
      accesoConcedido();
    } else if ( ! nuevoIngreso(Usuario3) ) {
      Usuario3[4] = 0x00;
      lugaresDisponibles ++;
      salidaConcedida();
    }
  } else {
    accesoDenegado();
  }
}

/** 
 * Muestra en el LCD el texto de acceso concedido y levanta la pluma 
 */
void accesoConcedido() {
  mostrarMensajeEnLcd("Acceso concedido", 0, 0, "Puede ingresar", 1, 1);

  levantarPluma();

  (lugaresDisponibles > 0) ? bienvenida() : lugaresAgotados() ;
}

/** 
 * Muestra en el LCD el texto de acceso denegado y enciende el led rojo 
 */
void accesoDenegado() {

  mostrarMensajeEnLcd("Acceso denegado", 0, 0, "Pase invalido", 1, 1);
  
  digitalWrite(ledRojo, HIGH);
  delay(retraso);
  digitalWrite(ledRojo, LOW);

  (lugaresDisponibles > 0) ? bienvenida() : lugaresAgotados() ;
}

/** 
 * Muestra en el LCD el texto de bienvenida 
 */
void bienvenida() {
  digitalWrite(ledRojo, LOW);
  mostrarMensajeEnLcd("Bienvenido", 3, 0, "Presenta tu pase", 0, 1);
}

/** 
 * Muestra en el LCD el texto de de lugares agotados 
 */
void lugaresAgotados() {
  mostrarMensajeEnLcd("Lugares agotados", 0, 0, "Lo sentimos", 2, 1);
  digitalWrite(ledRojo, HIGH);
}

/** 
 * Muestra en el LCD el texto de salida concedida, levanta la pluma 
 * y enciende el led verde 
 */
void salidaConcedida() {
  digitalWrite(ledRojo, LOW);

  mostrarMensajeEnLcd("Salida concedida", 0, 0, "Vuelva pronto", 1, 1);
  levantarPluma();
  
  (lugaresDisponibles > 0) ? bienvenida() : lugaresAgotados() ;
}

/** 
 * Levanta la pluma y mantiene encendido el led verde por el momento 
 * que está levantada la pluma.
 */
void levantarPluma() {
  digitalWrite(ledVerde, HIGH);
  servoMotor.write(servoAbierto);
  delay(retraso);
  digitalWrite(ledVerde, LOW);
  servoMotor.write(servoCerrado); 
}

/** 
 * Se encarga de mostrar el texto en el LCD.
 * Soporta 2 mensajes correspondientes a las dos filas que dispone el LCD.
 *
 * Recibe los siguientes parametros: 
 * @param m1    : Mensaje 1
 * @param colm1 : Posición de la columna del mensaje 1
 * @param rowm1 : Posición de la fila del mensaje 1
 * @param m2    : Mensaje 2
 * @param colm2 : Posición de la columna del mensaje 2
 * @param rowm2 : Posición de la fila del mensaje 2
 */
void mostrarMensajeEnLcd(String m1, int colm1, int rowm1, String m2, int colm2, int rowm2) {
  LCD.clear();
  LCD.setCursor(colm1,rowm1);
  LCD.print(m1);
  LCD.setCursor(colm2,rowm2);
  LCD.print(m2);
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

/** 
 * Se encarga de saber si el UID del pase escaneado corresponde a un 
 * nuevo ingreso o a una salida.
 * Retorna true si corresponde a un nuevo ingreso.
 * Retorna false si corresponde a una salida
 */
boolean nuevoIngreso(byte usuario[]) {
  if (usuario[4] == 0x00) {
    return true;
  }
  
  return false;
}
