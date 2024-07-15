#include "Metronom.h"      // Plik zawierający strukturę oraz funkcje inicjalizujące i obsługujące metronom
#include <avr/io.h>        // Biblioteka do obsługi wejść/wyjść
#include <avr/interrupt.h> // Biblioteka do obsługi przerwań

// Inicjalizacja metronomu
void Metronom_init(Metronom* metronom) {
    metronom->tempo = 100;          // Ustawienie początkowego tempa na 100 BPM
    metronom->noteDuration = 4;     // Ustawienie domyślnej długości nuty na ćwierćnutę
}

// Obliczanie skalera dla danego tempa
int Metronom_calcPrescaler(Metronom* metronom, uint8_t bpm) {
    // Liczba milisekund w minucie to 60000
    long minuteMillis = 60000;
    // Obliczamy długość jednej ćwierćnuty (w milisekundach)
    long quarterNoteDuration = minuteMillis / bpm;
    // Obliczamy długość nuty (ćwierćnuta, ósemka, szesnastka) w milisekundach
    long noteDurationMillis = quarterNoteDuration / metronom->noteDuration;
    // Zwracamy wartość do ustawienia rejestru porównawczego
    return (16000000L / 256) * noteDurationMillis / 1000 - 1;
}
// Inne obliczenie skalera
//int Metronom_calcPrescaler(Metronom* vt, uint8_t bpm) {
    //long scaler = 256;
    //long bpmPrescaler = bpm * scaler;
    //long noteScaler = 960000000 / (vt->noteDuration);
    //return (noteScaler - bpmPrescaler) / bpmPrescaler;
//}

// Konfiguracja timera
void Metronom_setup(Metronom* metronom) {
    //noInterrupts();
    cli(); // Wyłączenie przerwania 
    // Wyczyszczenie rejestrów
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    // Ustawienie rejestru porównania wyjściowego
    OCR1A = Metronom_calcPrescaler(metronom, metronom->tempo);
    // Tryb CTC (Clear Timer on Compare Match)
    TCCR1B |= (1 << WGM12);
    // Ustawienie preskalera na 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // Włączenie przerwania porównania wyjściowego
    TIMSK1 |= (1 << OCIE1A);
    //interrupts();
    sei(); // Włączenie przerwania
}

// Reset timera
void Metronom_reset(Metronom* metronom) {
    TCNT1 = 0; // Ustawienie licznika timera na 0
}

// Ustawianie czasu trwania nuty
void Metronom_setNoteDuration(Metronom* metronom, uint8_t noteDuration) {
    metronom->noteDuration = noteDuration; // Ustaw długości nuty
    OCR1A = Metronom_calcPrescaler(metronom, metronom->tempo); // Przeliczenie wartości skalera
}

// Ustawianie tempa
void Metronom_setTempo(Metronom* metronom, uint8_t tempo) {
    metronom->tempo = tempo; // Ustawienie nowego tempa
    OCR1A = Metronom_calcPrescaler(metronom, tempo); 
}

// Zwiększanie tempa o 1 BPM
//void Metronom_addOneBPM(Metronom* metronom) {
    //if (metronom->tempo < 250) { // Sprawdzenie czy tempo nie przekracza maksymalnej wartości
        //metronom->tempo++; // Zwiększenie tempa
        //OCR1A = Metronom_calcPrescaler(metronom, metronom->tempo); 
    //}
//}

// Zwiększanie tempa o 1 BPM
void Metronom_addOneBPM(Metronom* metronom) {
    if (metronom->tempo < 250) { // Sprawdzenie czy tempo nie przekracza maksymalnej wartości
        metronom->tempo++; // Zwiększenie tempa
    } else {
        metronom->tempo = 30; // Cykliczne przejście z 250 do 120
    }
  OCR1A = Metronom_calcPrescaler(metronom, metronom->tempo);
}

// Zmniejszanie tempa o 1 BPM
void Metronom_subtractOneBPM(Metronom* metronom) {
    if (metronom->tempo > 1) { // Sprawdzenie czy tempo nie jest mniejsze niż minimalna wartość
        metronom->tempo--; // Zmniejsz tempo
        OCR1A = Metronom_calcPrescaler(metronom, metronom->tempo); // Przelicz wartość preskalera
    }
}
