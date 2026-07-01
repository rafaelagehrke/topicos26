# Monitor BLE com ESP32 e Flutter

## Disciplina
Programação para Dispositivos Móveis

## Descrição

Este projeto consiste no desenvolvimento de um sistema de monitoramento ambiental baseado no microcontrolador ESP32 utilizando Bluetooth Low Energy (BLE) para comunicação com um aplicativo Android desenvolvido em Flutter.

O sistema realiza a aquisição de temperatura e umidade através de um sensor DHT11, exibe as informações localmente em um display LCD 16×2 e disponibiliza esses dados em tempo real para um aplicativo móvel via Bluetooth Low Energy.

Além do monitoramento ambiental, o sistema permite o controle remoto de LEDs, gerenciamento do histórico dos sensores, visualização de gráficos em tempo real e acompanhamento do estado da conexão BLE.

---

# Objetivos

O projeto possui como objetivos:

- Desenvolver um servidor BLE utilizando ESP32;
- Desenvolver um aplicativo Android utilizando Flutter;
- Realizar aquisição de temperatura e umidade utilizando DHT11;
- Exibir informações em display LCD;
- Disponibilizar informações via Bluetooth Low Energy;
- Controlar LEDs remotamente através do aplicativo;
- Exibir gráficos das leituras dos sensores;
- Monitorar a intensidade do sinal BLE (RSSI);
- Registrar histórico dos sensores;
- Implementar interação utilizando botões e switches do kit.

---

# Arquitetura do Sistema

O sistema é dividido em duas partes principais.

## Firmware ESP32

Responsável por:

- leitura do sensor DHT11;
- gerenciamento do display LCD;
- gerenciamento dos LEDs;
- controle dos botões e switches;
- armazenamento do histórico;
- disponibilização dos serviços BLE.

## Aplicativo Flutter

Responsável por:

- localizar dispositivos BLE;
- conectar ao ESP32;
- receber notificações BLE;
- apresentar gráficos;
- controlar LEDs;
- redefinir histórico;
- monitorar RSSI;
- exibir quantidade de notificações BLE.

---

# Arquitetura Geral

```text
                +----------------------+
                |      Aplicativo      |
                |       Flutter        |
                +----------+-----------+
                           |
                     Bluetooth LE
                           |
                +----------+-----------+
                |        ESP32         |
                +----------+-----------+
                           |
        +------------------+------------------+
        |                  |                  |
     Sensor DHT11        Display LCD       LEDs
```

---

# Componentes Utilizados

| Componente | Descrição |
|------------|-----------|
| ESP32 Dev Module | Microcontrolador principal |
| DHT11 | Sensor de temperatura e umidade |
| LCD 16×2 I2C | Exibição local das informações |
| LED Verde | Atuador |
| LED Vermelho | Atuador |
| LED Bicolor | Atuador |
| Push Button | Troca manual de telas |
| Push Button | Reset do histórico |
| Switches | Controle de funcionalidades |
| Smartphone Android | Cliente BLE |

---

# Ligações de Hardware

| Componente | GPIO |
|------------|-----:|
| DHT11 | 26 |
| LCD SDA | 22 |
| LCD SCL | 23 |
| Push Button Tela | 33 |
| Push Button Reset | 25 |
| LED Verde | 5 |
| LED Vermelho | 18 |
| LED Bicolor Verde | 13 |
| LED Bicolor Vermelho | 19 |
| SW1 | 16 |
| SW2 | 4 |
| SW3 | 2 |
| SW4 | 15 |

---

# Funcionalidades Implementadas

## Display LCD

O display LCD apresenta cinco telas distintas:

1. Temperatura em Celsius e Umidade;
2. Temperatura em Fahrenheit e Umidade;
3. Temperatura mínima e máxima;
4. Umidade mínima e máxima;
5. Estado da conexão Bluetooth.

As telas são alternadas automaticamente a cada três segundos, podendo também ser alteradas manualmente através do Push Button.

---

## Sensores

O sensor DHT11 realiza a leitura periódica da temperatura e umidade.

As leituras são utilizadas para:

- atualização do LCD;
- envio via BLE;
- geração dos gráficos do aplicativo;
- atualização do histórico de valores mínimos e máximos.

---

## Histórico

O sistema mantém:

- temperatura mínima;
- temperatura máxima;
- umidade mínima;
- umidade máxima.

O histórico pode ser reiniciado de duas formas:

- através do Push Button do circuito;
- através do aplicativo Flutter utilizando Bluetooth.

---

## Comunicação Bluetooth Low Energy

O ESP32 atua como servidor BLE disponibilizando serviços responsáveis pelo envio das informações dos sensores e pelo controle remoto dos dispositivos conectados.

A comunicação utiliza notificações para envio contínuo dos dados, permitindo atualização praticamente em tempo real da interface do aplicativo.
