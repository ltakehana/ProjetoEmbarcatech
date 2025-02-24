#ifndef CONFIG_H
#define CONFIG_H

// ==================== CONFIGURAÇÕES DE GPIO ====================
// ⚡ Medição de corrente e tensão
#define ADC_PIN          28   // Entrada analógica (multiplexada)

// 🔘 Botões
#define BUTTON_A_PIN  5   // Liga/desliga carga
#define BUTTON_B_PIN  6   // Sair/cancelar
#define BUTTON_C_PIN  22  // Confirmar seleção

// 🎮 Joystick (Analógico)
#define JOYSTICK_VRX  26  // Navegação no menu
#define JOYSTICK_VRY  27  // Ajuste de parâmetros

// 🖥️ Display OLED (I²C)
#define OLED_SDA_PIN  14  // SDA do I2C
#define OLED_SCL_PIN  15  // SCL do I2C
#define OLED_ADDR 0x3C  // Endereço I2C do SSD1306
// Definições para I2C (usando as constantes definidas em config.h)
#define I2C_PORT i2c1
// (OLED_SDA_PIN e OLED_SCL_PIN, e OLED_ADDR já estão definidos em config.h)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ⚙️ Controle de Carga (MOSFET via PWM)
#define PWM_MOSFET_PIN  0  // Saída PWM para controle de corrente

// 🔔 Feedback Visual e Sonoro
#define LED_R_PIN  13  // Canal vermelho do LED RGB
#define LED_G_PIN  11  // Canal verde do LED RGB
#define LED_B_PIN  12  // Canal azul do LED RGB
#define BUZZER_PIN 21  // Alerta sonoro

// ==================== LIMITES DE OPERAÇÃO ====================
#define CORRENTE_MAXIMA  5.0   // 🔥 Máximo de 5A (aciona buzzer se ultrapassar)

// ==================== PARÂMETROS DO PID ====================
#define PID_KP  0.7  // Proporcional
#define PID_KI  0.01 // Integral
#define PID_KD  0.0001 // Derivativo


#endif  // CONFIG_H
