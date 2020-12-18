//Send emonPi data to Pi serial /dev/ttyAMA0 using struct JeeLabs RF12 packet structure
void send_emonpi_serial()
{
  byte binarray[sizeof(emonPi)];
  memcpy(binarray, &emonPi, sizeof(emonPi));

  Serial.print(F("OK "));
  Serial.print(nodeID);
  for (byte i = 0; i < sizeof(binarray); i++) {
    Serial.print(F(" "));
    Serial.print(binarray[i]);
  }
  Serial.print(F(" (-0)"));
  Serial.println();

  delay(10);
}

static void showString (PGM_P s) {
  for (;;) {
    char c = pgm_read_byte(s++);
    if (c == 0)
      break;
    if (c == '\n')
      Serial.print('\r');
    Serial.print(c);
  }
}

