#include <Adafruit_TSL2561_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>

#define dhtPin 2
#define buzzerPin 3
#define rainPin A0
#define snoozeButtonPin 4
#define backlightPin 5
#define tslAddr 0x39
#define lcdAddr 0x27
#define bmeAddr 0x76
uint8_t En=2, Rw = 1, Rs = 0, d0=4,d4 = 5, d5 = 6, d6 = 7, backlighPin = 3;

Adafruit_MLX90614 mlx;
Adafruit_BME280 bme;
Adafruit_TSL2561_Unified  tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(lcdAddr, En, Rw, Rs, d0, d4, d5, d6, backlighPin, POSITIVE);

sensors_event_t event;
uint16_t broadband, ir;

long backlightTimer, backlightDelay = 30 * 1000; // 30 Seconds
bool backlight = true;

uint64_t snoozeTimer = 1 * 60 * 60 * 1000; // One hour
bool snoozing = false;

String temperatureLabel = "T", humidityLabel = "Rh", pressureLabel = "P", dewPointLabel="Dp", 
						  skyTemperatureLabel = "S", lightLabel = "L",
						  rainLabel = "Rain:", snoozeLabel = "Snz:", weatherLabel = "Fcst:";

uint16_t const rainNONE = 1020, rainMIST = 950, rainLIGHT = 800, rainRAIN = 700, rainHEAVY = 400, rainDOWNPOUR = 300;
uint16_t const rainALARM = 950, skyTEMP = 10, dewPOINT = 5;


float temperature, humidity, pressure, dewPoint;
double skyTemperature;
float luminosity;
uint16_t rain;
String const spaces = "                    ";
String const zeros = "00000000000000000000";
String weather;

float tempTrend[48], skyTempTrend[48], pressureTrend[48], humidityTrend[48], dewPointTrend[48];

enum symbols  { TRENDUP, TRENDUPFAST, TRENDDOWN, TRENDDOWNFAST, DEGREEC, MB, SQUARED };
byte trendUp[8] = {
	0b00000,
	0b01111,
	0b00011,
	0b00101,
	0b01001,
	0b10000,
	0b00000,
	0b00000
};
byte trendUpFast[8] = {
	0b00100,
	0b01110,
	0b10101,
	0b00100,
	0b00100,
	0b00100,
	0b00000,
	0b00000
};
byte trendDown[8] = {
	0b00000,
	0b10000,
	0b01001,
	0b00101,
	0b00011,
	0b01111,
	0b00000,
	0b00000
};
byte trendDownFast[8] = {
	0b00100,
	0b00100,
	0b00100,
	0b10101,
	0b01110,
	0b00100,
	0b00000,
	0b00000
};
byte degreeC[8] = {
	0b01000,
	0b10100,
	0b01000,
	0b00110,
	0b01001,
	0b01000,
	0b01001,
	0b00110
};

byte mb[8] = {
	0b01010,
	0b10101,
	0b10001,
	0b00100,
	0b00100,
	0b00111,
	0b00101,
	0b00111
};
byte squared[8] = {
	0b11100,
	0b00100,
	0b11100,
	0b10000,
	0b11100,
	0b00000,
	0b00000,
	0b00000
};

void setup()
{
	

	Serial.begin(115200);
	lcd.begin(20, 4);
	if (!bme.begin(bmeAddr)) {
		Serial.println("Could not find BME280!");
	}
	if (!mlx.begin() ){
		Serial.println("Could not find MLX!");
	}
	if (!tsl.begin()) {
		Serial.println("Could not find TSL!");
	}
	tsl.enableAutoRange(true);
	tsl.setGain(TSL2561_GAIN_16X);
	tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);

	humidity = bme.readHumidity();
	pressure = bme.readPressure() / 100.0F;
	temperature = bme.readTemperature();
	dewPoint = temperature - ((100 - humidity) / 5.);
	skyTemperature = mlx.readObjectTempC();
	
	fillTrend(tempTrend, temperature);
	fillTrend(skyTempTrend, skyTemperature);
	fillTrend(humidityTrend, humidity);
	fillTrend(pressureTrend, pressure);
	fillTrend(dewPointTrend, dewPoint);

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
	lcd.createChar(0, trendUp);
	lcd.createChar(1, trendUpFast);
	lcd.createChar(2, trendDown);
	lcd.createChar(3, trendDownFast);
	lcd.createChar(4, degreeC);
	lcd.createChar(5, mb);
	lcd.createChar(6, squared);
	displayLabels();
}

void loop()
{
	humidity = bme.readHumidity();
	pressure=bme.readPressure()/100.0F;
	temperature = bme.readTemperature();
	dewPoint = temperature - ((100 - humidity) / 5.);
	skyTemperature=mlx.readObjectTempC();
	tsl.getLuminosity(&broadband, &ir);
	luminosity = broadband - ir;
	rain = analogRead(rainPin);
	weather = getWeather();
	backlight = needBacklight();
	if (backlight) {
		lcd.backlight();
	} else {
		lcd.noBacklight();
	}
	if (rain < rainALARM && !snoozing) {
		soundAlarm();
	} else {
		offAlarm();
	}
	displayValues();
	delay(1000);

}

void soundAlarm() {
	analogWrite(buzzerPin, 127);
}

void offAlarm() {
	analogWrite(buzzerPin, 0);
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
	lcd.setCursor(0, 1);
	lcd.print(humidityLabel);
	lcd.setCursor(0,2);
	lcd.print(dewPointLabel);
	lcd.setCursor(0,3);
	lcd.print(skyTemperatureLabel);
	lcd.setCursor(13, 0);
	lcd.print(pressureLabel);
	lcd.setCursor(13, 1);
	lcd.print(lightLabel);
	lcd.setCursor(13, 2);
	lcd.print(rainLabel);
	lcd.setCursor(10, 3);
	lcd.print(weatherLabel);
}

void displayValues() {
	lcd.setCursor(2, 0);
	lcd.print(spacePad(temperature, 5));
	lcd.write((uint8_t)DEGREEC);
	Serial.print("Temperature: "); Serial.println(temperature);
	lcd.setCursor(2, 1);
	lcd.print(spacePad(humidity, 5));
	lcd.print("%");
	Serial.print("Humidity: "); Serial.println(humidity);
	lcd.setCursor(2, 2);
	lcd.print(dewPoint);
	lcd.write((uint8_t)DEGREEC);
	Serial.print("Dew Point: "); Serial.println(dewPoint);
	lcd.setCursor(2, 3);
	lcd.print(spacePad((float)skyTemperature, 5));
	lcd.write((uint8_t)DEGREEC);
	Serial.print("Sky Temperature: "); Serial.println(skyTemperature);

	lcd.setCursor(14, 0);	
	lcd.print(spacePad(int(pressure), 5));
	lcd.write((uint8_t)MB);
	Serial.print("Pressure: "); Serial.println(pressure);
	lcd.setCursor(14, 1);	
	lcd.print(spacePad((int)luminosity, 5));
	Serial.print("Luminosity: "); Serial.println(luminosity);
	lcd.setCursor(14, 2);
	lcd.print(spacePad(getRain(), 5));
	Serial.print("Rain: "); Serial.println(getRain());
	lcd.setCursor(12, 3);
	lcd.print(spacePad(weather, 5));
	Serial.print("Weather: "); Serial.println(weather);
	Serial.println("\n\n\n");
}

void fillTrend(float *trend, float value) {
	for (int i = 0; i < 48; i++) {
		trend[i] = value;
	}
}

String spacePad(String _in, uint8_t _len) {
	return pad(_in, spaces, _len);
}

String spacePad(float _in, uint8_t _len) {
	return pad(_in, spaces, _len);
}


String zeroPad(int _in, uint8_t _len) {
	return pad(_in, zeros, _len);
}

String spacePad(int _in, uint8_t _len) {
	return pad(_in, spaces, _len);
}

String pad(float _in, String _pad, uint8_t _len) {
	String theString = _pad + String(_in, 2);
	theString = theString.substring(theString.length() - _len);
	return theString;
}

String pad(int _in, String _pad, uint8_t _len) {
	String theString = _pad + _in;
	theString = theString.substring(theString.length() - _len);
	return theString;
}

String pad(String _in, String _pad, uint8_t _len) {
	String theString = _pad + _in;
	theString = theString.substring(theString.length() - _len);
	return theString;
}

String getWeather() {
	if (rain < rainLIGHT) {
		return "RAIN";
	} else {
		return "None.";
	}
}

String getRain() {
	if (rain < rainDOWNPOUR) {
		return " POUR";
	} else if (rain < rainHEAVY) {
		return "HEAVY";
	} else if (rain < rainRAIN) {
		return " RAIN";
	} else if (rain < rainLIGHT) {
		return "LIGHT";
	} else if (rain < rainMIST) {
		return " MIST";
	} else return "  DRY";
}