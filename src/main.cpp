#include <Arduino.h>
#include <EEPROM.h>
#include <Tone.h>

struct Led;
struct Sound;

void blink(int pin, int blinkMs, int blinkAmount);
void blink(int pins[4], int blinkMsm, int amount);
bool buttonIsPressed(int pin);
int getButtonPressed();
void showLed(Led* led);
void pickMode();
void checkHighScore(unsigned int score);
void printf(char* string, int value);
void debug();
void reset();
void start();
void choosing();
void playSound(Sound tones[], int length);
void choosingLed();
void showLedArray();

#define EEPROM_ADRESS_HIGHSCORE 0

#define BUTTON_1_PIN A0
#define BUTTON_2_PIN A1
#define BUTTON_3_PIN A2
#define BUTTON_4_PIN A3

#define LED_RED_PIN 2
#define LED_BLUE_PIN 3
#define LED_GREEN_PIN 4
#define LED_YELLOW_PIN 5

#define BUZZER_PIN 11

#define MAX_ROUNDS 5

enum GAME_STATE {
	NOTRUNNING,
	SHOWLED,
	CHOOSING,
	DEAD
};

GAME_STATE STATE = NOTRUNNING;

uint16_t speed = 500;
uint16_t gameRound = 1;
uint8_t highScore = 0;

struct Sound {
	uint16_t tone;
	uint16_t delay;
};

struct Led {
	uint16_t tone;
	int8_t led;
	int8_t button;
};

const Led leds[4] = {
	{ 100, LED_RED_PIN, BUTTON_4_PIN },
	{ 200, LED_BLUE_PIN, BUTTON_3_PIN },
	{ 300, LED_GREEN_PIN, BUTTON_2_PIN },
	{ 400, LED_YELLOW_PIN, BUTTON_1_PIN }
};

Sound startSound[5] = {
	{ NOTE_C4, 200 }, { NOTE_E4, 200 }, { NOTE_G4, 200 }, { NOTE_C4, 400 }, { NOTE_A4, 200 }
};

Sound deathSound[5] = {
    { NOTE_A4, 200 }, { NOTE_G4, 200 }, { NOTE_E4, 200 }, { NOTE_D4, 400 }, { NOTE_C4, 400 }
};

Sound victorySound[5] = {
    { NOTE_C4, 200 }, { NOTE_E4, 200 }, { NOTE_G4, 200 }, { NOTE_C5, 400 }, { NOTE_G4, 300 }
};

Sound menuSound[3] = {
    { NOTE_D4, 100 }, { NOTE_E4, 100 }, { NOTE_F4, 100 }
};

Led* rounds[MAX_ROUNDS];

int ledPins[4] = { LED_RED_PIN, LED_BLUE_PIN, LED_GREEN_PIN, LED_YELLOW_PIN };

void setup() {
  
	Serial.begin(9600);

	for (int i = 2; i < 6; i++)
		pinMode(i, OUTPUT);

	pinMode(BUTTON_1_PIN, INPUT_PULLUP);
	pinMode(BUTTON_2_PIN, INPUT_PULLUP);
	pinMode(BUTTON_3_PIN, INPUT_PULLUP);
	pinMode(BUTTON_4_PIN, INPUT_PULLUP);

	randomSeed(analogRead(A5));
	noTone(BUZZER_PIN);

	// Initialize rounds array to avoid undefined values
    for (size_t i = 0; i < MAX_ROUNDS; i++) {
        rounds[i]->tone = 0;
        rounds[i]->led = 0;
        rounds[i]->button = 0;
    }

	// Gets the highscore out of the EEPROM chip
	EEPROM.get(EEPROM_ADRESS_HIGHSCORE, highScore);
	Serial.println(highScore);
}

void loop() {

	// Opens a menu where you can pick diff modes
	pickMode();

	// if state is notrunning welcome the player
	if (STATE == NOTRUNNING) {
		if (getButtonPressed() != -1) {
			playSound(startSound, sizeof(startSound) / sizeof(Sound));
			delay(200);
			start();
		}

		return;
	}

	if (gameRound == 1)
		rounds[0] = &leds[random(0, 4)];

	showLedArray();

	// debug();

	choosingLed();
	
	if (STATE == DEAD) {
		playSound(deathSound, sizeof(deathSound) / sizeof(Sound));
		printf("You have lost with a score of %d", gameRound);
		blink(ledPins, 250, 3);
		checkHighScore(gameRound);
		reset();
		return;
	}
	
	if (gameRound == MAX_ROUNDS) {
		playSound(victorySound, sizeof(victorySound) / sizeof(Sound));
		printf("You have won with a score of %d", gameRound);
		checkHighScore(gameRound);

		blink(ledPins, 200, 10);

		reset();
		return;
	}

	STATE = SHOWLED;

	gameRound++;
}

