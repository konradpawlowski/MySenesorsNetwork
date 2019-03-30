**The project is in development. You can see the status of this and other nodes from [my work panel](https://trello.com/b/6nWJ42Qx/smarthome) (In Spanish)**



### English  (Google Translate): 
-----------------------------------------------------

This circuit is a touch switch that replaces the wall switches and allows you to control the lighting (on and off, in the future I want to make another plate to regulate the brightness), uses two PCBs, the first one is black and includes the microcontroller, antenna, etc., the second PCB is red and includes the AC part, power supply, relays, etc.

**Features:**
- ATmega328P 3.3V 8Mhz microcontroller, with pads for external oscillator (optional)
- Antenna NRF24L01 SMD
- Touch push button TTP223 + HTTM light reflector to interact with the switch manually.
- RGB LED (The color blue does not have PWM) to indicate the status of the relays or RF connection.
- Flash chip for FOTA
- Chip ATSHA204A for signature.
- JST-8-1MM AVRISP + UART programming connector (UART requires 100nF capacitor in the cable)
- 2 relays of HF46F-5-HS1 of 5A to control AC load
- DS18B20 sensor near the power supply to control the internal temperature.
- Power supply HILINK HLK-PM01 5V
- Voltage regulator AMS1117-3.3

**Requirements:**
- It is necessary to connect the Neutral cable to the power supply. 
- It is necessary to print two pieces of plastic with a 3D printer to house the circuits, hold the glass and be able to screw the switch to the wall.

** Project status (OK / PENDING TO TEST / ERROR): **
- ATmega328P _OK_
- NRF24L01 _OK_
- RGB LED _OK_
- ISP Flash FOTA _OK_
- ATSHA204A _PENDING_
- Connector AVRISP + UART _OK_
- Reles _OK_
- DS18B20 _OK_
- HILINK HLK-PM01 5V _OK_
- AMS1117-3.3 _OK_

**Notes:**
- The data of the BOM tab may be obsolete.


### Spanish:
-----------------------------------------------------

Este circuito se trata de un interruptor táctil que sustituye los interruptores de pared y que permite controlar la iluminación (encendido y apagado, en el futuro quiero hacer otra placa para regular el brillo), utiliza dos PCB, el primero es de color negro e incluye el microcontrolador, antena, etc., el segundo PCB es de color rojo e incluye la parte AC, fuente de alimentación, relés, etc.

**Características:**
- Microcontrolador ATmega328P 3.3V 8Mhz, con pads para oscilador externo (opcional)
- Antena NRF24L01 SMD
- Pulsador táctil TTP223 + reflector de luz HTTM para interactuar con el interruptor manualmente.
- Led RGB (El color azul no tiene PWM) para indicar el estado de los relés o conexión RF.
- Chip flash para FOTA
- Chip ATSHA204A para firma.
- Conector de programación JST-8-1MM AVRISP+UART (UART requiere condensador de 100nF en el cable)
- 2 relés de HF46F-5-HS1 de 5A para controlar la carga AC
- Sensor DS18B20 cerca de la fuente de alimentación para controlar la temperatura interna.
- Fuente de alimentación HILINK HLK-PM01 5V
- Regulador de tensión AMS1117-3.3

**Requisitos:**
- Es necesario conectar el cable de Neutro para la fuente de alimentación.
- Es necesario imprimir dos piezas de plástico con una impresora 3D para albergar los circuitos, sujetar el cristal y poder atornillar el interruptor a la pared.

**Estado del proyecto (OK / PENDIENTE DE PROBAR / ERRORES):**
- ATmega328P _OK_
- NRF24L01 _OK_
- LED RGB _OK_
- ISP Flash FOTA _OK_
- ATSHA204A _PENDIENTE_
- Connector AVRISP+UART _OK_
- Reles _OK_
- DS18B20 _OK_
- HILINK HLK-PM01 5V _OK_
- AMS1117-3.3 _OK_

**Notas:**
- Los datos de la pestaña BOM pueden estar obsoletos.


### Changelog
-----------------------------------------------------

**2018/XX**

    + TOUCH SWITCH BOARD V2.2:
        - Improve logos and change position.

    + RELAY SWITCH BOARD V1.2:
        - Change AC connectors by ones at an angle (and modify the plastic shell with the new design)


**2017/12**

    + TOUCH SWITCH BOARD V2.1:
        - Move programming connector to the inside of the board, it was colliding with the plastic housing.
        - Modify the pinout of the RGB LED, the green and blue colors of the reverse.
        - Add text that identifies the functionality of the board.

    + RELAY SWITCH BOARD V1.1:
        - Replace SSR relay with 5A mechanical relays.
        - Add text that identifies the functionality of the board.


**2017/10 INITIAL VERSION**

    + TOUCH SWITCH BOARD V2.0.
    + RELAY SWITCH BOARD V1.0.