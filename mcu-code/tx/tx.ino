#include <SPI.h>
#include <math.h>
#include <RF24.h>

#define MAX_BYTES 32
#define RGB_SIZE 3
//#define DEBUG

#ifdef DEBUG
#define pf(...) printf(__VA_ARGS__)
#else
#define pf(...)
#endif

#define dbg(str) void()
#define dbgln(str) void()
#define out(x) Serial.print(x)
#define outh(x) Serial.print(x, HEX)
#define outln(x) Serial.println(x)

// radio(CE, CSN)
RF24 radio(7, 8);

byte addresses[][6] = {"1Node","2Node"};

#define RX 0
#define TX 1

byte multicast = 1; // turns off ack request
bool role = TX;
unsigned long last = 0;

void setup() {
  Serial.setTimeout(50);
  //Serial.begin(115200);
  //while (!Serial) ; // wait for serial monitor
  delay(2000); // WNG - println's below started working after I put in this delay

  radio.begin();
  //radio.setPALevel(RF24_PA_HIGH);
  //radio.setCRCLength(RF24_CRC_16); // XXX haven't tested
  radio.setPALevel(RF24_PA_MAX);
  radio.enableDynamicAck();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.setAutoAck(false);
  radio.setAddressWidth(5);
  radio.setDataRate(RF24_250KBPS); // RF24_2MBPS
  radio.setChannel(125); // XXX

  if (role)
  {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
  } else {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }

  // Start the radio listening for data
  //radio.startListening();
  //radio.printDetails();
}

byte mem[MAX_BYTES];

#define BAD_DATA do { Serial.flush(); return; } while(0)

void readPix()
{
  byte startChar = Serial.read();

  // XXX add timeout? how long does serial timeout take?
  // transmit next byte (T=multicast)
  if (startChar == 'T' || startChar == 't') // T [bytes] <bytes of data to TX>
  {
    byte bytes = Serial.read();

    pf("a1 %d %d ", startChar, bytes);
    // if too big
    if (bytes > MAX_BYTES) BAD_DATA;
    byte in = Serial.readBytes((char *)mem, bytes);
    pf("a2=%d ", in);

    if (in != bytes) BAD_DATA;

    pf("a3 %d\n", startChar == 'T');
    pf("a4 %02x%02x%02x%02x\n", mem[0], mem[1], mem[2], mem[3]);
    radio.write(&mem, bytes, startChar == 'T');
  }
  else if (startChar == '*') // * [RGB]
  {
    mem[0] = '*';
    byte in = Serial.readBytes((char *)mem+1, RGB_SIZE);
    if (in != RGB_SIZE) BAD_DATA;

    radio.write(&mem, 1 + RGB_SIZE, multicast);
  }
  else if (startChar == 'S') // S [starting id] [bytes] [RGB ...]
  {
    mem[0] = 'S';
    //mem[1] = Serial.read(); // id
    //mem[2] = Serial.read(); // bytes
    Serial.readBytes((char *)mem+1, 2); // read in 2 bytes (id + pixels)

    if (3 + mem[2]*RGB_SIZE > MAX_BYTES) BAD_DATA;

    byte in = Serial.readBytes((char *)mem+3, mem[2]*RGB_SIZE); // read in pixels*RGB_SIZE
    if (in != mem[2]*RGB_SIZE) BAD_DATA;

    radio.write(&mem, 3 + RGB_SIZE * mem[2], multicast);
  }
  else if (startChar == '?')
  {
    outln("1,1,1,x,y,0,0,1,1,10,11,12");
  }
  //else Serial.flush();
}

void loop()
{
  readPix();
  /*
  return;

  if (role == TX) {
    radio.stopListening();
    dbgln("Now sending");

    unsigned long start_time = micros(); // Take the time, and send it. This will block until complete
    if (!radio.write( &start_time, sizeof(unsigned long) )){
      dbgln("failed");
    }

    radio.startListening(); // Now, continue listening

    unsigned long started_waiting_at = micros(); 
    boolean timeout = false; 

    while ( ! radio.available() ){ // While nothing is received
      if (micros() - started_waiting_at > 200000 ){ 
        timeout = true;
        break;
      } 
    }

    if ( timeout ){ // Describe the results
      dbgln("Failed, response timed out.");
    }else{
      unsigned long got_time; // Grab the response, compare, and send to debugging spew
      radio.read( &got_time, sizeof(unsigned long) );
      unsigned long end_time = micros();

      // Spew it
      dbg("Sent ");
      dbg(start_time);
      dbg(", Got response ");
      dbg(got_time);
      dbg(", Round-trip delay ");
      dbg(end_time-start_time);
      dbgln(" microseconds");
    }

    // Try again 1s later
    delay(1000);
  }

  // ****************** Pong Back Role ***************************
  else
  {
    unsigned long got_time;

    if( radio.available()){
      // Variable for the received timestamp
      while (radio.available()) { // While there is data ready
        radio.read( &got_time, sizeof(unsigned long) ); // Get the payload
      }
      out(got_time);
      radio.stopListening(); // First, stop listening so we can talk 
      radio.write( &got_time, sizeof(unsigned long) ); // Send the final one back. 
      radio.startListening(); // Now, resume listening so we catch the next packets. 

      last = millis();
      
      for (int i = 0; i < strip.numPixels(); i++)
        strip.setPixelColor(i, got_time % (unsigned long)pow(2, 24));
      strip.show();
     

      dbg("Sent response ");
      dbgln(got_time);
    }
  }

*/

} // Loop
