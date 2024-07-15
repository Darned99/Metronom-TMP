
// Opis projektu:
// Stworzyliśmy elektroniczny metronom z wyborem odpowiednich opcji, do jego stworzenia
// zostały wykorzystane następujące elementy:
// -Płytka zawierająca mikrokontroler Atmega328PB
// -Enkoder z przyciskiem
// -Przycisk do włączenia metronomu
// -Głośnik
// -Rezystor 220
// -Wyświetlacz LCD 
// -Breadboard

// W projekcie mamy do wyboru kilka opcji takich jak:
// -wybór uderzeń na minutę
// -wybór ilości uderzeń w takcie (sygnalizowany jest początek taktu głośniejszym uderzeniem)
// -wybór "nuty" tzn. ćwierćnuty, ósemki, szesnastki
// -wybór głośności, przy tej opcji również zmienia się wysokość dźwięku
// 
#include "Metronom.h" // Plik zawierający strukturę oraz funkcje inicjalizujące i obsługujące metronom
#include <LiquidCrystal.h> // Biblioteka potrzebna do wyświetlacza LCD
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
    
    //CTC (Clear Timer on Compare Match) to tryb pracy timerów w mikrokontrolerach AVR, 
    //który umożliwia generowanie precyzyjnych okresów czasowych poprzez 
    //automatyczne wyzerowanie timera po osiągnięciu wartości określonej w rejestrze porównawczym,
    // co jest użyteczne do generowania przerwań, sygnałów PWM oraz dokładnego odmierzania czasu.
    
    // Tryb CTC 
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

// Definicje pinów i stałych
#define LED 13                 // pin diody LED
#define ENCODER_CLK 2          // pin CLK enkodera
#define ENCODER_DT  3          // pin DT enkodera
#define ENCODER_BTN 4          // pin przycisku enkodera
#define SELECT_BTN 6           // pin przycisku wyboru

#define MAX_BEATS_STEPS 9      // maksymalna liczba kroków w takcie

#define NOTE_DURATION_MIN 1    // minimalna długość nuty
#define NOTE_DURATION_MAX 2    // maksymalna długość nuty

#define SOUND_MIN 1            // minimalna głośność
#define SOUND_MAX 10           // maksymalna głośność

// Identyfikatory do menu
#define MENU_TEMPO 0           // tempa
#define MENU_BEATS 1           // taktu
#define MENU_NOTE_DURATION 2   // długości nuty
#define MENU_SOUND 3           // głośności

// Definicje nut
#define NOTE_QUARTERS 4        // ćwierćnuta
#define NOTE_EIGHTS 8          // ósemka
#define NOTE_SIXTEENTHS 16     // szesnastka

Metronom metronom; // Stworzenie obiektu metronomu
LiquidCrystal lcd(12, 11, 10, 9, 8, 7); // Inicjalizacja wyświetlacza LCD oraz odpowiednie piny, które wykorzystujemy
int lastClk = HIGH; // Zmienna przechowująca ostatni stan pinu CLK enkodera
bool isOn = false; // Flaga określająca czy metronom jest włączony

uint8_t step = 0; // Aktualny krok taktu
uint8_t totalSteps = 4; // Liczba kroków w takcie początkowej 

uint8_t sound = 6; // Poziom głośności początkowej

uint8_t noteDuration = NOTE_QUARTERS; // Długość nuty początkowej - ćwierćnuta

uint8_t menuSelect = 0; // Wybrana opcja menu

// Początkowe ustawienia to:
// -BPM:100
// -takt 4/4
// -ćwierćnuta
// -6 poziom głośności

//Odpowiednie symbole zaprojektowane dla komórki 5x8

