#include "pico/stdlib.h"  // Inclui a biblioteca padrão para trabaçhar com a raspberry pi pico.
#include <stdlib.h> // biblioteca padrão da linguagem C.
#include <stdio.h> // Inclui a biblioteca padrão de entrada/saída para funções como printf e scanf.
#include "hardware/pio.h" // Biblioteca para programar o PIO e utilizar a matriz de leds.
#include "hardware/clocks.h" // Inclui a biblioteca de hardware para configuração de clocks.
#include "hardware/adc.h"  // Inclui a biblioteca para controle do adc.
#include "inc/ssd1306.h" // Inclui a biblioteca para controle do display6.
#include "inc/font.h"  // Inclui a biblioteca de fontes para exibição de texto no display.

#include "projeto.pio.h" //inclusão do arquivo pio

// display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

//variáveis da Matriz de leds
#define NUM_PIXELS 25 //quantidade de leds na matriz
#define OUT_PIN 7 //gpio da matriz de leds

// Pinos leds e botões
#define sinal_verde 11
#define sinal_azul 12
#define sinal_vermelho 13
#define sensor_alto 5 // simula sensor para ativat o modo para o trânsito alto.
#define sensor_baixo 6 // simula sensor para ativat o modo para o trânsito baixo.
#define sensor_neutro 22 // simula sensor para ativat o modo para o trânsito notmal.

// Estados do trânsito
#define TRAFEGO_BAIXO 1
#define TRAFEGO_NEUTRO 2
#define TRAFEGO_ALTO 3

// variáveis para executar funções de interrupção e debouncing
volatile uint32_t last_time_A = 0;
volatile uint32_t last_time_B = 0;
volatile uint32_t last_time_N = 0;

int estado_atual = 2;// variável para mudar o estado de opereção do semáforo 


// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica o debounce para o sensor alto(butão A).
    if (gpio == sensor_alto && current_time - last_time_A > 200000) { // 200 ms de debounce
        last_time_A = current_time; // muda o último estado para atual    
        // Ação para o botão A
        if (!gpio_get(sensor_alto)  ) {
            estado_atual = TRAFEGO_ALTO; // Muda para o estado alto  
        }
    }
    // Verifica o debounce para o sensor baixo(botão B).
    if (gpio == sensor_baixo && current_time - last_time_B > 200000) {
        last_time_B = current_time; // muda o último estado para atual        
        // Ação para o botão B
        if (!gpio_get(sensor_baixo)) {
            estado_atual = TRAFEGO_BAIXO; // Muda para o estado baixo
        }
    }
    // Verifica o debounce para o sensor neuro.
    if (gpio == sensor_neutro && current_time - last_time_N > 200000) {
        last_time_N = current_time; // muda o último estado para atual        
        // Ação para o botão B
        if (!gpio_get(sensor_neutro)) {
            estado_atual = TRAFEGO_NEUTRO; // Muda para o estado neutro
}
    }
}

//definir intensidade do led
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

//Animação do Trânsito com fluxo normal.
void trafego_neutro(uint32_t valor_led, PIO pio, uint sm) {
    double frames[10][25][3] = {
        {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}}, 
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

      {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3}}, 
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
    };

    int delays[10] = {2000, 2000, 2000, 2000, 2000, 1500, 2000, 2000, 2000, 2000};// Delays entre as animação.

for (int frame = 0; frame < 10; frame++) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        double r = frames[frame][i][0];
        double g = frames[frame][i][1];
        double b = frames[frame][i][2];
        valor_led = matrix_rgb(r, g, b);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
    // Controla os leds do semáforo com base no frame
    if (frame == 0 || frame == 6 || frame == 7 || frame == 8 || frame == 9) {  
        gpio_put(sinal_verde, 1);   
        gpio_put(sinal_vermelho, 0);
        gpio_put(sinal_azul, 0);
    } else if (frame == 1) {
        gpio_put(sinal_verde, 1);
        gpio_put(sinal_vermelho, 1);
        gpio_put(sinal_azul, 0);    
    } else {
        gpio_put(sinal_verde, 0);
        gpio_put(sinal_vermelho, 1); 
        gpio_put(sinal_azul, 0);
    }

    sleep_ms(delays[frame]);  // Usa o delay específico para cada frame para sincronizar a ação do semáforo com a animação.
}
};

//Animação do Trânsito com fluxo baixo.
void trafego_baixo(uint32_t valor_led, PIO pio, uint sm) {
    double frames[10][25][3] = {
        {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}}, 
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

      {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}}, 
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
    };

    int delays[10] = {2000, 2000, 2000, 2000, 1500, 2000, 2000, 2000, 2000, 2000}; // Delays entre as animação.

