#ifndef MENU_H
#define MENU_H

#include "ssd1306.h"
#include <stdint.h>
#include <stdbool.h>

// Enumeração dos modos de menu
typedef enum {
    MENU_CC = 0, 
    MENU_MAX
} menu_t;

// Inicializa o sistema de menus com o display OLED
void menu_init(ssd1306_t *oled, double *measured_current, double *current_setpoint);

// Atualiza a tela do menu de acordo com o estado atual
void menu_update(ssd1306_t *oled, double *current_setpoint, double *measured_current);

// Funções para tratamento de entradas (botões e joystick)
// Botão C: Em modo de edição, avança para o próximo dígito/campo; fora da edição, inicia a edição
void menu_button_c_pressed(void);

// Botão B: Sai ou cancela o modo de edição, salvando o valor atual
void menu_button_b_pressed(void);

// Atualiza os valores do menu com base no movimento do joystick
// vrx: eixo horizontal (em edição, muda o dígito selecionado; fora da edição, alterna telas)
// vry: eixo vertical (em edição, ajusta o dígito selecionado)
void menu_update_joystick(int16_t vrx, int16_t vry, double *current_setpoint);

menu_t menu_get_mode(void);

#endif // MENU_H
