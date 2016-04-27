# SkyPilot
Observatory sky weather monitor

This project watches the sky above an observatory to notify the user of impending weather.

We use the following hardware to accomplish this task:

* TSL2561 Light Meter
* MLX90614 Infrared Thermometer
* BME280 Barometer
* DHT11 Hygrometer
* HiLetgo Rain Meter
* KS0062 20x4 LCD
* Two analog buttons
* One loud-assed buzzer
* Arduino UNO
* USB B Cable

The light meter will watch for a brightening sky to indicate either clouds or daybreak
or some jackhole with a flashlight. The light meter utilizes a 200 degree door peephole to
watch a large area of the sky. Hopefully, this won't incinerate any hardware if I open
the observatory roof during daylight hours.

The infrared thermometer will watch for the sky temperature to rise above freezing
indicating cloudy skies.

The hygrometer will watch for the dewpoint approaching the ambient temperature to detect
dew formation on optics or impending precipitation

The Barometer will watch for falling pressure indicating a low pressure front approaching

The rain meter will watch for something. I forget.

One analog button is used to turn on the LCD backlight for a timed duration.

The other analog button is used to silence the loud-assed buzzer for an hour.
