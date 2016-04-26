#include <TSL2561.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <dht11.h>

#define dhtPin 2
#define buzzerPin 3
#define rainPin A0
#define snoozeButtonPin 4
#define backlightPin 5
#define tslAddr 0x29
#define lcdAddr 0x27

uint8_t Rw = 0, Rs = 4, d4 = 5, d5 = 6, d6 = 7, backlighPin = 3;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_BME280 bme = Adafruit_BME280();
TSL2561 tsl = TSL2561(tslAddr);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(lcdAddr, SCL, SDA, Rw, Rs, d4, d5, d6, backlighPin, POSITIVE);

dht11 DHT;

long backlightTimer, backlightDelay = 30 * 1000; // 30 Seconds
bool backlight = true;

uint64_t snoozeTimer = 1 * 60 * 60 * 1000; // One hour
bool snoozing = false;

String temperatureLabel = "Temp:", humidityLabel = "Humi:", pressureLabel = "Baro:", 
						  skyTemperatureLabel = "Sky:", lightLabel = "Mag/S^2:",
						  rainLabel = "Rain:", snoozeLabel = "Snz:", weatherLabel = "Fcst:";

uint16_t const rainNONE = 1020, rainMIST = 950, rainLLIGHTt = 800, rainRAIN = 700, rainHEAVY = 400, rainDOWNPOUR = 300;
uint16_t const rainALARM = 950, skyTEMP = 10, dewPOINT = 5;

int dhtTemp, humidity;

float temperature, pressure;
double skyTemperature;
uint16_t luminosity;
uint16_t rain;

void setup()
{
	Serial.begin(115200);
	lcd.begin(20, 4);
	bme.begin();
	mlx.begin();
	tsl.begin();
	tsl.setGain(TSL2561_GAIN_16X);
	tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);

	DHT.attach(dhtPin);
	Serial.println("SkyPilot");
	Serial.println("Sky Conditions");
	Serial.println("Monitor V1.0, 2016");
	Serial.println("By Jeffrey Voight");
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("SkyPilot");
	lcd.setCursor(0, 1);
	lcd.print("Sky Conditions");
	lcd.setCursor(0, 2);
	lcd.print("Monitor V1.0, 2016");
	lcd.setCursor(0, 3);
	lcd.print("By Jeffrey Voight");
	delay(3000);
	lcd.clear();
	displayLabels();
}

void loop()
{
	DHT.read();
	humidity = DHT.humidity;
	pressure=bme.readPressure();
	temperature = bme.readTemperature();
	skyTemperature=mlx.readObjectTempC();
	luminosity = tsl.getLuminosity(TSL2561_VISIBLE);
	rain = analogRead(rainPin);
	backlight = needBacklight();
	if (backlight) {
		lcd.backlight();
	} else {
		lcd.noBacklight();
	}
	if (rain < rainALARM && !snoozing) {
		soundAlarm();
	}
  /* add main program code here */

}

void soundAlarm() {

}

bool needBacklight() {
	bool returnVal = true;
	bool buttonPushed = debounce(backlightPin);
	if (buttonPushed) {
		returnVal = true;
		backlightTimer = millis() + backlightDelay;
	} else if (backlightTimer > millis()) {
		returnVal = false;
	}
	return returnVal;
}

bool debounce(uint8_t _pin) {
	uint8_t read = digitalRead(_pin);
	delay(50);
	if (digitalRead(_pin) == read && read == LOW) {
		return true;
	} else {
		return false;
	}
}

void displayLabels() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(temperatureLabel);
	lcd.setCursor(10, 0);
	lcd.print(humidityLabel);
	lcd.setCursor(0, 1);
	lcd.print(pressureLabel);
	lcd.setCursor(10, 1);
	lcd.print(skyTemperatureLabel);
	lcd.setCursor(0, 2);
	lcd.print(lightLabel);
	lcd.setCursor(10, 2);
	lcd.print(rainLabel);
	lcd.setCursor(0, 3);
	lcd.print(weatherLabel);
	lcd.setCursor(10, 3);
	lcd.print(snoozeLabel);
}