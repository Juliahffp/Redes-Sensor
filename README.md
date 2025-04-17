# Redes-Sensor
# 🌐 Trabalho Prático 2 - Redes de Sensores Sem Fio (RSSF)

## 📘 Descrição Geral

Este projeto simula uma **Rede de Sensores Sem Fio (RSSF)** utilizando **sockets POSIX** e o protocolo **TCP** na linguagem C, com foco em conceitos como comunicação em sistemas distribuídos, gerenciamento de vizinhança e atualização cooperativa de medições.

Inspirado em cenários reais como monitoramento ambiental e automação, o sistema reproduz uma rede onde sensores estão posicionados em uma malha 10x10 e comunicam-se entre si por meio de um servidor central. Cada sensor atua de forma autônoma, reportando medições periódicas e ajustando seus valores conforme dados recebidos de vizinhos próximos, respeitando um modelo _publish/subscribe_.

---

## 🎯 Objetivos

- Desenvolver um **servidor multithread** capaz de lidar com múltiplos clientes simultaneamente.
- Criar um **cliente sensor** que envia dados periódicos e atualiza medições com base na vizinhança.
- Implementar comunicação entre sensores do **mesmo tipo**, organizados por tópicos (_temperature_, _humidity_, _air_quality_).
- Simular o comportamento de sensores reais: entrada e saída da rede, medições dinâmicas, descoberta de vizinhos e atualização de valores com base em distância euclidiana.

---

## ⚙️ Funcionamento do Sistema

### Tipos de Sensores
- `temperature`: 20.0 ºC – 40.0 ºC (envio a cada 5s)
- `humidity`: 10.0 % – 90.0 % (envio a cada 7s)
- `air_quality`: 15.0 μg/m³ – 30.0 μg/m³ (envio a cada 10s)

Cada sensor inicia com uma medição aleatória dentro do intervalo. Após isso, passa a **corrigir** o valor com base em até **3 sensores mais próximos**.

### Atualização das Medições

Fórmula:
