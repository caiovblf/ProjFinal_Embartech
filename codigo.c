// main.c - Raspberry Pi Pico, Cronômetro Pomodoro com LEDs indicadores, pausa e reset

#include <stdio.h>
#include "pico/stdlib.h"

// Definições das GPIO utilizadas no projeto
#define BTN_START 2        // Botão de início do cronômetro
#define BTN_PAUSE 6        // Botão para pausar e retomar o cronômetro
#define BTN_RESET 8        // Botão para resetar o cronômetro
#define LED_FUNCIONANDO 0  // LED azul que indica cronômetro ativo
#define LED_PAUSE 16       // LED verde que indica o cronômetro pausado
#define LED1 17            // LEDs amarelos que indicam progresso
#define LED2 18
#define LED3 19
#define LED4 20
#define LED5 21
#define BUZZER 22          // Buzzer para feedback sonoro

// Tempos definidos para trabalho e descanso (em segundos)
#define TEMPO_TRABALHO 1500
#define TEMPO_DESCANSO 300

// Variáveis globais que indicam estado do cronômetro
volatile bool pausado = false;
volatile bool resetado = false;

// Array que armazena os GPIOs dos LEDs amarelos para progresso
const int leds_progresso[5] = {LED1, LED2, LED3, LED4, LED5};

// Função para emitir um beep no buzzer (feedback sonoro)
void emitir_beep(int vezes) {
    for (int i = 0; i < vezes; i++) {
        gpio_put(BUZZER, true);
        sleep_ms(200);
        gpio_put(BUZZER, false);
        sleep_ms(200);
    }
}

// Função que aguarda o botão Start ser pressionado para iniciar
void esperar_botao() {
    while (gpio_get(BTN_START)) {
        sleep_ms(10);
    }
    sleep_ms(200);
}

// Função que verifica continuamente os botões de pausa e reset
void verificar_pausa_reset() {
    // Checa botão de pausa
    if (!gpio_get(BTN_PAUSE)) {
        pausado = !pausado;
        gpio_put(LED_PAUSE, pausado);
        emitir_beep(1);
        sleep_ms(300);
        while (!gpio_get(BTN_PAUSE)) sleep_ms(10);
    }
    // Checa botão de reset
    if (!gpio_get(BTN_RESET)) {
        resetado = true;
        emitir_beep(2);
        sleep_ms(300);
        while (!gpio_get(BTN_RESET)) sleep_ms(10);
    }
}

// Função que desliga todos os LEDs de progresso
void resetar_leds() {
    for (int i = 0; i < 5; i++) {
        gpio_put(leds_progresso[i], false);
    }
}

// Função principal para contar o tempo de trabalho e descanso
void contar_tempo(int segundos, bool descanso) {
    int intervalo_led = segundos / 5;
    resetar_leds();
    gpio_put(LED_FUNCIONANDO, true);

    for (int i = segundos; i > 0; i--) {
        int progresso = (segundos - i) / intervalo_led;

        // Ativa LEDs progressivamente; no descanso eles piscam
        for(int j = 0; j <= progresso && j < 5; j++) {
            gpio_put(leds_progresso[j], descanso ? (i % 2 == 0) : true);
        }

        // Verifica constantemente pausa e reset
        for(int j = 0; j < 10; j++) {
            verificar_pausa_reset();
            if (resetado) {  // Se resetado, desliga LEDs e sai da função
                gpio_put(LED_FUNCIONANDO, false);
                resetar_leds();
                return;
            }
            while (pausado && !resetado) {
                verificar_pausa_reset();
                sleep_ms(100);
            }
            sleep_ms(100);
        }
    }
    gpio_put(LED_FUNCIONANDO, false);
    resetar_leds();
}

// Função principal do programa
int main() {
    // Configuração inicial dos botões e LEDs
    gpio_init(BTN_START); gpio_set_dir(BTN_START, GPIO_IN); gpio_pull_up(BTN_START);
    gpio_init(BTN_PAUSE); gpio_set_dir(BTN_PAUSE, GPIO_IN); gpio_pull_up(BTN_PAUSE);
    gpio_init(BTN_RESET); gpio_set_dir(BTN_RESET, GPIO_IN); gpio_pull_up(BTN_RESET);

    gpio_init(LED_PAUSE); gpio_set_dir(LED_PAUSE, GPIO_OUT); gpio_put(LED_PAUSE, false);
    gpio_init(LED_FUNCIONANDO); gpio_set_dir(LED_FUNCIONANDO, GPIO_OUT); gpio_put(LED_FUNCIONANDO, false);

    for (int i = 0; i < 5; i++) {
        gpio_init(leds_progresso[i]); gpio_set_dir(leds_progresso[i], GPIO_OUT); gpio_put(leds_progresso[i], false);
    }

    gpio_init(BUZZER); gpio_set_dir(BUZZER, GPIO_OUT); gpio_put(BUZZER, false);

    // Loop principal infinito
    while (true) {
        resetado = false;
        esperar_botao();  // Espera o usuário iniciar

        emitir_beep(1);
        contar_tempo(TEMPO_TRABALHO, false);  // Ciclo de trabalho

        if (!resetado) {
            emitir_beep(2);
            contar_tempo(TEMPO_DESCANSO, true);  // Ciclo de descanso
        }

        emitir_beep(3);  // Finalização do ciclo
    }

    return 0;
}
