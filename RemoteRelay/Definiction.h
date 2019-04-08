struct RadioPacket // Any packet up to 32 bytes can be sent.
{
    uint8_t FromRadioId;
    bool Pb1;
    bool Pb2;
    
};

void Blink(int led, int count)    {
  for( int i = 0; i < count ; i++)
  {
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(200);
  }

}

void SetState(uint8_t pin, bool state)
{
	digitalWrite(pin, state ? HIGH : LOW);
}
