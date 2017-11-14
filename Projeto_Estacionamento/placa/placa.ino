// include the library code:
#include <LiquidCrystal.h>

//#include <Ethernet.h>
#include <SPI.h>
#include <UIPEthernet.h>
#include <utility/logging.h>
#include <PubSubClient.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int vagasDisponiveis = 0;
int vagasOcupadas = 0;
long tempoInicial;
int redpin = A0;    //Pin A0
int greenpin = 9;  //Pin 9
int bluepin = 8;    //Pin 8

int vagas[12] = {};
int msgComoNumero;
int index;

//// Atualizar ultimo valor para ID do seu Kit para evitar duplicatas
//byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x55 };
//// Endereço do Cloud MQTT
//char* server = "m12.cloudmqtt.com";
//// Valor da porta do servidor MQTT
//int port = 12397;

byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x55 };
int port = 1883;
IPAddress server(192, 168, 3, 186);
EthernetClient ethClient;


// FUNÇÃO que irá receber o retorno do servidor.
void whenMessageReceived(char* topic, byte* payload, unsigned int length) {
  // Converter pointer do tipo `byte` para typo `char`
  char* payloadAsChar = payload;

  // Converter em tipo String para conveniência
  String msg = String(payloadAsChar);
  Serial.print("Topic received: "); Serial.println(topic);

  // Dentro do whenMessageReceived (callback) da biblioteca MQTT,
  // devemos usar Serial.flush() para garantir que as mensagens serão enviadas
  Serial.flush();

  tempoInicial = millis(); // somente se quiser resetar o tempo
  lcd.display();

  msgComoNumero = msg.toInt();

  Serial.print("Numero lido: "); Serial.println(msgComoNumero);
  Serial.flush();

  String topicID = String(topic[12]);
  topicID += String(topic[13]);
  index = topicID.toInt();


  vagas[index] = msgComoNumero;
  
}

void atualizaVagas() {
  vagasOcupadas = 0;
  vagasDisponiveis = 0;

  for (int i = 0; i < 12; i++) {
    if (vagas[i] == 0) {
      vagasDisponiveis = vagasDisponiveis + 1;
    } else {
      vagasOcupadas = vagasOcupadas + 1;
    }

  }

  lcd.setCursor(12, 0);
  lcd.print("    ");
  lcd.setCursor(12, 0);
  lcd.print(vagasDisponiveis);
  lcd.setCursor(12, 1);
  lcd.print("    ");
  lcd.setCursor(12, 1);
  lcd.print(vagasOcupadas);
}


// Dados do MQTT Cloud
PubSubClient client(server, port, whenMessageReceived, ethClient);

void setup() {

  Serial.println("Pronto");

  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Iniciando...");

  pinMode(bluepin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(redpin, OUTPUT);


  Serial.println("Connecting...");

  while (!Serial) {}

  if (!Ethernet.begin(mac)) {
    Serial.println("DHCP Failed");
  } else {
    Serial.println(Ethernet.localIP());
  }

  // Faz a conexão no cloud com nome do dispositivo, usuário e senha respectivamente
  reconnect();

  lcd.setCursor(0, 0);
  lcd.print("Disponiveis:");
  lcd.setCursor(0, 1);
  lcd.print("Ocupadas   :");

  //atualizaVagas();
  Serial.println("VVVVVVVVVVVVVVVVVVVVVVVaaaaa");
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  // A biblioteca PubSubClient precisa que este método seja chamado em cada iteração de `loop()`
  // para manter a conexão MQTT e processar mensagens recebidas (via a função callback)
  client.loop();

  if (millis() >= (tempoInicial + 10000)) {
    lcd.noDisplay();
  }

  if (vagasDisponiveis == 0) {
    analogWrite(bluepin, 250);   //MAGENTA
    analogWrite(redpin, 255);
    //    analogWrite(greenpin, 9);
  } else {
    analogWrite(redpin, 0);
    analogWrite(greenpin, 0);
    analogWrite(bluepin, 0);
  }

  delay(100);
  atualizaVagas();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    //if (client.connect("id23", "codexpiot", "iot")) {
    if (client.connect("id23")) {
      //Conecta no topic para receber mensagens
      client.subscribe("vagas/+");
      //      client.subscribe("vaga");
      Serial.println("conectado topico");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 4 seconds");
      // Wait 5 seconds before retrying
      delay(4000);
    }
  }
}
