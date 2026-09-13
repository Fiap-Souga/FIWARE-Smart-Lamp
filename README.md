# FIWARE Smart Lamp

Projeto de uma lampada inteligente com ESP32, controle por MQTT e leitura de luminosidade. A simulacao pode ser executada no [Wokwi](https://wokwi.com/).

## Integrantes

- Eduardo Menzel - RM 570405
- Guilherme Kooji Kubota - RM 570541
- Luís Carlos - RM 569214
- Victor da Silva Souto - RM 571093

## Objetivo

O ESP32 controla o LED onboard como uma lampada e disponibiliza seu estado para um broker MQTT. O projeto tambem le a luminosidade por meio de um sensor LDR e publica o valor para integracao com o FIWARE.

## Componentes

- ESP32 DevKit v1
- Sensor de luminosidade (LDR/photoresistor)
- LED onboard do ESP32
- Broker MQTT

## Funcionamento

1. O ESP32 conecta-se a rede Wi-Fi configurada no sketch.
2. Em seguida, conecta-se ao broker MQTT e assina o topico de comandos.
3. Ao receber `lamp200@on|`, liga o LED onboard.
4. Ao receber `lamp200@off|`, desliga o LED onboard.
5. O estado do LED e publicado periodicamente.
6. A leitura do LDR e convertida para uma escala de 0 a 100 e publicada no MQTT.

## Topicos MQTT

| Finalidade | Topico | Exemplo |
| --- | --- | --- |
| Receber comandos | `/TEF/lamp200/cmd` | `lamp200@on|` |
| Publicar estado | `/TEF/lamp200/attrs` | `s|on` ou `s|off` |
| Publicar luminosidade | `/TEF/lamp200/attrs/l` | `0` a `100` |

## Ligacoes do circuito

| Componente | ESP32 |
| --- | --- |
| Saida analogica do LDR (AO) | GPIO 34 |
| VCC do LDR | 3V3 |
| GND do LDR | GND |
| LED onboard | GPIO 2 |

As ligacoes completas estao no arquivo [`diagram.json`](diagram.json).

## Como executar no Wokwi

1. Abra o projeto no Wokwi usando o arquivo `diagram.json`.
2. Confira as configuracoes de Wi-Fi e MQTT no inicio de [`sketch.ino`](sketch.ino).
3. Instale a biblioteca `PubSubClient`, listada em [`libraries.txt`](libraries.txt), caso necessario.
4. Inicie a simulacao e acompanhe as mensagens pelo monitor serial.

## Configuracao

Antes de usar uma rede ou broker diferente, altere estas constantes em `sketch.ino`:

- `default_SSID`: nome da rede Wi-Fi;
- `default_PASSWORD`: senha da rede Wi-Fi;
- `default_BROKER_MQTT`: endereco do broker MQTT;
- `default_BROKER_PORT`: porta do broker;
- topicos MQTT e `default_ID_MQTT`, se a infraestrutura exigir outros valores.

## Historico

- **Rev1 - 26/08/2023:** codigo portado para ESP32 e adicionada a leitura de luminosidade.
- **Rev2 - 28/08/2023:** ajustes para funcionamento com o FIWARE Descomplicado.
- **Rev3 - 01/11/2023:** refinamento do codigo e novos ajustes de integracao.