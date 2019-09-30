# Projeto Final

**INE5424@UFSC**

**Autores:**
 - Arthur Bridi Guazzelli
 - Gabriel Müller
 - Juliana Silva Pinheiro


### 1. Driver de dispositivo de rede
---
> Usando como modelo a implementação do mediador de hardware do EPOS para a NIC PCNet32 da AMD, implemente um mediador (i.e. device driver) para uma NIC suportada pelo QEMU para a arquitetura Intel que ainda não exista para o EPOS. Cada grupo deve escolher uma NIC diferente (considere a família 8255x da Intel, que tem aproximadamente 10 variações implementadas pelo QEMU).  Projete e implemente uma aplicação de teste capaz de validar o correto funcionamento do mediador.

**NIC:** RTL8139

### 2. Protocolo crosslayer
---
> Projete um protocolo de comunicação crosslayer simples, que permita a propagação de eventos de rede, bem como de dados, entre a placa de rede (NIC no EPOS) e a API (Communicator no EPOS). Considere expansões futuras para a implementação de mecanismos de sincronização temporal e espacial. Implemente o protocolo projetado reutilizando o código entregue na etapa anterior do projeto final. Projete e implemente uma aplicação de teste capaz de validar o correto funcionamento do protocolo.


### 3. Sincronização temporal
---
> Projete um mecanismo de sincronização temporal para o protocolo de comunicação crosslayer entregue na etapa anterior do projeto final. Projete e implemente uma aplicação de teste capaz de validar o correto funcionamento do mecanismo integrado ao protocolo.

### 4. Sincronização espacial
---
> Projete um mecanismo de sincronização espacial para o protocolo de comunicação crosslayer entregue na etapa anterior do projeto final. Projete e implemente uma aplicação de teste capaz de validar o correto funcionamento do mecanismo integrado ao protocolo.
