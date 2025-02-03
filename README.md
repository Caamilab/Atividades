
## **Descrição Geral**

Este programa foi desenvolvido para ser executado em uma Raspberry Pi Pico W e utiliza uma matriz de LEDs WS2812B para exibir números de 0 a 9. Isso ocorre conforme o uso de botões físicos para navegar entre os números, incrementando ou decrementando o valor exibido.
 Além disso, o programa utiliza um LED RGB que pisca continuamente 5 vezes por segundo, funcionando como um indicador visual do funcionamento do sistema.
---

## **Funcionamento**

### **1. Estrutura do Programa**
O código é dividido em várias seções principais:
- **Configuração de Hardware:** Define pinos GPIO para LEDs, botões e a matriz WS2812B.
- **Matrizes Numéricas:** Contém a representação visual dos números de 0 a 9 em uma matriz 5x5.
- **Controle do LED RGB:** Pisca continuamente com frequência de 5 Hz (5 vezes por segundo).
- **Interrupções:** Gerencia a interação com os botões, incluindo *debouncing* para evitar múltiplos acionamentos indesejados.
- **Controle da Matriz de LEDs:** Utiliza a interface PIO (Programável I/O) para controlar os LEDs WS2812B.

### **2. Componentes Utilizados**
- **Matriz de LEDs WS2812B:** Exibe os números.
- **Botões Físicos:**
  - Botão A (GPIO6): Incrementa o número exibido.
  - Botão B (GPIO5): Decrementa o número exibido.
- **LED RGB:**
  - Controlado pelos pinos GPIO13 (vermelho), GPIO12 (azul) e GPIO11 (verde).
  - Pisca continuamente com frequência de 5 Hz.

### **3. Fluxo do Programa**
1. Inicializa os pinos GPIO e configura o PIO para controlar a matriz de LEDs.
2. Configura interrupções nos botões para detectar cliques e alterar o número exibido.
3. No loop principal:
   - Atualiza a matriz de LEDs com base no número atual.
   - Controla o LED RGB para piscar continuamente com frequência de 5 Hz.

### **4. Interação com o Usuário**
- Pressionar o botão A incrementa o número exibido na matriz. Se o número atual for 9, ele retorna para 0.
- Pressionar o botão B decrementa o número exibido na matriz. Se o número atual for 0, ele retorna para 9.

### **5. Debouncing**
O programa implementa *debouncing* via software, ignorando múltiplos acionamentos dentro de um intervalo de 200 ms, garantindo uma operação mais estável.

---

## **Configuração do Hardware**

### **Pinos Utilizados**
| Componente         | Pino GPIO |
|--------------------|-----------|
| Matriz WS2812B     | GPIO7     |
| Botão A            | GPIO6     |
| Botão B            | GPIO5     |
| LED RGB Vermelho   | GPIO13    |
| LED RGB Azul       | GPIO12    |
| LED RGB Verde      | GPIO11    |

---
## Melhorias
Tratamento de concorrências para evitar possíveis condições de corrida.

## **Como Usar**

1. Compile o código utilizando o SDK do Raspberry Pi Pico.
2. Carregue o binário gerado na Raspberry Pi Pico W via USB.
3. Ligue a Raspberry Pi Pico W e interaja com os botões:
   - Pressione o botão A para aumentar o número exibido.
   - Pressione o botão B para diminuir o número exibido.

---

## **Video demonstrativo:**
https://youtu.be/tiZsCVg9-Tw?si=tkhR51iKKS4kqDDE
