#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdlib.h>
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

// Arquivo do programa PIO
#include "Atividade.pio.h"

// Definições de constantes
#define LINHAS 5    // Número de linhas na matriz de LEDs
#define COLUNAS 5   // Número de colunas na matriz de LEDs
#define OUT_PIN 7   // Pino de saída para controle dos LEDs
#define TEMPO 100   // Tempo para piscar o LED (em ms)

// Configuração dos pinos GPIO
const uint LED_R_PIN = 13; // LED Vermelho => GPIO13
const uint LED_B_PIN = 12; // LED Azul => GPIO12
const uint LED_G_PIN = 11; // LED Verde => GPIO11
const uint BUTTON_INC = 6; // Botão para incrementar (A) => GPIO6
const uint BUTTON_DEC = 5; // Botão para decrementar (B) => GPIO5

// Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (para debouncing)
static int current_number = 0;          // Número atual exibido na matriz de LEDs

// Estrutura para configuração da PIO
typedef struct {
    PIO pio;
    uint sm;
} PioConfig;

PioConfig config;

// Protótipos das funções
void desenho_pio(int numero);
void gpio_irq_handler(uint gpio, uint32_t events);

// Definição das matrizes que representam os números de 0 a 9
int numeros[10][5][5] = {
    // Número 0
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 1
    {
        {0, 0, 1, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1}
    },
    // Número 2
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1}
    },
    // Número 3
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 4
    {
        {0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0}
    },
    // Número 5
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 6
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
     // Número 7
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0}
    },
    // Número 8
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 9
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    }
};


int main() {
    // Configuração inicial da PIO
    config.pio = pio0; // Seleciona o PIO0
    config.sm = pio_claim_unused_sm(config.pio, true); // Reivindica um State Machine disponível

    // Configuração do clock do sistema
    set_sys_clock_khz(128000, false); // Define a frequência do clock para 128 MHz

    // Carrega o programa PIO e inicializa o State Machine
    uint offset = pio_add_program(config.pio, &pio_matrix_program); // Adiciona o programa PIO
    pio_matrix_program_init(config.pio, config.sm, offset, OUT_PIN); // Inicializa o programa com o pino de saída

    // Inicializações gerais
    stdio_init_all(); // Inicializa a comunicação serial padrão

    // Configuração dos LEDs como saída
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);

    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);

    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Configuração dos botões como entrada com pull-up interno
    gpio_init(BUTTON_INC);
    gpio_set_dir(BUTTON_INC, GPIO_IN);
    gpio_pull_up(BUTTON_INC);

    gpio_init(BUTTON_DEC);
    gpio_set_dir(BUTTON_DEC, GPIO_IN);
    gpio_pull_up(BUTTON_DEC);

    // Configuração das interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_INC, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_DEC, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Loop principal do programa
    while (true) {
        desenho_pio(current_number);   // Atualiza a matriz de LEDs com o número atual
        gpio_put(LED_R_PIN, true);     // Liga o LED vermelho
        sleep_ms(TEMPO);               // Aguarda por TEMPO ms
        gpio_put(LED_R_PIN, false);    // Desliga o LED vermelho
        sleep_ms(TEMPO);               // Aguarda por TEMPO ms novamente
    }
}

// Função para desenhar um número na matriz de LEDs usando PIO
void desenho_pio(int numero) {
    int R = 0;   // Intensidade da cor vermelha (0)
    int G = 225; // Intensidade da cor verde (máxima)
    int B = 0;   // Intensidade da cor azul (0)

    for (int16_t i = LINHAS - 1; i >= 0; i--) {       // Varre as linhas de baixo para cima
        for (int16_t j = 0; j < COLUNAS; j++) {       // Varre as colunas da esquerda para a direita
            int tempG = G * numeros[numero][i][j];   // Calcula a intensidade do verde com base no número
            uint32_t valor_led = (tempG << 24) | (R << 16) | (B << 8); // Combina as cores em um único valor
            pio_sm_put_blocking(config.pio, config.sm, valor_led);     // Envia o valor para a PIO
        }
    }
}

// Função de interrupção com debouncing para tratar os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual em microssegundos

    if (current_time - last_time > 200000) { // Verifica se passaram mais de 200 ms desde o último evento (debouncing)
        last_time = current_time;           // Atualiza o tempo do último evento

        if (gpio_get(BUTTON_INC)) {         // Verifica se o botão de incremento foi pressionado
            current_number++;               // Incrementa o número atual
            if (current_number > 9) {
                current_number = 0;         // Reinicia para 0 se ultrapassar 9
            }
        } else if (gpio_get(BUTTON_DEC)) {  // Verifica se o botão de decremento foi pressionado
            current_number--;               // Decrementa o número atual
            if (current_number < 0) {
                current_number = 9;         // Reinicia para 9 se for menor que 0
            }
        }

        desenho_pio(current_number);        // Atualiza a matriz de LEDs com o novo número
    }
}