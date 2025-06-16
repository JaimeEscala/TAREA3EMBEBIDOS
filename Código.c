#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define BOTON_IZQ PC0
#define BOTON_DER PC1

#define VELOCIDAD_FACIL 300
#define VELOCIDAD_MEDIA 150
#define VELOCIDAD_DIFICIL 75

uint16_t velocidad_actual = VELOCIDAD_FACIL;

void init_botones() {
    DDRC &= ~( (1 << BOTON_IZQ) | (1 << BOTON_DER) );
    PORTC |= (1 << BOTON_IZQ) | (1 << BOTON_DER);
}

void init_matriz() {
    DDRD = 0xFF;
    DDRB = 0xFF;
}

void limpiar_matriz() {
    PORTD = 0xFF;
    PORTB = 0xFF;
}

void mostrar_pixel(uint8_t fila, uint8_t columna) {
    PORTD = ~(1 << fila);
    PORTB = ~(1 << columna);
    _delay_ms(2);
}

void seleccionar_nivel(uint8_t nivel) {
    switch (nivel) {
        case 1:
            velocidad_actual = VELOCIDAD_FACIL;
            break;
        case 2:
            velocidad_actual = VELOCIDAD_MEDIA;
            break;
        case 3:
            velocidad_actual = VELOCIDAD_DIFICIL;
            break;
        default:
            velocidad_actual = VELOCIDAD_FACIL;
    }
}

void init_adc() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t leer_adc() {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void mostrar_puntaje(uint16_t puntaje) {
    uint8_t col_puntaje = puntaje % 8;
    for (uint8_t i = 0; i < 5; i++) {
        limpiar_matriz();
        mostrar_pixel(0, col_puntaje);
        _delay_ms(150);
        limpiar_matriz();
        _delay_ms(150);
    }
}

void delay_variable(uint16_t ms) {
    // Divide el retardo en ms unitarios para evitar error _delay_ms()
    for (uint16_t i = 0; i < ms; i++) {
        _delay_ms(1);
    }
}

int main(void) {
    init_botones();
    init_matriz();
    init_adc();
    srand(leer_adc());

    seleccionar_nivel(2);

    uint8_t jugador_col = 3;
    uint8_t obst_col = rand() % 8;
    int8_t obst_fila = 0;
    uint16_t puntaje = 0;

    while (1) {
        // Mover jugador
        if (!(PINC & (1 << BOTON_IZQ)) && jugador_col > 0) {
            jugador_col--;
            _delay_ms(150);
        }
        if (!(PINC & (1 << BOTON_DER)) && jugador_col < 7) {
            jugador_col++;
            _delay_ms(150);
        }

        // Actualizar obstáculo
        obst_fila++;
        if (obst_fila > 7) {
            obst_fila = 0;
            obst_col = rand() % 8;
            puntaje++;
            mostrar_puntaje(puntaje);
        }

        // Refrescar matriz
        for (uint8_t i = 0; i < 50; i++) {
            limpiar_matriz();
            mostrar_pixel(obst_fila, obst_col);
            mostrar_pixel(7, jugador_col);
        }

        // Detección colisión
        if (obst_fila == 7 && obst_col == jugador_col) {
            for (uint8_t j = 0; j < 3; j++) {
                limpiar_matriz();
                _delay_ms(100);
                mostrar_pixel(7, jugador_col);
                _delay_ms(100);
            }
            jugador_col = 3;
            obst_fila = 0;
            obst_col = rand() % 8;
            puntaje = 0;
        }

        delay_variable(velocidad_actual);
    }
}
