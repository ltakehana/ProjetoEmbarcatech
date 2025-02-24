#ifndef HARDWARE_SETUP_H
#define HARDWARE_SETUP_H

#include "pico/stdlib.h"
#include "config.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

// Função para configurar o buzzer
static inline void buzzer_setup() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_wrap(slice_num, 12500);
    pwm_set_clkdiv(slice_num, 4.0);
    pwm_set_gpio_level(BUZZER_PIN, 0);
    pwm_set_enabled(slice_num, false);
}


static inline void pwm_init_on_pin() {
    gpio_set_function(PWM_MOSFET_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_MOSFET_PIN);
    pwm_set_wrap(slice_num, 255);  // Wrap define a resolução
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PWM_MOSFET_PIN), 0);
    pwm_set_enabled(slice_num, true);
}

// Função para ativar o buzzer por 0.5 segundos
static inline void beep() {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, true);
    pwm_set_gpio_level(BUZZER_PIN, 250);
    sleep_ms(200);
    pwm_set_enabled(slice_num, false);
}

// Função para configurar o LED vermelho com PWM (50% de duty será definido posteriormente)
static inline void led_r_setup() {
    gpio_set_function(LED_R_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_R_PIN);
    pwm_set_wrap(slice_num, 255);           // Resolução de 8 bits (0 a 255)
    pwm_set_gpio_level(LED_R_PIN, 0);         // Inicialmente desligado
    pwm_set_enabled(slice_num, true);
}

// Função para inicializar todo o hardware
static inline void hardware_setup() {
    // Inicializa a UART para depuração
    stdio_init_all();

    // Configura o barramento I2C
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(OLED_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA_PIN);
    gpio_pull_up(OLED_SCL_PIN);

    // Configura os botões como entradas com pull-up
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_init(BUTTON_C_PIN);
    gpio_set_dir(BUTTON_C_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_C_PIN);

    // Inicializa o ADC e configura os pinos para o joystick
    adc_init();
    adc_gpio_init(JOYSTICK_VRX);
    adc_gpio_init(JOYSTICK_VRY);

    // Configura o buzzer
    buzzer_setup();

    // Configura o LED vermelho (usado para indicar load_on)
    led_r_setup();

    pwm_init_on_pin();
    
}

static inline void update_measured_current(double *measured_current) {
    adc_select_input(2);
    int raw = adc_read();
    double voltage = (raw * 3.3 / 4095.0);
    double current = voltage/0.15; 
    *measured_current = current;
}


#endif // HARDWARE_SETUP_H
