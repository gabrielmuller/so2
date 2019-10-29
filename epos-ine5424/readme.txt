# Simple cross-layer protocol

## Como testar

    make APPLICATION=port_test run

## Teste

O port_test envia 5 mensagens a 5 ports (25 pacotes no total), intercalando os
ports.

O port 0 simula uma perda total de pacote. O comportamento esperado é 
retransmitir três vezes e em seguida uma falha é detectada na aplicação.
O loop receive pula o port 0, já que nenhum pacote é recebido.

O port 1 simula um canal lento. O comportamento esperado é retransmitir uma ou
duas vezes até haver sucesso.

Os ports de 2 a 4 funcionam normalmente.
