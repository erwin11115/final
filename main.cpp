#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"
#include <cmath>





Ticker servo_ticker;
Ticker encoder_ticker;
PwmOut pin5(D5), pin6(D6);

BufferedSerial uart(D1,D0);
BufferedSerial xbee(D10, D9);
DigitalIn encoder(D11);
DigitalInOut pin10(D12);
BBCar car(pin5, pin6, servo_ticker);

void set_cm(Arguments *in, Reply *out);
RPCFunction rpcset_cm(&set_cm, "set_cm");

void park(Arguments *in, Reply *out);
RPCFunction rpcPark(&park, "park");

void trace(Arguments *in, Reply *out);
RPCFunction rpcTrace(&trace, "trace");

void findTag(Arguments *in, Reply *out);
RPCFunction rpcfindTag(&findTag, "findTag");

int mode;

volatile int steps;
volatile int last;

void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}

void findTag(Arguments *in, Reply *out) {
   float x_pos = in->getArg<int>();
   int theta = in->getArg<int>();
   printf("in\n\r");

   if (mode != 1 && mode != 2) return;
   mode = 2;

   if (1.5 <= x_pos) { // turn right
      car.turn_normp(25, 0.4);
   } else if (-1.5 >= x_pos) { //
      car.turn_normp(25, -0.4);
   } else if (-1.5 < x_pos && x_pos < 1.5) { //
      car.turn_normp(20, 1);
   }  else {
      car.stop();
   }

   /*
   if ( 350 <= theta && theta < 360) { // turn right
      car.turn_normp(20, -0.5);
   } else if (340 <= theta && theta < 350) { //
      car.turn_normp(20, -0.95);
   } else if (0 <= theta && theta < 10) { // turn left  
      car.turn_normp(20, 0.5);
   } else if (10 <= theta && theta < 20) { // turn left
      car.turn_normp(20, 0.95);
   } else {
      car.stop();
   }
   */

   //ThisThread::sleep_for(20ms);
   printf("theta = %d\r\n", theta);
   //return;
}

void trace(Arguments *in, Reply *out) {
   int x_pos = in->getArg<int>();
   int theta = in->getArg<int>();
   double cos_theta;

   char buffer[200], outbuf[256];
   char strings[20];
   if (mode != 0) return;

   if (0 < theta && theta < 90) { // turn right
      printf("right");
      if (x_pos < 80) {
         printf(" right\r\n");
         car.turn_normp(20, -0.3);
      } else {

         printf(" striagh\r\n");
         car.turn_normp(20, 1);
      }
   } else if (180 > theta && theta > 90) { // turn left
      printf("left\r\n");
      if (x_pos > 60) {
         printf("left\r\n");
         car.turn_normp(20, 0.3);
      } else {
         printf(" striagh\r\n");
         car.turn_normp(20, 1);
      }
   }  else {
      car.stop();
   }
}

void park(Arguments *in, Reply *out) {
   int d1 = in->getArg<int>();
   int d2 = in->getArg<int>();
   char dir = in->getArg<char>();

   char buffer[200], outbuf[256];
   char strings[20];

   if (dir == 'E') {
      d1 += 28;
      d2 += 35;

      sprintf(strings, "/turn/run 55 0.1 \n", d2);
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(7000ms);

/*
      RPC::call("/turn/run 90 0.1 \n", outbuf);
      ThisThread::sleep_for(1500ms);
      RPC::call("/stop/run \n"), outbuf);
*/

      sprintf(strings, "/turn/run 90 0.1 \n");
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(1500ms);
      sprintf(strings, "/stop/run \n");
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);

      sprintf(strings, "/set_cm/run %d \n", d1);
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(8000ms);


   } else if (dir == 'W') {

      d1 += 28;
      d2 += 35;

      sprintf(strings, "/set_cm/run %d \n", d2);
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(7000ms);

      //RPC::call("/turn/run 90 -0.2 \n", outbuf);
      sprintf(strings, "/turn/run 90 -0.2 \n");
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(950ms);
      //RPC::call("/stop/run \n"), outbuf);
      sprintf(strings, "/stop/run \n");
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      

      sprintf(strings, "/set_cm/run %d \n", d1);
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(8000ms);
   } else if (dir = 'S') {
      d1 = d1 + 28;
      
      sprintf(strings, "/set_cm/run %d \n", d2);
      strcpy(buffer, strings);
      RPC::call(buffer, outbuf);
      ThisThread::sleep_for(8000ms);
   }
}

void set_cm(Arguments *in, Reply *out) {
   int len = in->getArg<int>();
   len = len - 6;
   
   steps = 0;
   last = 0;
   car.goStraight(-55);
   //int len =  50;
   while(steps*6.5*3.14/32< len) {
      //printf("encoder = %d\r\n", steps);
      ThisThread::sleep_for(100ms);
   }
   car.stop();
}

int main() {

   encoder_ticker.attach(&encoder_control, 10ms);

   char buf[256], outbuf[256];
   uart.set_baud(9600);

   //FILE *devin = fdopen(&xbee, "r");
   //FILE *devout = fdopen(&xbee, "w");


   parallax_ping  ping1(pin10);
   mode = 0;
   xbee.write("mode 0\r\n", 8);
   while(1) {
      printf("%d\r\n", (int)ping1);


      //printf("where?\r\n", buf);
      if((int)ping1>25){

         for (int i=0; ; i++) {
            char *recv = new char[1];
            //printf("stop?\r\n");
            uart.read(recv, 1);
            buf[i] = *recv;
            if (*recv == '\n') {
               break;
            }
         }

         printf("%s\r\n", buf);
         RPC::call(buf, outbuf);
      } else {
         if (mode == 0) {
            car.stop();
            printf("out\r\n");
            mode = 1;
            car.turn_normp(20, 0.1);
            xbee.write("mode 1\r\n", 8);
         }
         if (mode == 2) {
            car.stop();
            mode = 3;
            xbee.write("stop\r\n", 6);
         }

         
      }
      ThisThread::sleep_for(10ms);
   }




}