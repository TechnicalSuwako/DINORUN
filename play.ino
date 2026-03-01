void play(byte pin, unsigned int frequency, byte duty, unsigned long duration) {
  unsigned long period = 1000000UL / frequency;
  unsigned long high = period * duty / 100UL;
  unsigned long low = period - high;

  for (unsigned long i = 0; i < duration * 1000UL; i += period) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(high);
    digitalWrite(pin, LOW);
    delayMicroseconds(low);
  }
}
