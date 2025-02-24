#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware.h"
#include "ssd1306.h"
#include "menu.h"
#include "config.h"
#include "pico/multicore.h"

ssd1306_t oled;

// Setpoints e medições (valores exemplo)
static double current_setpoint = 2.5;   // Setpoint de corrente (MENU_CC)
static double measured_current = 2.5;

// Variáveis para o controlador PID
static float integral = 0.0f;
static float last_error = 0.0f;
static uint32_t last_time = 0;
static uint8_t pid_output = 0;

// Estado de carga: indica se a carga está ligada (load_on)
static bool load_on = false;

uint8_t pid_controller(float setpoint, float measurement) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    float dt = (current_time - last_time) / 1000.0f; // dt em segundos
    if (dt <= 0) dt = 0.001f;  // Evita divisão por zero

    float error = setpoint - measurement;
    integral += error * dt;
    float derivative = (error - last_error) / dt;
    
    float output = PID_KP * error + PID_KI * integral + PID_KD * derivative;
    
    // Limita a saída para a faixa do PWM (0 a 255)
    if (output < 0) output = 0;
    if (output > 255) output = 255;
    
    last_error = error;
    last_time = current_time;
    
    return (uint8_t)output;
}

// Thread responsável pela leitura dos sensores e controle PID
void sensor_thread() {
    while (1) {
        // Atualiza as medições de tensão e corrente
        update_measured_current(&measured_current);

        // Executa o PID apenas se load_on estiver ativo
        if (load_on) {
            // Verifica o modo definido no menu: MENU_CV ou MENU_CC[]
            pid_output = pid_controller(current_setpoint, measured_current);
        } else {
            // Se a carga estiver desligada, zera o PID
            integral = 0.0f;
            last_error = 0.0f;
            pid_output = 0;
        }
        
        // Atualiza o nível do PWM com base na saída do PID
        pwm_set_gpio_level(PWM_MOSFET_PIN, pid_output);

        sleep_ms(1);  // Intervalo de atualização
    }
}


// Thread responsável pelo menu, botões e leitura do joystick
void menu_thread() {
    // Variável para controle da frequência de impressão
    uint32_t last_print_time = 0;

    while (1) {
        // Leitura do joystick (e atualização dos setpoints)
        adc_select_input(0);
        int16_t vrx = adc_read();
        adc_select_input(1);
        int16_t vry = adc_read();

        // Centraliza os valores (valor médio ≈ 2048)
        int16_t vrx_offset = vrx - 2048;
        int16_t vry_offset = vry - 2048;

        // Atualiza os setpoints com base no movimento do joystick
        menu_update_joystick(vrx_offset, vry_offset, &current_setpoint);

        // --- Processa Botões ---
        // Botão A: alterna estado load_on (liga/desliga carga)
        if (!gpio_get(BUTTON_A_PIN)) {  // Botão A pressionado (ativo em LOW)
            load_on = !load_on;         // Alterna o estado load_on

            // Atualiza o LED vermelho com 50% do PWM se load_on estiver ativo; caso contrário, desliga o LED
            if (load_on) {
                pwm_set_gpio_level(LED_R_PIN, 60);  // Aproximadamente 50% de 255
            } else {
                pwm_set_gpio_level(LED_R_PIN, 0);
            }
            sleep_ms(200); // Debounce
        }

        // Botão C: confirmar seleção
        if (!gpio_get(BUTTON_C_PIN)) {
            menu_button_c_pressed();
            sleep_ms(200); // Debounce
        }

        // Botão B: sair/cancelar
        if (!gpio_get(BUTTON_B_PIN)) {
            menu_button_b_pressed();
            sleep_ms(200);
        }

        // Atualiza a tela do menu com os setpoints e medições
        menu_update(&oled, &current_setpoint, &measured_current);

        // Imprime os valores a cada 1 segundo
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_print_time >= 1000) {
            last_print_time = now;
            printf("Setpoint Corrente = %.2f A; Medida Corrente = %.2f A; Ativo = %d; Modo: %s; PWM = %d\n",
                   current_setpoint, measured_current, load_on,
                    "CC", pid_output);
        }
        
        sleep_ms(100);  // Intervalo de atualização do menu
    }
}


int main() {
    // Configuração do hardware (GPIOs, ADC, PWM, I2C, etc.)
    hardware_setup();

    // Inicializa o display OLED
    if (!ssd1306_init(&oled, SCREEN_WIDTH, SCREEN_HEIGHT, OLED_ADDR, I2C_PORT)) {
        sleep_ms(20000);
        printf("Erro ao inicializar SSD1306!\n");
        return -1;
    }

    // Inicializa o sistema de menus (passa as referências para as variáveis compartilhadas)
    menu_init(&oled, &measured_current, &current_setpoint);

    // Lança a thread do menu (Core1)
    multicore_launch_core1(menu_thread);

    // Executa a thread de sensores (Core0)
    sensor_thread();

    return 0;
}
