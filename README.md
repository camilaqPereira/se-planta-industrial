<h1 align="center"><i>Sistema Integrado de Produção</i></h1>

<h3 align="center">Sistema embarcado para automação de uma planta industrial</h3>

<p align="center">O 'Sistema Integrado de produção' monitora a produção de blocos de madeira via Arduino Nano, integrando sensores e atuadores
de segurança para garantir uma operação segura e precisa.</p>

<h4 align="center">
<a href="https://github.com/camilaqPereira/se-planta-industrial/commits"> <img alt="se-planta-industrial commits" 
										  src="https://img.shields.io/github/last-commit/camilaqPereira/se-planta-industrial"></a>
<a href="https://github.com/camilaqPereira/se-planta-industrial/pulls"> <img alt="se-planta-industrial Pull Requests" 
										  src="https://img.shields.io/github/issues-pr/camilaqPereira/se-planta-industrial">
</a>
<a href="https://github.com/camilaqPereira/se-planta-industrial"><img alt="se-planta-industrial repo" 
								   src="https://img.shields.io/github/created-at/camilaqPereira/se-planta-industrial">
</a>
</h4>

## Sobre o projeto

<details>
  <summary>
    <h4> ⚙️ Periféricos utilizados </h4>  
</summary>
  
- 2 potênciometros de 10K;
- 2 _pushbuttons_;
- 1 sensor de temperatura LM35;
- 1 buzzer passivo
- 1 LED verde;
- 1 LED vermelho;
- 1 sensor de inclinação SW-520D;
- 1 servo motor;
- 2 motores CC;
- 2 displays de 7 segmentos;
- 1 LDR;
- 1 sensor ultrassônico HC-SR04;

</details>
<details>
  <summary> 
    <h4> ✔️ Funcionalidades </h4></summary>

  #### Supervisor
  - Botão de parada: interrupção e/ou retomada da produção a qualquer momento;
  - Controle da velocidade dos motores por meio de potenciômetros;
  - (**NÃO FUNCIONAL**) Atualização periódica do status da produção via monitor serial: a cada 3 segundos, as seguintes informações são exibidas no monitor
      - Status do Sensor de Temperatura
      - Status do Sensor de Inclinação
      - Status do Sensor de Presença
      - Status do Nível Tanque de Óleo
      - Status da produção
      - Velocidade dos motores
      - Quantidade de blocos cortados
  
  #### Chão de fábrica
  - Botão de parada: interrupção e/ou retomada da produção a qualquer momento;
  - Corte de blocos de madeira no tamanho 10cm x 25cm;
  - Contagem da quantidade de blocos cortados (**NÃO FUNCIONAL**);
  - Monitoramento da temperatura do sistema: faixa de operação 10°C a 40°C;
  - Monitoramento da orientação da madeira;
  - Monitoramento da prensença humana em torno da esteira;
  - Monitoramento do nível de óleo no tanque;
  - Notificação no monitor serial em caso de erros;
  - Notificação do supervisor via I2C em caso de erro.

  A descrição completa dos requisitos funcionais do sistema pode ser encontrada [aqui]().
</details>


## Softwares utilizados
<details><summary><b>Linguagem C</b></summary>

### Linguagem C

É uma linguagem de programação de propósito geral que combina abstrações e controles de baixo nível sobre o hardware resultando em ganho de eficiência. O software criado em 1970 por 
Dennis Ritchie é estreitamente associada ao sistema operacional UNIX, uma vez que as versões desse sistema foram escritas em linguagem C. Além disso, a sintaxe simples e a alta 
portabilidade desta linguagem entre dispositivos contribui para seu amplo uso em sistemas embarcados de recursos limitados.

</details>

<details><summary><b>Arduino IDE</b></summary>

### Arduino IDE
O [Arduino IDE](https://docs.arduino.cc/software/ide/) é um software de código aberto destinado a implementação, compilação e _upload_ de códigos em placas Arduino. Esta plataforma disponibiliza uma ampla
biblioteca de funções pré-definidas que simplificam o desenvolvimento de projetos, tornando-o ideal para iniciantes e desenvolvedores experientes experientes.
</details>

<details><summary><b>Wokwi</b></summary>

### Wokwi
O [Wokwi](https://wokwi.com/) é um simulador de eletrônica online que permite projetar, testar e depurar projetos de hardware e software em um ambiente virtual. Este simulador disponiliza
placas populares como Raspberry Pi Pico/W, Arduino e ESP32, além componentes eletrônicos e recursos avançados como analisador lógico e simulação de Wifi.

</details>

## Arduino Nano

<details><summary><b>Visão geral do Arduino Nano</b></summary></details>

Baseado no microcontrolador ATMega328p, o Arduino Nano é uma placa compacta, versátil e compatível com protoboards. Esta placa apresenta 14 pinos digitais, dos quais 6 podem ser 
configurados como saídas PWM, e 8 pinos analógicos. Além disso, possui comunicação serial, I2C e SPI.

</details>

<details><summary><b>Diagrama de pinos do Arduino Nano</b></summary></details>
![image](https://github.com/user-attachments/assets/6235cf3c-0075-49cd-8a4c-d6155d92b94f)