//Symbol strzałki
uint8_t customCharArrow[] = {
  B01000,
  B01100,
  B11110,
  B11111,
  B11111,
  B11110,
  B01100,
  B01000
};
//Symbol głośniczka
uint8_t customCharSpeaker[] = {
  B00001,
  B00011,
  B01111,
  B01111,
  B01111,
  B00011,
  B00001,
  B00000
};
//Symbol ćwierćnuty
uint8_t customCharNoteQuarter[] = {
  B00010,
  B00010,
  B00010,
  B00010,
  B00010,
  B00110,
  B01110,
  B00100
};
//Symbol ósemki
uint8_t customCharNoteEights[] = {
  B00001,
  B00011,
  B00101,
  B01001,
  B01001,
  B01011,
  B11011,
  B11000
};
//Symbol szesnastki
uint8_t customCharNoteSixteenths[] = {
  B00011,
  B00101,
  B01011,
  B01101,
  B01001,
  B01011,
  B11011,
  B11000
};

// Obsługa przerwania timera
void handleTickCallback() {
  if(!isOn) return; // Jeśli metronom jest wyłączony, zakończ funkcję
  step++; // Zwiększ krok
  if(step > totalSteps - 1) step = 0; // Jeśli krok przekracza całkowitą liczbę kroków, zresetuj go do 0
  digitalWrite(LED, digitalRead(LED) ^ 1); // Odczytanie stanu z LED
  refreshScreen(true); // Odśwież ekran i odtwórz dźwięk
}

// Funkcja setup uruchamiana przy starcie programu
void setup() {
  pinMode(LED, OUTPUT); // Ustawienie pinu LED jako wyjścia
  pinMode(ENCODER_CLK, INPUT); // Ustawienie pinu enkodera CLK jako wejścia
  pinMode(ENCODER_DT, INPUT); // Ustawienie pinu enkodera DT jako wejścia
  pinMode(ENCODER_BTN, INPUT_PULLUP); // Ustawienie pinu przycisku enkodera jako wejścia z podciąganiem
  pinMode(SELECT_BTN, INPUT_PULLUP); // Ustawienie pinu przycisku SELECT jako wejścia z podciąganiem

  Metronom_init(&metronom); // Inicjalizacja metronomu
  Metronom_setup(&metronom); // Konfiguracja timera

  // Inicjalizacja LCD
  lcd.begin(16, 2); // Ustawienie wyświetlacza w tryb pracy 16x2
  //Stworzenie na wyświetlaczu symboli 
  lcd.createChar(1, customCharArrow); // Tworzenie znaku strzałki
  lcd.createChar(2, customCharSpeaker); // Tworzenie znaku głośniczka
  lcd.createChar(3, customCharNoteQuarter); // Tworzenie znaku ćwierćnuty
  lcd.createChar(4, customCharNoteEights); // Tworzenie znaku ósemki
  lcd.createChar(5, customCharNoteSixteenths); // Tworzenie znaku szesnastki
  lcd.setCursor(0, 0); // Ustawienie kursora na początku pierwszej linii
  lcd.print("Metronom v1"); 
  lcd.setCursor(0, 1); // Ustawienie kursora na początku drugiej linii
  lcd.print("BK WK"); 
  delay(2000); // Opóźnienie 2 sekundy
  refreshScreen(false); // Odświeżenie ekranu bez odtwarzania dźwięku
}

// Odświeżanie ekranu LCD
void refreshScreen(bool playTone) {
  lcd.clear(); // Wyczyść ekran LCD

  // Wyświetlanie tempa
  lcd.setCursor(1, 0);
  lcd.print("BPM:");
  lcd.print(metronom.tempo);
  
  // Obsługa dźwięku
  if(playTone) {
    soundBeats();
  }

  // Wyświetlanie uderzeń
  drawBeats();

  // Wyświetlanie nut
  lcd.setCursor(13, 0);
  lcd.write(4);
  lcd.write(":");
  switch(noteDuration) {
    case NOTE_QUARTERS:
      lcd.write(3);  // Rysowanie ćwierćnuty
    break;
    case NOTE_EIGHTS:
      lcd.write(4);  // Rysowanie ósemki
    break;
    case NOTE_SIXTEENTHS:
      lcd.write(5);  // Rysowanie szesnastki
    break;
  }

  // Wyświetlanie dźwięku
  lcd.setCursor(13, 1);
  lcd.write(2);
  lcd.write(":");
  lcd.print(sound);

  // Wyświetlanie menu
  drawMenu();
}

