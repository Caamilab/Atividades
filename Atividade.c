#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdlib.h>
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

//arquivo .pio
#include "Atividade.pio.h"

//número de LEDs
#define lin 5
#define col 5

//pino de saída
#define OUT_PIN 7

// Configurações dos pinos
const uint ledC_pin = 13; // Red => GPIO13
const uint ledA_pin = 12; // Blue => GPIO12
const uint ledB_pin = 11; // Green=> GPIO11
const uint button_0 = 6; // Botão A = 6
const uint button_1 = 5; // Botão B = 5

#define tempo 100

// Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

static int current_number = 0;  


static void gpio_irq_handler(uint gpio, uint32_t events);

// Estrutura para armazenar configurações da PIO
typedef struct {
    PIO pio;
    uint sm;
} PioConfig;

PioConfig config;

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

//rotina para acionar a matrix de leds - ws2812b
void desenho_pio(int numero){
    int R = 0;
    int G = 225;
    int B =0;
  for (int16_t i = lin - 1; i >= 0; i--) { // Varre as linhas de baixo para cima
    for (int16_t j = 0; j < col; j++) { // Colunas permanecem da esquerda para a direita
        int tempG = G * numeros[numero][i][j];
        uint32_t valor_led = (tempG << 24) | (R << 16) | (B << 8);
        pio_sm_put_blocking(config.pio, config.sm, valor_led);
    }
    }
}

int main()
{
    config.pio = pio0;
    config.sm = pio_claim_unused_sm(config.pio, true);

    set_sys_clock_khz(128000, false);

    // Configurações da PIO
    uint offset = pio_add_program(config.pio, &pio_matrix_program);
    pio_matrix_program_init(config.pio, config.sm, offset, OUT_PIN);

    // Inicializações
    stdio_init_all();
   
    gpio_init(ledA_pin);              // Inicializa o pino do LED
    gpio_set_dir(ledA_pin, GPIO_OUT); // Configura o pino como saída

    gpio_init(ledB_pin);              // Inicializa o pino do LED
    gpio_set_dir(ledB_pin, GPIO_OUT); // Configura o pino como saída

    gpio_init(ledC_pin);              // Inicializa o pino do LED
    gpio_set_dir(ledC_pin, GPIO_OUT); // Configura o pino como saída
                                      
    gpio_init(button_0); // Inicializa o botão
    gpio_set_dir(button_0, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(button_0);          // Habilita o pull-up interno

    gpio_init(button_1);
    gpio_set_dir(button_1, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(button_1);          // Habilita o pull-up interno


    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Loop principal
    while (true)
    {
        desenho_pio(current_number);
        gpio_put(ledC_pin, true);
        sleep_ms(tempo);
        gpio_put(ledC_pin, false);
        sleep_ms(tempo);
    }
}

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        last_time = current_time; // Atualiza o tempo do último evento
    
       
        if (gpio_get(button_0)) { // Botão para aumentar os números
            current_number++;
            if (current_number > 9) {
                current_number = 0; // Reinicia para 0 se passar de 9
            }
        } else if (gpio_get(button_1)) { // Botão para diminuir os números
            current_number--;
            if (current_number < 0) {
                current_number = 9; // Reinicia para 9 se passar de 0
            }
        }     
        desenho_pio(current_number);                         
    }
}