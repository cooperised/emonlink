//emonPi startup proceadure, check for AC waveform and print out debug
void emonPi_startup()
{
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);

  pinMode(shutdown_switch_pin,INPUT_PULLUP);            //enable ATmega328 internal pull-up resistors

  pinMode(emonpi_GPIO_pin, OUTPUT);                     //Connected to RasPi GPIO pin 17
  digitalWrite(emonpi_GPIO_pin, LOW);

  pinMode(emonPi_int1_pin, INPUT_PULLUP);               // Set RJ45 interrupt pulse counting pin to input (INT 1) and pullup

  Serial.begin(BAUD_RATE);
  Serial.print(F("emonRX V")); Serial.println(firmware_version*0.01);
  Serial.println(F("Andy Pomfret"));
  Serial.println(F("startup..."));
}