//Rysowanie uderzeń na ekranie LCD
void drawBeats() {
  lcd.setCursor(1, 1);
  lcd.print(totalSteps);
  lcd.print(":");

  for(uint8_t i = 0; i < totalSteps; i++) {
    if(step == i && isOn) { // Jeśli to jest aktualny krok i metronom jest włączony
      if(step == 0) {
        lcd.write(255); // Pierwszy krok oznaczony pełnym blokiem
      } else {
        lcd.write(219); // Kolejne kroki oznaczone częściowym blokiem
      }
    } else {
      lcd.write("."); // Krok nieaktywny oznaczony kropką
    }
  }
}

// Obsługa dźwięków uderzeń
void soundBeats() {
  lcd.setCursor(1, 1);
  lcd.print(totalSteps);
  lcd.print(":");

  for(uint8_t i = 0; i < totalSteps; i++) {
    if(step == i && isOn) { // Jeśli to jest aktualny krok i metronom jest włączony
      if(step == 0) {
        tone(5, 400 * sound + 200, 20); // Wysokość dźwięku dla pierwszego kroku
      } else {
        tone(5, 300 * sound + 200, 20); // Wysokość dźwięku dla kolejnych kroków
      }
    } else {
      lcd.write("."); // Krok nieaktywny oznaczony kropką
    }
  }
}

// Rysowanie menu na ekranie LCD
void drawMenu() {
  switch(menuSelect) {
    case MENU_TEMPO:
      lcd.setCursor(0, 0);
      lcd.write(1); // Rysowanie strzałki przy opcji tempa
    break;
    case MENU_BEATS:
      lcd.setCursor(0, 1);
      lcd.write(1); // Rysowanie strzałki przy opcji uderzeń
    break;
    case MENU_NOTE_DURATION:
      lcd.setCursor(12, 0);
      lcd.write(1); // Rysowanie strzałki przy opcji długości nuty
    break;
    case MENU_SOUND:
      lcd.setCursor(12, 1);
      lcd.write(1); //Rysowanie strzałki przy opcji głośności
    break;
  }
}
// Funckaj loop odpowiedzialna głównie za obsługę urządzenia
void loop() {
  int newClk = digitalRead(ENCODER_CLK); // odczyt stanu pinu CLK enkodera
  if (newClk != lastClk) {
    // Zmiana na pinie CLK
    lastClk = newClk;
    int dtValue = digitalRead(ENCODER_DT); // odczyt stanu pinu DT enkodera
    if (newClk == LOW && dtValue == HIGH) {
      handleRotaryUp(); // obsługa obrotu enkodera w górę
      refreshScreen(false);
    } else if (newClk == LOW && dtValue == LOW) {
      handleRotaryDown(); // obsługa obrotu enkodera w dół
      refreshScreen(false);
    }
    delay(5); // opóźnienie dla stabilizacji odczytu
  }
  // Obsługa przycisku
  if (digitalRead(SELECT_BTN) == LOW) {
    isOn = !isOn;           // zmiana stanu metronomu
    step = 0;               // reset kroku
    Metronom_reset(&metronom); // reset metronomu
    refreshScreen(true);    // odświeżenie ekranu z dźwiękiem
    delay(200);             // opóźnienie zmiany opcji
  }
  // Obsługa menu
  if(digitalRead(ENCODER_BTN) == LOW) {
    menuSelect++;           // przejście do następnej opcji menu
    if(menuSelect > 3) menuSelect = 0; // resetowanie wyboru menu, jeśli przekroczono maksymalną wartość
    refreshScreen(false);   // odświeżenie ekranu bez dźwięku
    delay(200);             // opóźnienie 
  }
}

