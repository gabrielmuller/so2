# Sincronização temporal / espacial (entrega parcial)

## Como testar

    make APPLICATION=nmea_test run && python3 client.py

## Teste

O nmea_test envia um pedido e recebe uma mensagem NMEA pela porta serial, faz o
parse, atualiza a hora do relógio sem alterar a data e converte latitude /
longitude para coordenadas cartesianas.

Ainda falta desenvolver o protocolo de sincronização e a trilateração.
