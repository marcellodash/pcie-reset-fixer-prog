/*
killall cu
cu -l /dev/ttyACM0 -s 19200
arduino --upload ~/Arduino/sketch_aug06b/sketch_aug06b.ino --port /dev/ttyACM0
 */

#include <assert.h>

#define MAGIC "mc"
const int MOSFET = 8;
const int RXLED = 17;

void handleSerial();
bool getSwitch();
void setGpu(bool value);
bool getGpu();
void rxled(bool value); 
void txled(bool value); 

/**
 *
 */
void loop()
{
  auto sw = getSwitch();

  handleSerial();
 
  if(0)
  {
     setGpu(sw); 
     txled(sw);
  }
}

void setup()
{
   pinMode(MOSFET, OUTPUT);
   digitalWrite(MOSFET, HIGH);
  
   pinMode(RXLED, OUTPUT);  // Set RX LED as an output
   // TX LED is set as an output behind the scenes

   // Power on GPU
   setGpu(true);

   rxled(false);
   txled(false);

   Serial.begin(9600); //This pipes to the serial monitor
   Serial1.begin(9600); //This is the UART, pipes to sensors attached to board
}

void cmdExec(int cmd)
{
   switch(cmd)
   {
      case 0: // Power off GPU
         setGpu(false);
	 Serial.println("-");
         break;
     
      case 1: // Power on GPU
         setGpu(true);
	 Serial.println("+");
         break;

      case 2: // Get GPU power status
         Serial.println(getGpu() ? '+' : '-');
         break;

      case 3:
         Serial.println(analogRead(0), DEC);
         break;

      case 4:
         Serial.println(analogRead(1), DEC);
         break;
      
      case 5:
         Serial.println(analogRead(2), DEC);
         break;

      case 9: // Ping
         Serial.println("0"); // Pong
         break;
    }
}

void handleSerial()
{
  static int state = 0;
  static int cmd = 0;
  const char *magic = MAGIC;
  static int s = 0;

  if(s <= 0)
  {
     s = strlen(MAGIC);
  }

  while(Serial.available())
  {
     auto c = Serial.read();
     if(state < s)
     {
        if(c == magic[state])
        {
           state++;
        }
        else
        {
           state = 0;
        }
     }
     else if(state == s)
     {
        cmd = c - '0';
        state++;   
     }
     else 
     {
        if((c == '\n') || (c == '\r'))
        {
	   Serial.print("*");
	   Serial.println(cmd, DEC);
           state = 0;
           cmdExec(cmd);
        }
        else 
        {
           state = 0;
        }
     }
  }
}

void sendCommand(int cmd)
{
  String a(cmd);
  Serial.println(a);
}

void rxled(bool value) 
{
  digitalWrite(RXLED, value ? HIGH : LOW);
}

void txled(bool value) 
{
  if(value) 
    TXLED1; 
  else
    TXLED0; 
}


static bool gpuPower = false;

bool getGpu()
{
  return gpuPower;
}

void setGpu(bool value)
{
  digitalWrite(MOSFET, value ? HIGH : LOW);
  gpuPower = value;
}

bool getSwitchDebounce(bool reading, long long &state) {
  const int debounceDelay = 100;

  struct DebounceStruct 
  {
     unsigned long time;
     char init;
     char current;
     char last;
  };

  assert(sizeof(DebounceStruct) <= sizeof(state));

  auto ds = (DebounceStruct *)&state;

  if(!ds->init)
  {
     // Initialization
     ds->init    = true;
     ds->current = char(reading);
     ds->last    = char(reading);
     ds->time    = 0;
  }
  
  if(reading != bool(ds->last))
  {
     ds->last = char(reading);

     if(ds->time == 0)
     {
        ds->current = ds->last;
     }  

     // Start/restart bouncing window time
     ds->time = millis();
  }
  else if(ds->time > 0)
  {
     // Bouncing window time
     if(millis() > (ds->time + debounceDelay))
     {
        // End bouncing window time
        ds->time = 0;
     }
  }

  return bool(ds->current);
}

bool getSwitch()
{
  static long long state = 0;
  auto v = analogRead(3);
  return getSwitchDebounce((v < 20), state);
}