void start() {
	gameRound = 1;
	STATE = CHOOSING;
	blink(ledPins, 500, 1);
}

void pickMode() {
	if (!buttonIsPressed(BUTTON_1_PIN) || !buttonIsPressed(BUTTON_2_PIN) 
		|| !buttonIsPressed(BUTTON_3_PIN) || !buttonIsPressed(BUTTON_4_PIN))
		return;

	playSound(menuSound, sizeof(menuSound) / sizeof(Sound));

	blink(ledPins, 200, 5);

	bool loop = true;

	while (loop)
	{
		Serial.println(getButtonPressed());
		delay(500);

		switch (getButtonPressed())
		{
			case 14:
				delay(200);
				blink(LED_YELLOW_PIN, 100, 5);
				speed = 1000;
				loop = false;	
				break;
			case 15:
				delay(200);
				blink(LED_GREEN_PIN, 100, 5);
				speed = 500;
				loop = false;	
				break;
			case 16:
				delay(200);
				blink(LED_BLUE_PIN, 100, 5);
				speed = 250;
				loop = false;	
				break;
			case 17:
				while (true)
				{
					delay(100);

					if (buttonIsPressed(BUTTON_4_PIN)) {
						loop = false;	
						break;
					}
					else if (buttonIsPressed(BUTTON_3_PIN)) {
						highScore = 0;
						EEPROM.put(EEPROM_ADRESS_HIGHSCORE, 0);
						blink(LED_BLUE_PIN, 100, 5);
					}

				}
				break;
		}

	}

	delay(200);
	STATE = NOTRUNNING;
	gameRound = 1;
}

void choosingLed() {
	STATE = CHOOSING;
	int input = 0;

	while (input < gameRound)
	{
		int button = getButtonPressed();

		if (button != -1) {
			// Gets the led that needs to be choosen.
			Led* led = rounds[input];

			if (led->button == button) {
				Serial.println("Correct button pressed!");
				showLed(led);
				input++;
			} else {
				Serial.println("Incorrect button! Game over.");
				STATE = DEAD;
				break;
			}
		}
	}

}

void showLedArray() {
	for (size_t i = 0; i < gameRound; i++) {
		if (gameRound - 1 == i && gameRound != 1) {
			Led* led = &leds[random(0, 4)];
			showLed(led);
			rounds[i] = led;
			break;
		}

		showLed(rounds[i]);
	}
}

int getButtonPressed() {
	if (buttonIsPressed(BUTTON_4_PIN))
		return BUTTON_4_PIN;
	else if (buttonIsPressed(BUTTON_3_PIN))
		return BUTTON_3_PIN;
	else if (buttonIsPressed(BUTTON_2_PIN))
		return BUTTON_2_PIN;
	else if (buttonIsPressed(BUTTON_1_PIN))
		return BUTTON_1_PIN;

	return -1;
}

void checkHighScore(unsigned int score) {
	// checks for highscore
	if (score > highScore) {
		printf("You got a new highscore of %d", score);
		EEPROM.put(EEPROM_ADRESS_HIGHSCORE, score);
	}
}

void showLed(Led* led) {
	Serial.println(led->led);
	digitalWrite(led->led, HIGH);
	tone(BUZZER_PIN, led->tone);
	delay(speed);
	digitalWrite(led->led, LOW);
	noTone(BUZZER_PIN);
	delay(speed);
}

bool buttonIsPressed(int pin) {
	return digitalRead(pin) == 0;
}

void blink(int pins[4], int blinkMs, int amount) {
	for (size_t j = 0; j < amount; j++) {

		for (size_t i = 0; i < 4; i++) 
			digitalWrite(pins[i], HIGH);

		delay(blinkMs);

		for (size_t i = 0; i < 4; i++) 
			digitalWrite(pins[i], LOW);

		delay(blinkMs);
	}

}

void blink(int pin, int blinkMs, int blinkAmount) {
	for (size_t i = 0; i < blinkAmount; i++) {
		digitalWrite(pin, HIGH);
		delay(blinkMs);
		digitalWrite(pin, LOW);
		delay(blinkMs);
	}
}

void printf(char* string, int value) {
	sprintf(string, string, value);
	Serial.println(string);
}

// Plays a sound of array sound with a certain length
void playSound(Sound sounds[], int length) {

	for (int i = 0; i < length; i++) {
		tone(BUZZER_PIN, sounds[i].tone);
		delay(sounds[i].delay);
	}

	noTone(BUZZER_PIN);
}

void reset() {
	gameRound = 1;
	STATE = NOTRUNNING;
	delay(1000);
}

// Debug function
void debug() {
	for (size_t i = 0; i < gameRound; i++) {
		Serial.print("rounds["); Serial.print(i); 
		Serial.print("].button = "); Serial.println(rounds[i]->button);
	}
}