#include "menu.h"
#include "hardware.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

// Variáveis estáticas para manter o estado do menu
static menu_t current_menu = MENU_CC;
static bool editing = false;       // Indica se está em modo de edição
static int selected_field = 0;     // Para menus com múltiplos campos (PID e MQTT)
static int selected_digit = 0;     // Para escolher qual digito


// Variável para piscar o campo em edição (simulada alternância)
static bool blink_on = false;

// Função auxiliar para simular o efeito de piscar
static void update_blink(void) {
    blink_on = !blink_on;
}

// Inicializa o menu definindo o estado inicial e exibindo a tela "CC"
void menu_init(ssd1306_t *oled, double *measured_current, double *current_setpoint) {
    current_menu = MENU_CC;
    editing = false;
    selected_field = 0;
    
    ssd1306_clear(oled);
    ssd1306_draw_string(oled, 10, 0, 1, "CC");
    
    char buf[32];
    sprintf(buf, "Corrente: %.2fA", *measured_current);
    ssd1306_draw_string(oled, 0, 16, 1, buf);
    
    sprintf(buf, "Setpoint: %.2fA", *current_setpoint);
    ssd1306_draw_string(oled, 0, 48, 1, buf);
    
    
    ssd1306_show(oled);
}

// Atualiza a tela do menu de acordo com o modo atual
void menu_update(ssd1306_t *oled, double *current_setpoint, double *measured_current) {
    update_blink();
    ssd1306_clear(oled);
    char buf[32];
    char current_str[10];
    
    switch(current_menu) {
        case MENU_CC:
            ssd1306_draw_string(oled, 10, 0, 1, "CC");
            sprintf(buf, "Corrente: %.2fA", *measured_current);
            ssd1306_draw_string(oled, 0, 32, 1, buf);
            if (blink_on && editing){
                sprintf(current_str, "%.2f", *current_setpoint);
                current_str[3-selected_digit] = ' ';
                sprintf(buf, "Setpoint: %sA", current_str);
            }else
                sprintf(buf, "Setpoint: %.2fA", *current_setpoint);
            ssd1306_draw_string(oled, 0, 48, 1, buf);
            break;
            
            
        default:
            break;
    }
    
    ssd1306_show(oled);
}

// Botão C: Inicia ou alterna o modo de edição no menu atual
void menu_button_c_pressed(void) {
    editing = !editing;
}

// Botão B: Sai do modo de edição
void menu_button_b_pressed(void) {
    current_menu = (current_menu + 1) % MENU_MAX;
}

// Atualiza os valores do menu com base no movimento do joystick
// vrx (eixo horizontal): fora da edição, muda a tela; em edição, altera o campo (para PID e MQTT)
// vry (eixo vertical): em edição, ajusta o valor do parâmetro selecionado
void menu_update_joystick(int16_t vrx, int16_t vry, double *current_setpoint) {
    const int16_t threshold = 1000;
    
    if (editing) {
        if (current_menu == MENU_CC) {
            if (vrx > threshold) {
                switch (selected_digit){
                    case 0:
                        *current_setpoint+=0.01;
                        break;
                    
                    case 1:
                        *current_setpoint+=0.1;
                        break;

                    case 3:
                        *current_setpoint+=1;
                        break;
                }
            } else if (vrx < -threshold) {
                switch (selected_digit){
                    case 0:
                        *current_setpoint-=0.01;
                        break;
                    
                    case 1:
                        *current_setpoint-=0.1;
                        break;

                    case 3:
                        *current_setpoint-=1;
                        break;
                }
            }
            if (vry > threshold) {
                selected_digit--;
                if(selected_digit==2)
                    selected_digit--;
            } else if (vry < -threshold) {
                selected_digit++;
                if(selected_digit==2)
                    selected_digit++;
            }
            if(selected_digit<=0)
                selected_digit=0;
            if(selected_digit>=3)
                selected_digit=3;
            if(*current_setpoint<0){
                beep(); 
                *current_setpoint=0;
            }
            if(*current_setpoint>CORRENTE_MAXIMA){
                beep(); 
                *current_setpoint=CORRENTE_MAXIMA;
            }
        } 
    }
}


menu_t menu_get_mode(void) {
    return current_menu;
}