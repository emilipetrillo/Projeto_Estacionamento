
// incluindo as bibliotecas
#include <PubSubClient.h> // MQTT
#include <SPI.h> // comunicacao
#include <UIPEthernet.h> // ethernet
#include <utility/logging.h> //ethernet
#include <Ultrasonic.h>

// declarando as variáveis
const int greenPin = 2; // colocar a porta
const int redPin = 4;
int distance;

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

  // tenta conexão com a internet
  Serial.begin(9600);
  while (!Serial) {}

  Serial.println("Tentando conexão via DHCP/IP...");

  // tentando conectar via DHCP, se falhar tenta por IP
  if (!Ethernet.begin(mac)) {
    Serial.println("DHCP falhou");
  } else {
    Serial.print("Conectado no IP: ");
    Serial.println(Ethernet.localIP());
    delay(2000);
  }

  // tentando conectar no MQTT
  if (mqttClient.connect("ClientId", "teste", "123", "TopicWill", 0, false, "0")) {
    Serial.println("Conectado com MQTT");
    mqttClient.publish("Vaga", "1", true); // reportando vaga disponível. O true faz a função do retain
    //mqttClient.publish("TopicWill", "0", false); // replicando a msg do broker
    mqttClient.subscribe("Vaga");
    mqttClient.setCallback(callback);
    Serial.flush();
  } else {
    Serial.println("Não conectado com MQTT");
  }
}

void loop() {
  // se perder a conexão com a internet tenta reconectar
  if (!ethClient.connected()) {
    Serial.println("Conexão com internet perdida");
    delay(2 * 1000);
    reconnectEth();
  }

  // se perder a conexão com o MQTT tenta reconectar
  if (!mqttClient.connected()) {
    Serial.println("Conexão com MQTT perdida.");
    delay(2 * 1000);
    reconnectMQTT();
  }
  //  mqttClient.loop();


  // le a distancia
  distance = ultrasonic.distanceRead();
  Serial.println(distance);

  // acende o led vermelho se a distancia for menor que 6,
  // acende o led ver se a distancia for maior ou igual a 6.
  if (distance >= 6) {
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
    delay(100);
  } else {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    delay(100);
  }
  mqttClient.loop();
}

// essa função fica tentando se reconectar a internet.
void reconnectEth() {
  //Ethernet.begin(mac);
  while (!ethClient.connected()) {
    Serial.println("Tentando reconectar internet");
    if (ethClient.available()) {
      //    Serial.println("Tentando reconectar internet");
      char c = ethClient.read();
      Serial.print(c);
      Serial.print("Conectado no IP: ");
      Serial.println(Ethernet.localIP());
      delay(2000);
    }

  }

  // essa função fica tentando se reconectar ao MQTT.
  void reconnectMQTT() {
    while (!mqttClient.connected()) {
      Serial.println("Tentando reconectar MQTT...");
      if (mqttClient.connect("clientId2", "teste", "123")) {
        Serial.println("Conectado");
        delay(2 * 1000);
        // envia via mqtt uma mensagem de conectado
        mqttClient.publish("Vaga", "Conectado!!! eeeeeee");
        mqttClient.subscribe("Vaga");
        delay(5000);
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println("Tentará reconecta em 2 segundos");
        delay(2 * 1000);
      }
    }
  }








