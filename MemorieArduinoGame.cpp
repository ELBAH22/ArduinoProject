#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // Largeur de l'écran OLED
#define SCREEN_HEIGHT 64  // Hauteur de l'écran OLED
#define OLED_RESET -1     // Pas de reset

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Déclaration des broches pour les LEDs et les boutons
const int ledPins[] = {2, 3, 4, 5};     // 4 LEDs sur les broches 2 à 5
const int buttonPins[] = {6, 7, 8, 9};  // 4 boutons sur les broches 6 à 9

int sequence[5];         // Tableau pour stocker la séquence des LEDs (jusqu'à 5 étapes)
int sequenceLength = 3;  // Longueur initiale de la séquence
int playerInput[5];      // Tableau pour stocker la réponse du joueur
int score = 0;           // Score du joueur
int multiplier = 1;      // Multiplicateur pour les bonnes réponses consécutives
bool gameRunning = false;  // Indicateur de l'état du jeu

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP); // Activer les résistances de pull-up internes
  }
  
  Serial.begin(9600);
  Serial.println("Jeu de memoire avec Arduino!");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Écran OLED non trouvé !"));
    while(true);
  }
  
  display.clearDisplay();
  showMainMenu();
}

void loop() {
  if (!gameRunning) {
    checkStartGame();
  } else {
    playGame();
  }
}

void showMainMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Appuyez sur Bouton 1");
  display.println("pour demarrer une nouvelle partie");
  display.display();
}

void checkStartGame() {
  if (digitalRead(buttonPins[0]) == LOW) {
    gameRunning = true;
    score = 0;
    multiplier = 1;
    sequenceLength = 3;
    delay(500);
  }
}

void playGame() {
  loadingScreen();       // Afficher l'écran de chargement avant de commencer
  adjustSequenceLength(); // Ajuste la longueur de la séquence en fonction du score

  // Affiche "Reproduis la sequence!" après le chargement
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Reproduis la sequence!");
  display.display();
  delay(1500); // Pause de 1 seconde avant de montrer la séquence

  generateSequence(); // Générer et afficher la séquence
  
  if (!checkPlayerInput()) {
    endGame();
    return;
  }

  updateScore();
}

// Fonction d'affichage de l'écran de chargement animé
void loadingScreen() {
  const char* baseText = "Chargement";
  int baseTextWidth = 12 * strlen(baseText); // Largeur estimée du texte sans les points
  
  for (int i = 0; i < 4; i++) {  // Boucle d'animation pour 4 cycles
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    
    // Calcul pour centrer le texte
    int xPosition = (SCREEN_WIDTH - baseTextWidth) / 2;
    int yPosition = SCREEN_HEIGHT / 2 - 8; // Aligné au centre verticalement
    
    display.setCursor(xPosition, yPosition);
    display.print(baseText);
    
    // Ajouter des points en fonction de l'étape d'animation
    for (int j = 0; j <= i % 3; j++) {
      display.print(".");
    }
    
    display.display();
    delay(500);  // Attente pour l'effet d'animation
  }
}

void adjustSequenceLength() {
  sequenceLength = 3 + (score / 10); // Ajuste la longueur de la séquence en fonction du score
  if (sequenceLength > 5) sequenceLength = 5; // Limite la séquence à une longueur maximale de 5
}

void generateSequence() {
  Serial.print("Sequence des LEDs: ");
  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = random(0, 4);
    Serial.print(sequence[i] + 1);
    Serial.print(" ");
    digitalWrite(ledPins[sequence[i]], HIGH);
    delay(500);
    digitalWrite(ledPins[sequence[i]], LOW);
    delay(250);
  }
  Serial.println();
}

bool checkPlayerInput() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Reproduis la sequence!");
  display.display();

  for (int i = 0; i < sequenceLength; i++) {
    bool buttonPressed = false;
    while (!buttonPressed) {
      for (int j = 0; j < 4; j++) {
        if (digitalRead(buttonPins[j]) == LOW) {
          playerInput[i] = j;
          buttonPressed = true;
          delay(300); // Anti-rebond

          if (playerInput[i] != sequence[i]) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Mauvaise Reponse");
            display.display();
            delay(1000);
            return false;
          }
        }
      }
    }
  }
  return true;
}

void updateScore() {
  score += 1 * multiplier;
  multiplier++;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Bonne Reponse!");
  display.print(" Score: ");
  display.println(score);
  display.display();
  delay(1000);
}

void endGame() {
  gameRunning = false;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Jeu termine!");
  display.println("Score final:");
  display.println(score);
  display.println("Rejouer ? B1: Oui B2: Non");
  display.display();
  
  while (true) {
    if (digitalRead(buttonPins[0]) == LOW) {
      gameRunning = true;
      score = 0;
      multiplier = 1;
      sequenceLength = 3;
      delay(500);
      return;
    } else if (digitalRead(buttonPins[1]) == LOW) {
      showMainMenu();
      delay(500);
      return;
    }
  }
}
