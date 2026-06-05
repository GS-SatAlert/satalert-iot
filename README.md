# SatAlert: Sistema de Monitoramento de Queimadas e Desmatamento

## Integrantes do Grupo

| Nome | RM | Turma |
|------|----|-------|
| Andrei de Paiva Gibbini | 563061 | 2TDSPF |
| Pedro Sakai Silva Zambaca | 565956 | 2TDSPF |
| Pedro Santos Pequini | 561842 | 2TDSPF |
| Arthur Câmara | 562310 | 2TDSPG |
| Diogo Cunha | 563654 | 2TDSPF |


## 1. Descrição do Projeto
O **SatAlert** é uma solução integrada de monitoramento ambiental. Enquanto nossa plataforma principal consome dados de satélites (NASA/ESA) para uma análise macro, este protótipo **IoT (Internet das Coisas)** atua como uma estação terrestre de detecção precoce. Ele monitora condições críticas de temperatura e gases em pontos estratégicos, permitindo uma resposta imediata a focos de incêndio antes que sejam detectáveis via satélite.

## 2. Tecnologias e Arquitetura
O sistema foi desenvolvido seguindo os requisitos acadêmicos para sistemas embarcados e conectividade IoT:

- **Simulação:** Wokwi (ESP32).
- **Entradas:** - Sensor DHT22 (Temperatura).
    - Sensor MQ-2 (Detecção de fumaça/gases).
- **Saídas:**
    - LED de alerta (Sinalização visual).
    - Buzzer (Alerta sonoro).
- **Interface Local:** Display LCD 16x2 com controlador I2C.
- **Conectividade:** Protocolo MQTT (HiveMQ Cloud) sobre Wi-Fi.
- **Dashboard:** Node-RED para visualização de dados em tempo real.



## 3. Estrutura de Comunicação (MQTT Endpoints)
O sistema utiliza JSON para estruturar os dados, garantindo que o Dashboard e o Hardware falem a mesma língua:

| Tópico MQTT | Estrutura JSON (Exemplo) | Descrição |
| :--- | :--- | :--- |
| `satalert/telemetria` | `{"temperatura": 28.5, "nivel_fumaca": 12}` | Dados brutos dos sensores. |
| `satalert/alertas` | `{"foco_incendio": true, "risco": "CRITICO"}` | Estado de emergência. |
| `satalert/status` | `{"dispositivo": "SatAlert_01", "status": "online"}` | Verificação de conexão. |

## 4. Lógica de Funcionamento
1. **Coleta:** Leitura dos sensores via ESP32 a cada 2 segundos.
2. **Processamento:** O ESP32 normaliza os dados brutos ($0-4095$) para escala percentual utilizando a função `map()`.
3. **Tomada de Decisão:** Caso Temperatura $> 50^\circ C$ ou Fumaça $> 60\%$, o sistema ativa o LED e o Buzzer localmente e dispara o alerta via MQTT.
4. **Interface:** O Dashboard no Node-RED processa o status em tempo real, utilizando uma lógica de estados que garante que o alerta visual mude instantaneamente conforme as condições do ambiente.

## 5. Instruções de Instalação
### Hardware (Simulação Wokwi)
1. Importe os arquivos `sketch.ino`, `diagram.json` e `library.txt` para o [Wokwi](https://wokwi.com/).
    Ou se preferir entre no nosso projeto já pronto [SatAlert_Wokwi](https://wokwi.com/projects/465949480051578881).
2. Ajuste as variáveis `mqtt_server`, `mqtt_user` e `mqtt_pass` no arquivo `sketch.ino` com as credenciais do seu broker.

### Node-RED
1. Abra o Node-RED e selecione a opção **Importar**.
2. Cole o conteúdo do arquivo `flows.json`.
3. Configure os nós MQTT com o mesmo endereço do broker utilizado no ESP32.
4. Clique em **Deploy**.
5. Após o deploy, acesse a interface através do navegador:
`http://localhost:1880/ui`

---
*Projeto desenvolvido para o primeiro semestre da Global Solution da disciplina de [IoT].*