for (int frame = 0; frame < 10; frame++) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        double r = frames[frame][i][0];
        double g = frames[frame][i][1];
        double b = frames[frame][i][2];
        valor_led = matrix_rgb(r, g, b);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
    // Controla os LEDs do semáforo com base no frame
    if (frame == 0 || frame == 1 || frame == 2 || frame == 3 || frame == 8 || frame == 9) {  
        gpio_put(sinal_verde, 1);   
        gpio_put(sinal_vermelho, 0);
        gpio_put(sinal_azul, 0);
    } else if (frame == 4) {
        gpio_put(sinal_verde, 1);
        gpio_put(sinal_vermelho, 1);
        gpio_put(sinal_azul, 0);    
    } else {
        gpio_put(sinal_verde, 0);
        gpio_put(sinal_vermelho, 1); 
    }

    sleep_ms(delays[frame]); // Usa o delay específico para cada frame para sincronizar a ação do semáforo com a animação.
}
};
//Animação do Trânsito com fluxo alto.            
void trafego_alto(uint32_t valor_led, PIO pio, uint sm) {
    double frames[11][25][3] = {
        {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
        {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

       {{0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.5, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.5}, {0.5, 0.0, 0.0}, {0.0, 0.3, 0.3}}, 
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.5, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.0, 0.5}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.5, 0.0}, {0.5, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

      {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}, {0.0, 0.3, 0.3}}, 
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
       
       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},

       {{0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, {0.0, 0.3, 0.3},
       {0.0, 0.3, 0.3}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.3, 0.3}},
    };

    int delays[11] = {2000, 2000, 2000, 2000, 2000, 2000, 2000, 1500, 2000, 2000, 2000}; // Delays entre as animação.

for (int frame = 0; frame < 11; frame++) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        double r = frames[frame][i][0];
        double g = frames[frame][i][1];
        double b = frames[frame][i][2];
        valor_led = matrix_rgb(r, g, b);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
    // Controla os LEDs do semáforo com base no frame
    if (frame == 9 || frame == 10 || frame == 11) {  
        gpio_put(sinal_verde, 0);   
        gpio_put(sinal_vermelho, 1 );
        gpio_put(sinal_azul, 0);
    } else if (frame == 8) {
        gpio_put(sinal_verde, 1);
        gpio_put(sinal_vermelho, 1);
        gpio_put(sinal_azul, 0);    
    } else {
        gpio_put(sinal_verde, 1);
        gpio_put(sinal_vermelho, 0); 
        gpio_put(sinal_azul, 0);
    }

    sleep_ms(delays[frame]); // Usa o delay específico para cada frame para sincronizar a ação do semáforo com a animação.
}

}
// Configura a clock
void inicializar_clock() {
    bool ok = set_sys_clock_khz(128000, false);
    if (ok) {
        printf("Clock set to %ld\n", clock_get_hz(clk_sys));
    } else {
        printf("Falha ao configurar o clock\n");
    }
}

// Configura a PIO
void configurar_pio(PIO pio, uint *offset, uint *sm) {
    *offset = pio_add_program(pio, &projeto_program);
    *sm = pio_claim_unused_sm(pio, true);
    projeto_program_init(pio, *sm, *offset, OUT_PIN);
}

void atualizar_display(ssd1306_t *ssd){  //função para atualizar o display de acordo com o estado atual do trânsito.
        // Limpa o display
        ssd1306_fill(ssd, 0);

    // Verifica o estado do trânsito e exibe a mensagem correspondente
    if (estado_atual == TRAFEGO_NEUTRO) {
        ssd1306_rect(ssd, 3, 3, 122, 58, 1, 0); // retângulo
        ssd1306_draw_string(ssd, "FLUXO NORMAL", 20, 30); 
    } 
    else if (estado_atual == TRAFEGO_ALTO) {
        ssd1306_rect(ssd, 3, 3, 122, 58, 1, 0); // retângulo
        ssd1306_draw_string(ssd, "FLUXO ALTO", 20, 30);
    } 
    else if (estado_atual == TRAFEGO_BAIXO) {
        ssd1306_rect(ssd, 3, 3, 122, 58, 1, 0); // retângulo
        ssd1306_draw_string(ssd, "FLUXO BAIXO", 20, 30); 
    }
        // Atualiza o display
        ssd1306_send_data(ssd);
    }

int main() {
    PIO pio = pio0;
    uint offset, sm;
    uint32_t valor_led;
    double r = 0.0, b = 0.0, g = 0.0;

    stdio_init_all();
    inicializar_clock();
    configurar_pio(pio, &offset, &sm);

    // inicia os pinos GPIOs dos leds
    gpio_init(sinal_verde);
    gpio_set_dir(sinal_verde, GPIO_OUT);

    gpio_init(sinal_azul);
    gpio_set_dir(sinal_azul, GPIO_OUT);

    gpio_init(sinal_vermelho);
    gpio_set_dir(sinal_vermelho, GPIO_OUT);

    gpio_init(sensor_alto);
    gpio_set_dir(sensor_alto, GPIO_IN);
    gpio_pull_up(sensor_alto); 

    gpio_init(sensor_baixo);
    gpio_set_dir(sensor_baixo, GPIO_IN);
    gpio_pull_up(sensor_baixo); 

    gpio_init(sensor_neutro);
    gpio_set_dir(sensor_neutro, GPIO_IN);
    gpio_pull_up(sensor_neutro); 
    
    // Inicia o I2C com 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    // Defini os pinos GPIO I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL); 
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
 
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configura a interrupção para os pinos dos botões
     gpio_set_irq_enabled_with_callback(sensor_alto, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
     gpio_set_irq_enabled_with_callback(sensor_baixo, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
     gpio_set_irq_enabled_with_callback(sensor_neutro, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

while (true) {
    atualizar_display(&ssd);
switch (estado_atual){

    case TRAFEGO_ALTO:
    trafego_alto(valor_led, pio, sm);
    break;

    case TRAFEGO_BAIXO:
    trafego_baixo(valor_led, pio, sm);
    break;
    
    case TRAFEGO_NEUTRO:
    trafego_neutro(valor_led, pio, sm);
    break;
default:
    break;
}
sleep_ms(100); // Adiciona um delay de 100 ms

}
}
