#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
  
int receta[] = {1,4, 2,3, 2,2};
int motores[] = {0,1,2};
int pos_env[] = {1,4};
int *pos_rec;
int a = 7;
int f_tl;
int c_tl;
int f_pf;
int c_pf;

int Inicio = 1;

int filaPines[] = {18, 32, 33, 25};
int columnaPines[] = {26, 27, 14, 12};


String strs[20]; //si algo falla eso era strs[20]
int StringCount = 0;
      
TaskHandle_t RECEIVERWIFI;
TaskHandle_t PATHFIND;
TaskHandle_t BOTON;
TaskHandle_t SYSTEM;
xQueueHandle fila_q2tl;
xQueueHandle columna_q2tl;
xQueueHandle fila_q2pf;
xQueueHandle columna_q2pf;
xQueueHandle motors_q2m;

const char* ssid = "OnePlus 7T";
const char* password = "111222333";

Servo miServo; // Crear un objeto Servo para controlar el servo motor

boolean alreadyConnected = false;  // whether or not the client was connected previously
WiFiServer server(10000);  // server port to listen on


void connectWiFi(void) {
  Serial.printf("Connecting to %s\n", ssid);
  Serial.printf("\nattempting to connect to WiFi network SSID '%s' password '%s' \n", ssid, password);
  // attempt to connect to Wifi network:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(" listening on port 10000");
}

void receiverWiFi(void *parameter) { 
  while (1) {
    ledazul();
    static WiFiClient client;
    static int16_t seqExpected = 0;
    if (!client)
      client = server.available();  // Listen for incoming clients
    if (client) {                   // if client connected
      if (!alreadyConnected) {
        // clead out the input buffer:
        client.flush();
        Serial.println("We have a new client");
        alreadyConnected = true;
      }
      // if data available from client read and display it
      int length;
      String value;
      String str;
      if ((length = client.available()) > 0) {
        Serial.println("Received length: " + String(length));
        // if data is correct length read and display it

        //if (length == sizeof(value)) {
          //value = client.readBytes((char*)&value, sizeof(value));
          value = client.readString();
          Serial.println("[PRINT] " + value);
          //Serial.println("[PRINT] " + typeof(value));
          Serial.println("[PRINT] Length: " + String(value.length()));
          while (value.length() > 0)
          {
            int index = value.indexOf(' ');
            if (index == -1) // No space found
            {
              strs[StringCount++] = value;
              break;
            }
            else
            {
              strs[StringCount++] = value.substring(0, index);
              value = value.substring(index+1);
            }
          }

          for (int i = 0; i < StringCount; i = i + 2)
          {
            int fila = strs[i].toInt();
            int columna = strs[i+1].toInt();
            Serial.println("Fila: " + strs[i]);
            xQueueSend(fila_q2pf, (void *) &fila, (TickType_t) 10);
            Serial.println("Columna: " + strs[i + 1]);
            xQueueSend(columna_q2pf, (void *) &columna, (TickType_t) 10);
          }
          StringCount = 0;
          vTaskResume(PATHFIND);
          Serial.println("[QUEUE] " + String(uxQueueMessagesWaiting(fila_q2pf)));
          





        //} else {
        //  Serial.println("NEL");
        //  while (client.available()) Serial.print((char)client.read());  // discard corrupt packet
        //}
      }
    }
    
  }
}

