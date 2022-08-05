/*
 * Equipo: 32
 * Participantes: Erika Yunue Morales Mateo
 * Programacion del nodo sensor Biblioteca 
 * Por: Equipo 
 * Integrates: 
 * Fecha: 05 de Agosto de 2022
 * 
 * En las siguientes lineas de programacion se configura la red de internet, el protocolo mqtt, los sensores propuesto y el actuador visual (Led)
 */

//*******************************Bibliotecas*****************************************

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"        // Libreria del sensor de temperatura y humedad
#include <MQ2.h> // Libreria del sensor de mq2 para gas y humo

// *********************** Espesificaciones de la red a conectar el nodo *************************

const char* ssid = "INFINITUMB5A3_2.4";// Pon aquí el nombre de la red a la que deseas conectarte INFINITUMB5A3_2.4
const char* password = "EBsH6qd96W"; //Escribe la contraseña de dicha red EBsH6qd96W
const char* mqtt_server = "192.168.1.132";//Escribe la direccion ip del broker mqtt

//**************** dht11***************************
#define DHTTYPE DHT11   //Definimos espesificamente que sensor DHT usamos en este caso es el DHT11

#define dht_dpin D6 // Define el pin a cual llegaran los datos del sensor dht11
DHT dht(dht_dpin, DHTTYPE); 
//*****************mq2****************************


int pinAout = A0; // Definimos el Pin A0 del micro para conectar pin Analogico del sensor de gas
int LPG, Co, Smoke; // Variables que puede medir el sensor de gas lpg, CO-gas y humo
MQ2 mq2(pinAout); //Declaramos la variable de mq2
int S=0;//Declaramos una variable que usaremos para armar el mensaje mqtt

//********************Pir****************************
int pir = D7; // Declaramos el pin de entrada para del microcontrolador para coectar el pin de datos den sensor de movimiento

//**********************Alerta************************

 int alerta = D8; //declaramos el pin D8 para la salida de la alerta



WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// *************************** Iniciar el WiFi *******************************
void setup_wifi() {

  delay(10);
  // Empezamos por conectarnos a una red WiFi
  Serial.println();
  Serial.print("Conectando a .....");
  Serial.println(ssid);

  WiFi.begin(ssid, password);// Esta es la función que realiz la conexión a WiFi

  while (WiFi.status() != WL_CONNECTED) // Este bucle espera a que se realice la conexión 
  {
    delay(500);
    Serial.print(".");// Indicador de progreso
  }

  randomSeed(micros());
  
  // Cuando se haya logrado la conexión, el programa avanzará, por lo tanto, puede informarse lo siguiente

  Serial.println("");
  Serial.println("Conectado a la red WiFi!");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

// Esta función permite tomar acciones en caso de que se reciba un mensaje correspondiente a un tema al cual se hará una suscripción

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido bajo el topico de [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

// establecemos las acciones a realizar del topico alerta
 if ((char)payload[0] == '0') // declaramos en 0 e estado del topico  
    digitalWrite(alerta, LOW);   // en caso de recibir un 0 la alerta no se activa
   else {
    digitalWrite(alerta, HIGH);  // en caso de recibir un un valo diferente de 0 se activa la alerta en este caso un 1
  }
 }
  
  


void reconnect() {
   // Bucle para reconectar a la red
  while (!client.connected()) {
    Serial.print("Intentando cnexion mqtt...");
    // Creamos un cliente al azar esp8266
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // intento de conexión
    if (client.connect(clientId.c_str())) {
      Serial.println("conexion mqtt exitosa");
      // Una vez conectado, publicar un anuncio...
      client.publish("salida", "hello world");
      // ... nos resubcribimos
      client.subscribe("alertas");
    } else {
      Serial.print("Fallo la conexion ");
      Serial.print(client.state());
      Serial.println(" Se intentara de nuevo en 5 segundos ");
      // esperamo 5 para conectarse de nuevo
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

   pinMode(alerta, OUTPUT); //Inicializar el pin alerta como salida
  
//**************dht11***************
  dht.begin();// Se inicializa las instrucciones del sesor dht
  delay(700);

 //*************mq2***********

 mq2.begin(); //Se inicializa las instrucciones del sesor dht
   pinMode(pinAout,INPUT); //Declaramos el modo de la variable pinAout como entrada

   //*****************PIR*************************
    pinMode(pir, INPUT);Declaramos el modo de la variable pir como entrada
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// Cuerpo del programa, bucle principal
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastMsg > 2000){
   lastMsg = millis();
   value++;
   String mes = String(value);
   mes.toCharArray(msg,50);
   client.publish ("Valor",msg);
   Serial.println ("Mensaje enviado->" + String(value));

   
// ************************************* dht11***************************************
   float h = dht.readHumidity(); //leemos el dato de la humedad
  float te = dht.readTemperature();   //leemos el dato de la temperatura
    Serial.print("Current humidity = ");//imprimiemos el mensaje qu esta entre comillas
    Serial.print(h);// imprimimos el dato de humedad
    Serial.print("%  ");//imprimiemos el mensaje qu esta entre comillas
    Serial.print("temperature = ");//imprimiemos el mensaje qu esta entre comillas
    Serial.print(te); //imprimiemos el dato de temperatura
    Serial.println("C  ");//imprimiemos el mensaje qu esta entre comillas
  delay(800);

// establecemos el mensaje a transmitir por mqtt de los datos del sensor dht11 en este caso humedad
  h;
   String hmes = String(h);
   hmes.toCharArray(msg,50);
   client.publish ("Humedad",msg);
   Serial.println ("Mensaje enviado->" + String(h));

// establecemos el mensaje a transmitir por mqtt de los datos del sensor dht11 en este caso temperatura
   te;
   String temes = String(te);
   temes.toCharArray(msg,50);
   client.publish ("Temperatura",msg);
   Serial.println ("Mensaje enviado->" + String(te));


   //***************************mq2********************************

   float* values= mq2.read(true); //Si se establece en "falso" el dato no aparecerá en el monitor serie
   
   Smoke = mq2.readSmoke();//leemos el dato de humo del sensor
   S=Smoke;// pasamos el datos la variable S para incorporar el dato al mensaje mqtt
   
   delay(1000);

   // establecemos el mensaje a transmitir por mqtt de los datos del sensor mq2
   S;
   String Smes = String(S);
   Smes.toCharArray(msg,50);
   client.publish ("Humo",msg);
   Serial.println ("Mensaje enviado->" + String(S));

   //**************************PIR*********************************
   int v = digitalRead(pir);// declaramos una variable para incorporarlo en el mensaje mqtt
  
// en la siguiente condicionante establecemo si hay movimiento o no segun los datos del sensor  
  if (v == HIGH) {
    Serial.println(1);
    }
  else {
    Serial.println(0);
    }
    
delay(100);

// establecemos el mensaje a transmitir por mqtt de los datos del sensor pir (movimiento)
 v;
   String vmes = String(v);
   vmes.toCharArray(msg,50);
   client.publish ("movimiento",msg);
   Serial.println ("Mensaje enviado->" + String(v));

  }

  }
