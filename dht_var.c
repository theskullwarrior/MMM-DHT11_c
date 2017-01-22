
/*
 *      dht11.c:
 *  Simple test program to test the wiringPi functions
 *  Based on the existing dht22.c
 *  Amended by skulwarrior 
 */

#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXTIMINGS 85
static int  DHTPIN = 7;
static int dht22_dat[5] = {0,0,0,0,0};

static uint8_t sizecvt(const int read)
{
  /* digitalRead() and friends from wiringpi are defined as returning a value
  < 256. However, they are returned as int() types. This is a safety function */

  if (read > 255 || read < 0)
  {
    printf("Invalid data from wiringPi library\n");
    exit(EXIT_FAILURE);
  }
  return (uint8_t)read;
}

static int read_dht22_dat()
{
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;

  dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

  // pull pin down for 18 milliseconds
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  delay(10);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  // then pull it up for 40 microseconds
  digitalWrite(DHTPIN, LOW);
  delayMicroseconds(40); 
  // prepare to read the pin
  pinMode(DHTPIN, INPUT);

  // detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (sizecvt(digitalRead(DHTPIN)) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = sizecvt(digitalRead(DHTPIN));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      dht22_dat[j/8] <<= 1;
      if (counter > 16)
        dht22_dat[j/8] |= 1;
      j++;
    }
  }

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
  if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
        float h = (float)((dht22_dat[0] << 8) + dht22_dat[1]) / 10;
	if ( h > 100 )
		{
		h = dht22_dat[0];	// for DHT11
		}
        float t = (float)(((dht22_dat[2] & 0x7F) << 8) + dht22_dat[3]) / 10;
	if ( t > 125 )
		{
		t = dht22_dat[2];	// for DHT11
		};
        if ((dht22_dat[2] & 0x80))  t = -1;

  /*
     *printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
     */
     printf("%.1f,%.1f\n", h, t );
    return 1;
  }
  else
  {
    /*
     *printf("Data not good, skip\n");
     */
    return 0;
  }
}

int main (int argc, char *argv[])
{
  int tries = 100;

  if (argc < 2)
    printf ("usage: %s <pin> (<tries>)\ndescription: pin is the wiringPi pin number\nusing 2 (GPIO 27)\nOptional: tries is the number of times to try to obtain a read (default 100)",argv[0]);
  else
    DHTPIN = atoi(argv[1]);
   

  if (argc == 3)
    tries = atoi(argv[2]);

  if (tries < 1) {
    printf("Invalid tries supplied\n");
    exit(EXIT_FAILURE);
  }

  if (wiringPiSetup () == -1)
    exit(EXIT_FAILURE) ;
  
  if (setuid(getuid()) < 0)
  {
    perror("Dropping privileges failed\n");
    exit(EXIT_FAILURE);
  }

  while (read_dht22_dat() == 0 && tries--) 
  {
     delay(1000); // wait 1sec to refresh
  }

  return 0 ;
}
