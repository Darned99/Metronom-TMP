#ifndef METRONOM_H
#define METRONOM_H

#include <stdint.h> // Załączanie nagłówka dla typów całkowitych o określonej wielkości

// Definicja struktury dla metronomu
typedef struct {
  uint8_t tempo;           // tempo metronomu w BPM (Beats Per Minute)
  uint8_t noteDuration;    // czas trwania nuty (w jednostkach, np. ćwierćnuta, ósemka)
} Metronom;

// Funkcje inicjalizujące i obsługujące metronom
void Metronom_init(Metronom* metronom); // Inicjalizacja metronomu z domyślnymi wartościami
int Metronom_calcPrescaler(Metronom* metronom, uint8_t bpm); // Obliczanie preskalera na podstawie tempa
void Metronom_setup(Metronom* metronom); // Konfiguracja timera 
void Metronom_reset(Metronom* metronom); // Resetowanie timera 
void Metronom_setNoteDuration(Metronom* metronom, uint8_t noteDuration); // Ustawianie czasu trwania nuty
void Metronom_setTempo(Metronom* metronom, uint8_t tempo); // Ustawianie tempa 
void Metronom_addOneBPM(Metronom* metronom); // Zwiększanie tempa o 1 BPM
void Metronom_subtractOneBPM(Metronom* metronom); // Zmniejszanie tempa o 1 BPM

#endif // METRONOM_H
