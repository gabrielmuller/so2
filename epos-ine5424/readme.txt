# RTL8139 driver + comunicador NetService

## Como testar

    make APPLICATION=some_test run

Onde "some_test" é o nome do teste.

## Testes

Os testes disponíveis são:

* nic_test (adaptado): Envia 100 pacotes, com números de 0 a 9, 10 vezes.

* interop_test: Envia 200 pacotes, metade é lixo IP. Apenas os 100 pacotes relevantes são recebidos.

* dma_test: Compara o tempo de execução de uma aplicação CPU-bound fazendo ou não transferências DMA.

* overflow_test: Executa o nic_test junto com uma thread que desabilita interrupções periodicamente, forçando overflows RX e TX.

A comunicação entre QEMUs e as interrupções são observadas em todos os testes.
Todos testes usam o comunicador NetService criado.