void printWifiStatus(void) {
  // print the SSID of the network you're attached to:
  Serial.print("\nSSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void turnLEDs(void) {
  //while(1) {
    while (xQueueReceive(fila_q2tl, &(f_tl), (TickType_t) 1) != pdPASS) {
      //Serial.println("[TURNLEDS] Esperando Fila");
    }
    //Serial.println("[TURNLEDS] Fila Recibida");
    while (xQueueReceive(columna_q2tl, &(c_tl), (TickType_t) 1) != pdPASS) {
      //Serial.println("[TURNLEDS] Esperando Columna");
    }
    //Serial.println("[TURNLEDS] Columna Recibida");
    //Serial.println("[TURNLEDS] " + String(f_tl) + String(c_tl));
    int led[] = {f_tl,c_tl};

    int fila = f_tl - 1;
    int columna = c_tl - 1;
    digitalWrite(23, LOW);
    for (int i = 0; i < 4; i++) {
      digitalWrite(filaPines[i], i == fila ? LOW : HIGH);
      digitalWrite(columnaPines[i], i == columna ? HIGH : LOW);
    }
  //}
}



void pathFind(void *parameter) {
  while (1 ) {
    
    if (Inicio == 1) {
      xQueueReceive(fila_q2pf, &(f_pf), (TickType_t) 1);
      xQueueReceive(columna_q2pf, &(c_pf), (TickType_t) 1);
      Inicio = 0;
    }

    

    vTaskSuspend(RECEIVERWIFI);
    vTaskResume(SYSTEM);
    int pos_i[] = {0,0};
    int pos_f[] = {0,0};
    int pos_a[] = {0,0};
    while (xQueueReceive(fila_q2pf, &(f_pf), (TickType_t) 1) != pdPASS) {
      Serial.println("[PATHFINDER] Esperando Fila");
    }
    Serial.println("[PATHFINDER] Fila Recibida");
    while (xQueueReceive(columna_q2pf, &(c_pf), (TickType_t) 1) != pdPASS) {
      Serial.println("[PATHFINDER] Esperando Columna");
    }
    ledrojo();
    Serial.println("[DEBUG] FILA: " + String(f_pf));
    Serial.println("[DEBUG] Columna: " + String(c_pf));
    int pos[] = {f_pf, c_pf};

    Serial.println("[PATHFINDER][DESTINO] " + String(pos[0] + String(pos[1])));
    if ((pos_a[0] == 0) && (pos_a[1] == 0)) {
      ledrojo();
      while ((pos_a[0] != pos[0]) || (pos_a[1] != pos[1])) {
        Serial.println("[PATHFINDER][VOY] " + String(pos_a[0] + String(pos_a[1])));
        //Serial.println("[PATHFINDER][DEBUG] " + String(pos[0] + String(pos[1])));
        if ((pos_a[0] == 0) && (pos_a[1] == 0)) {
          Serial.println("00");
          ledamarillo();
        }

        else {
          xQueueSend(fila_q2tl, (void *) &pos_a[0], (TickType_t) 1);
          xQueueSend(columna_q2tl, (void *) &pos_a[1], (TickType_t) 1);
        }
        
        delay(500);

        if ((pos_a[0] == 0) && (pos_a[1] == 0)) {
          pos_a[0] = 4;
          pos_a[1] = 1;
          xQueueSend(motors_q2m, (void *) &motores[1], (TickType_t) 1);
        }

        else {
          if (pos_a[0] != pos[0]) {
            pos_a[0] = pos_a[0] - 1;
            xQueueSend(motors_q2m, (void *) &motores[1], (TickType_t) 1);
          }

          else {
            //Serial.println("[PATHFINDER][DEBUG] " + String(pos_a[0] + String(pos_a[1])));
            if (pos_a[1] != pos[1]) {
              pos_a[1] = pos_a[1] + 1;
              xQueueSend(motors_q2m, (void *) &motores[0], (TickType_t) 1);
            }
          }
        }
      }
      Serial.println("[PATHFINDER] Llegue " + String(pos_a[0] + String(pos_a[1])));
      if ((pos_a[0] == 0) && (pos_a[1] == 0)) {
        Serial.println("00");
        ledamarillo();
      }

      else {
        xQueueSend(fila_q2tl, (void *) &pos_a[0], (TickType_t) 1);
        xQueueSend(columna_q2tl, (void *) &pos_a[1], (TickType_t) 1);
      }
      //xQueueSend(servo_q2s, (void *) &motores[2], (TickType_t) 1);
      servo();
      delay(1500);
      ledrojo();
      while ((pos_a[0] != 0) && (pos_a[1] != 0)) {
        Serial.println("[PATHFINDER][VENGO] " + String(pos_a[0] + String(pos_a[1])));
        if ((pos_a[0] == 0) && (pos_a[1] == 0)) {
          Serial.println("00");
          ledamarillo();
        }

        else {
          xQueueSend(fila_q2tl, (void *) &pos_a[0], (TickType_t) 1);
          xQueueSend(columna_q2tl, (void *) &pos_a[1], (TickType_t) 1);
        }
        delay(500);

        if ((pos_a[0] == 4) && (pos_a[1] == 1)) {
          pos_a[0] = 0;
          pos_a[1] = 0;
          xQueueSend(motors_q2m, (void *) &motores[1], (TickType_t) 1);
        }

        else {
          if (pos_a[0] != 4) {
            pos_a[0] = pos_a[0] + 1;
            xQueueSend(motors_q2m, (void *) &motores[1], (TickType_t) 1);
          }

          else {
            if (pos_a[1] != 1) {
              pos_a[1] = pos_a[1] - 1;
              xQueueSend(motors_q2m, (void *) &motores[0], (TickType_t) 1);
            }
          }
        }
      }
      Serial.println("[PATHFINDER] Regrese " + String(pos_a[0] + String(pos_a[1])));
      if ((pos_a[0] == 0) && (pos_a[1] == 0)) {
        Serial.println("00");
        ledamarillo();
      }

      else {
        xQueueSend(fila_q2tl, (void *) &pos_a[0], (TickType_t) 1);
        xQueueSend(columna_q2tl, (void *) &pos_a[1], (TickType_t) 1);
      }
      //xQueueSend(servo_q2s, (void *) &motores[2], (TickType_t) 1);
      servo();
      delay(1500);
    }

    if ((uxQueueMessagesWaiting(fila_q2pf) == 0) && (uxQueueMessagesWaiting(columna_q2pf) == 0)) {
      if ((uxQueueMessagesWaiting(fila_q2tl) == 0) && (uxQueueMessagesWaiting(columna_q2tl) == 0)) {
        //vTaskSuspend(SYSTEM);
        vTaskResume(BOTON);
        vTaskSuspend(NULL);
      }  
    }

  }
}

void servo(void) {
    // Mover el servo de 0 a 180 grados en dos segundos
    for (int pos = 0; pos <= 180; pos++) {
      miServo.write(pos);
      delay(1); // Ajusta este valor según sea necesario
    }
    for (int pos = 180; pos >= 0; pos--) {
      miServo.write(pos);
      delay(1); // Ajusta este valor según sea necesario
    }
}

void motorfila(void) {
  digitalWrite(19, HIGH);
  delay(500);
  digitalWrite(19, LOW);
}

void motorcolumna(void) {
  digitalWrite(21, HIGH);
  delay(500);
  digitalWrite(21, LOW);
}

void boton(void *parameter) {
  while (1) {
    Inicio = 1;
    int valor = digitalRead(35);
    ledverde();
    Serial.println("Valor leído en pin 35: " + String(valor));
    int cont;
    if (valor == 1) {
      cont++;
    }

    else {
      cont = 0;
    }

    
    if (cont == 50) {
      vTaskSuspend(SYSTEM);
      vTaskResume(RECEIVERWIFI);
      vTaskSuspend(NULL);
    }
  }
}

void ledrojo(void) {
  digitalWrite(5, HIGH);
  digitalWrite(17, LOW);
  digitalWrite(16, LOW);
}
void ledazul(void) {
  digitalWrite(5, LOW);
  digitalWrite(17, HIGH);
  digitalWrite(16, LOW);
}
void ledverde(void) {
  digitalWrite(5, LOW);
  digitalWrite(17, LOW);
  digitalWrite(16, HIGH);
}

void ledamarillo(void) {
  digitalWrite(18, LOW);
  digitalWrite(32, LOW);
  digitalWrite(33, LOW);
  digitalWrite(25, LOW);
  digitalWrite(27, LOW);
  digitalWrite(14, LOW);
  digitalWrite(12, LOW);
  digitalWrite(26, LOW);

  digitalWrite(23, HIGH);
}

void system(void *parameter) {
  while (1) {
    int mot = 11;
    int serv = 0;
    turnLEDs();
    xQueueReceive(motors_q2m, &(mot), (TickType_t) 1);

    if (mot == 0) {
      motorfila();
    }

    else if (mot == 1) {
      motorcolumna();
    }

    if (serv == 2) {
      Serial.println("hola");
      //servo();
      //delay(1500);
    }

  }
}


void setup() {
  Serial.begin(115200);
  fila_q2tl = xQueueCreate(32, sizeof(int32_t));
  columna_q2tl = xQueueCreate(32, sizeof(int32_t));
  fila_q2pf = xQueueCreate(32, sizeof(int32_t));
  columna_q2pf = xQueueCreate(32, sizeof(int32_t));
  motors_q2m = xQueueCreate(32, sizeof(int32_t));

  connectWiFi();
  xTaskCreatePinnedToCore(receiverWiFi, "RECEIVERWIFI", 5000, NULL, 1, &RECEIVERWIFI, 1);

  xTaskCreatePinnedToCore(pathFind, "PATHFIND", 5000, NULL, 1, &PATHFIND, 0);
  vTaskSuspend(PATHFIND);
  xTaskCreatePinnedToCore(system, "SYSTEM", 5000, NULL, 1, &SYSTEM, 1);
  vTaskSuspend(SYSTEM);
  xTaskCreatePinnedToCore(boton, "BOTON", 5000, NULL, 1, &BOTON, 0);
  vTaskSuspend(BOTON);

  
  
  
  //xTaskCreatePinnedToCore(turnLEDs, "TURNLED", 5000, NULL, 1, &TURNLED, 1);

  // Configurar pines de salida para LEDs
  pinMode(16, OUTPUT); // verde
  pinMode(17, OUTPUT); // azul
  pinMode(5, OUTPUT);  // rojo
  pinMode(23, OUTPUT); // entrega (amarillo)

  pinMode(21, OUTPUT);  // motor columnas
  pinMode(19, OUTPUT); // motor filas

  // Configurar pines de salida para filas
  pinMode(18, OUTPUT); // fila 1
  pinMode(32, OUTPUT); // fila 2
  pinMode(33, OUTPUT); // fila 3
  pinMode(25, OUTPUT); // fila 4

  // Configurar pines de salida para columnas
  pinMode(26, OUTPUT); // columna 1
  pinMode(27, OUTPUT); // columna 2
  pinMode(14, OUTPUT); // columna 3
  pinMode(12, OUTPUT); // columna 4

  pinMode(35, INPUT);  // botón

  // Inicializar el objeto Servo
  miServo.attach(22); // servo en pin 22

  
}


void loop() {}