// Obsługa enkodera w górę
void handleRotaryUp() {
  switch(menuSelect) {
    case MENU_TEMPO:
      Metronom_addOneBPM(&metronom); // zwiększenie tempa o 1 BPM
    break;
    case MENU_BEATS:
      addTotalSteps();               // zwiększenie liczby kroków
    break;
    case MENU_NOTE_DURATION:
      addNoteDuration();             // zmiana długości nuty w górę
    break;
    case MENU_SOUND:
      addSoundStep();                // zwiększenie głośności
    break;
  }
}

// Obsługa enkodera w dół
void handleRotaryDown() {
  switch(menuSelect) {
    case MENU_TEMPO:
      Metronom_subtractOneBPM(&metronom); // zmniejszenie tempa o 1 BPM
    break;
    case MENU_BEATS:
      subtractTotalSteps();               // zmniejszenie liczby kroków
    break;
    case MENU_NOTE_DURATION:
      subtractNoteDuration();             // zmiana długości nuty w dół
    break;
    case MENU_SOUND:
      subtractSoundStep();                // zmniejszenie głośności
    break;
  }
}

// Zmiana trwania nuty w górę
void addNoteDuration() {
  if(noteDuration == NOTE_QUARTERS) {
    noteDuration = NOTE_EIGHTS;           // zmiana na ósemkę
  } else if(noteDuration == NOTE_EIGHTS) {
    noteDuration = NOTE_SIXTEENTHS;       // zmiana na szesnastkę
  } else {
    noteDuration = NOTE_QUARTERS;         // zmiana na ćwierćnutę
  }
  Metronom_setNoteDuration(&metronom, noteDuration); // ustawienie nowego trwania nuty
  Metronom_reset(&metronom);            // reset metronomu
  step = 0;                             // reset kroku
  refreshScreen(true);                  // odświeżenie ekranu z dźwiękiem
}

// Zmiana trwania nuty w dół
void subtractNoteDuration() {
  if(noteDuration == NOTE_QUARTERS) {
    noteDuration = NOTE_SIXTEENTHS;       // zmiana na szesnastkę
  } else if(noteDuration == NOTE_SIXTEENTHS) {
    noteDuration = NOTE_EIGHTS;           // zmiana na ósemkę
  } else {
    noteDuration = NOTE_QUARTERS;         // zmiana na ćwierćnutę
  }
  step = 0;                               // reset kroku
  Metronom_setNoteDuration(&metronom, noteDuration); // ustawienie nowego trwania nuty
  Metronom_reset(&metronom);            // reset metronomu
  refreshScreen(true);                  // odświeżenie ekranu z dźwiękiem
}

// Zmiana głośności w górę
//void addSoundStep() {
  //if(sound < SOUND_MAX) {
    //sound++;                             // zwiększenie głośności
  //}  
//}

// Zamiana głośności w górę
void addSoundStep() {
    if (sound < SOUND_MAX) {
        sound++; // zwiększenie głośności
    } else {  
        sound = 1; // Rest poziomu głośności jeśli osiągnie maksymalny poziom
    }
    refreshScreen(false);
}

// Zmiana głośności w dół
void subtractSoundStep() {
  if(sound > SOUND_MIN) {
    sound--;                             // zmniejszenie głośności
  }  
}

// Zmiana liczby kroków w górę
//void addTotalSteps() {
  //if(totalSteps < MAX_BEATS_STEPS) {
    //totalSteps++;                        // zwiększenie liczby kroków
  //}
  //refreshScreen(false);                  // odświeżenie ekranu bez dźwięku
//}


void addTotalSteps() {
  if(totalSteps < MAX_BEATS_STEPS) {
    totalSteps++; // zwiększenie liczby kroków
  } else {
    totalSteps = 1; // Cykliczne przejście z 9 do 1
  }
  refreshScreen(false); // odświeżenie ekranu bez dźwięku
}

// Zmiana liczby kroków w dół
void subtractTotalSteps() {
  if(totalSteps > 1) {
    totalSteps--;                        // zmniejszenie liczby kroków
  }
  refreshScreen(false);                  // odświeżenie ekranu bez dźwięku
}

// Przerwanie timera
ISR(TIMER1_COMPA_vect) {
  handleTickCallback();                  
}
