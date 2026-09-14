//Autor: Fábio Henrique Cabrini
//Resumo: Esse programa possibilita ligar e desligar o led onboard, além de mandar o status para o Broker MQTT possibilitando o Helix saber
//se o led está ligado ou desligado.
//Revisões:
//Rev1: 26-08-2023 Código portado para o ESP32 e para realizar a leitura de luminosidade e publicar o valor em um tópico aprorpiado do broker 
//Autor Rev1: Lucas Demetrius Augusto 
//Rev2: 28-08-2023 Ajustes para o funcionamento no FIWARE Descomplicado
//Autor Rev2: Fábio Henrique Cabrini
//Rev3: 1-11-2023 Refinamento do código e ajustes para o funcionamento no FIWARE Descomplicado
//Autor Rev3: Fábio Henrique Cabrini
// Biblioteca para conectar o ESP32 a uma rede Wi-Fi.
#include <WiFi.h>
// Biblioteca para publicar e receber mensagens pelo protocolo MQTT.
#include <PubSubClient.h>

// ========================= CONFIGURACOES =========================
// Dados da rede Wi-Fi utilizada pelo projeto. No Wokwi, Wokwi-GUEST
// normalmente funciona sem senha.
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi

// Endereco e porta do broker MQTT que recebe os dados da lampada.
const char* default_BROKER_MQTT = ""; // IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT

// Topico no qual o ESP32 escuta os comandos de ligar e desligar.
const char* default_TOPICO_SUBSCRIBE = "/TEF/lamp200/cmd"; // Tópico MQTT de escuta

// Topicos usados para publicar o estado da lampada e a luminosidade medida.
const char* default_TOPICO_PUBLISH_1 = "/TEF/lamp200/attrs"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_2 = "/TEF/lamp200/attrs/l"; // Tópico MQTT de envio de informações para Broker

// Identificador unico usado pelo ESP32 ao se conectar ao broker MQTT.
const char* default_ID_MQTT = "fiware_200"; // ID MQTT

// Pino do LED onboard do ESP32 e prefixo usado no formato dos comandos.
const int default_D4 = 2; // Pino do LED onboard
const char* topicPrefix = "lamp200";

// Copias editaveis das configuracoes acima. Elas centralizam os valores
// usados pelas funcoes de Wi-Fi, MQTT e controle do LED.
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int D4 = default_D4;

// Cliente de rede usado como transporte para o MQTT.
WiFiClient espClient;
// Cliente MQTT configurado para usar a conexao Wi-Fi do ESP32.
PubSubClient MQTT(espClient);
// Estado atual do LED: '1' significa ligado e '0' significa desligado.
char EstadoSaida = '0';

// Inicia a comunicacao serial para exibir informacoes no monitor serial.
void initSerial() {
    Serial.begin(115200);
}

// Exibe mensagens de inicio e inicia a conexao com a rede Wi-Fi.
void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

// Define o broker MQTT e registra a funcao que trata mensagens recebidas.
void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

// Funcao executada uma vez ao iniciar o ESP32.
void setup() {
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH_1, "s|on");
}

// Funcao executada continuamente durante o funcionamento do ESP32.
void loop() {
    VerificaConexoesWiFIEMQTT();
    EnviaEstadoOutputMQTT();
    handleLuminosity();
    MQTT.loop();
}

// Conecta o ESP32 ao Wi-Fi caso ele ainda nao esteja conectado.
void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    // Garante que o LED comece desligado depois que a rede estiver pronta.
    digitalWrite(D4, LOW);
}

// Processa cada comando recebido no topico MQTT de inscricao.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // Converte o payload recebido, que chega como bytes, para uma String.
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    // Monta os comandos esperados no padrao definido pelo projeto.
    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    // Liga o LED quando o comando recebido for lamp200@on|.
    if (msg.equals(onTopic)) {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }

    // Desliga o LED quando o comando recebido for lamp200@off|.
    if (msg.equals(offTopic)) {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}

// Verifica e recupera as conexoes Wi-Fi e MQTT quando necessario.
void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}

// Publica no broker o estado atual do LED a cada ciclo de atualizacao.
void EnviaEstadoOutputMQTT() {
    if (EstadoSaida == '1') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|on");
        Serial.println("- Led Ligado");
    }

    if (EstadoSaida == '0') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|off");
        Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}

// Configura o pino do LED como saida e faz uma breve sinalizacao visual
// alternando o LED durante a inicializacao.
void InitOutput() {
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    boolean toggle = false;

    for (int i = 0; i <= 10; i++) {
        toggle = !toggle;
        digitalWrite(D4, toggle);
        delay(200);
    }
}

// Tenta conectar ao broker MQTT ate obter sucesso. Depois da conexao,
// inscreve o ESP32 no topico que recebe os comandos da lampada.
void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

// Le o sensor de luminosidade conectado ao pino analogico 34, converte
// a leitura para uma escala de 0 a 100 e publica o resultado via MQTT.
void handleLuminosity() {
    // O LDR esta ligado a entrada analogica 34 do ESP32.
    const int potPin = 34;
    int sensorValue = analogRead(potPin);
    // O ADC do ESP32 varia de 0 a 4095; a escala publicada vai de 0 a 100.
    int luminosity = map(sensorValue, 0, 4095, 0, 100);
    String mensagem = String(luminosity);
    Serial.print("Valor da luminosidade: ");
    Serial.println(mensagem.c_str());
    MQTT.publish(TOPICO_PUBLISH_2, mensagem.c_str());
}
