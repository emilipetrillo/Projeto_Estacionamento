// incluindo as bibliotecas
#include <PubSubClient.h> // MQTT
#include <SPI.h> // comunicacao
#include <UIPEthernet.h> // ethernet
#include <utility/logging.h> //ethernet
#include <Ultrasonic.h>

// declarando as variáveis
const int greenPin = 2; // colocar a porta
const int redPin = 4;
const int yellowPin = A1;
int distance;
int prevDistance;
int statusLedAtual;
int statusLedAnterior;


// setando as configuracoes do MQTT
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x14 };
const char* mqtt_server = "m12.cloudmqtt.com";
int port = 11528;

// setando as configurações do sensor de distnacia
Ultrasonic ultrasonic(7, 8);


// função callback - para receber as informações dos tópicos
void callback (char* topic, byte* payload, unsigned int length, boolean retained) {
  String topicStr = String(topic);
  Serial.print("Vagas disponíveis: ");
  Serial.print(topicStr);
  Serial.println();
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Muda o led que vai acender conforme a vaga estiver ou não vazia.
  // Se a vaga estiver vazia - acende led verde
  // seão acende a vermelha - ocupada
  if ((char)payload[0] == '1') {
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
    Serial.println("Vaga livre");
  } else if ((char)payload[0] == '0') {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    Serial.println("Vaga ocupada");
  }
}

EthernetClient ethClient;
PubSubClient mqttClient (mqtt_server, port, callback, ethClient);


void setup() {
  // seta os pinos leds
  pinMode (greenPin, OUTPUT);
  pinMode (redPin, OUTPUT);
  pinMode (yellowPin, OUTPUT);

  // tenta conexão com a internet
  Serial.begin(9600);
  while (!Serial) { }

  Serial.println("Tentando conexão via DHCP/IP...");
  //    Serial.println("teeeesssteee leeddd");
  //    digitalWrite(yellowPin, HIGH);
  //    delay(100);
  //    digitalWrite(yellowPin, LOW);
  //    delay(100);


  // tentando conectar via DHCP, se falhar tenta por IP
  if (!Ethernet.begin(mac)) {
    Serial.println("DHCP falhou");
  } else {
    Serial.print("Conectado no IP: ");
    Serial.println(Ethernet.localIP());
    delay(2000);
  }

  reconnectMQTT();
  /*
    // tentando conectar no MQTT
    if (mqttClient.connect("ClientId", "teste", "123", "Vaga", 0, true, "")) {
      // ClientID - nome unico
      // teste - nome do user no mqtt
      // 123 - senha
      // Vaga - nome do topico no mqtt
      // 0 - default
      // true - para ativar a função do retain
      // "" - apaga todas as mensagens quando for desligado abruptamente
      Serial.println("Conectado com MQTT");
      mqttClient.publish("Vaga", "1", true); // reportando vaga disponível. O true faz a função do retain
      //mqttClient.publish("TopicWill", "0", false); // replicando a msg do broker
      mqttClient.subscribe("Vaga");
      mqttClient.setCallback(callback);
      Serial.flush();
    } else {
      Serial.println("Não conectado com MQTT");
    }
  */
}

void loop() {

  Ethernet.maintain();
  // se perder a conexão com o MQTT tenta reconectar
  if (!mqttClient.connected()) {
    Serial.println("Conexão com MQTT perdida.");
    delay(2 * 1000);
    reconnectMQTT();
  }
  //  mqttClient.loop();


  // le a distancia
  distance = ultrasonic.distanceRead();
  Serial.print("Distancia Atual: ");
  Serial.println(distance);

  //  if (distance && (prevDistance > 6)) {
  // acende o led vermelho se a distancia for menor que 6,
  // acende o led ver se a distancia for maior ou igual a 6.
  if (distance >= 6) {
    digitalWrite(greenPin, 1);
    digitalWrite(redPin, 0);
    delay(100);
  } else {
    digitalWrite(greenPin, 0);
    digitalWrite(redPin, 1);
    delay(100);
  }
  statusLedAtual = digitalRead(greenPin);

  if (statusLedAtual != statusLedAnterior) {
    if (statusLedAtual == 1) {
      Serial.println("Vaga disponível");
      mqttClient.publish("Vaga", "1", true);
    } else {
      Serial.println("Vaga indisponível");
      mqttClient.publish("Vaga", "0", true);
    }
    statusLedAnterior = statusLedAtual;
    delay(100);
  }
  mqttClient.loop();
  //loop();
}



// essa função fica tentando se reconectar ao MQTT.
void reconnectMQTT() {
  digitalWrite(yellowPin, LOW);
  while (!mqttClient.connected()) {

    Serial.println("Tentando reconectar MQTT...");
    
    digitalWrite(yellowPin, HIGH);
    delay(100);
    digitalWrite(yellowPin, LOW);
    delay(100);
    digitalWrite(yellowPin, HIGH);
    delay(100);
    digitalWrite(yellowPin, LOW);
    delay(100);
    
    if (mqttClient.connect("ClientId", "teste", "123", "Vaga", 0, true, "")) {
      // ClientID - nome unico
      // teste - nome do user no mqtt
      // 123 - senha
      // Vaga - nome do topico no mqtt
      // 0 - default
      // true - para ativar a função do retain
      // "" - apaga todas as mensagens quando for desligado abruptamente
      Serial.println("Conectado com MQTT");
      Serial.println("Conectado");
      delay(2 * 1000);
      // envia via mqtt uma mensagem de conectado
      mqttClient.publish("Vaga", "1", true);
      mqttClient.subscribe("Vaga");
      mqttClient.setCallback(callback);
      Serial.flush();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println("Tentará reconecta em 2 segundos");
      delay(2 * 1000);
    }
  }
}









