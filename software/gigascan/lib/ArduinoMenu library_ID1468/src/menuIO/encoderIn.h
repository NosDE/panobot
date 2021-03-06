/* -*- C++ -*- */
/********************
Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
creative commons license 3.0: Attribution-ShareAlike CC BY-SA
This software is furnished "as is", without technical support, and with no
warranty, express or implied, as to its usefulness for any purpose.

Thread Safe: No
Extendable: Yes

quadrature encoder driver (PCINT)
quadrature encoder stream (fake, not using buffers)
*/

#ifndef RSITE_ARDUINO_MENU_ENCODER
  #define RSITE_ARDUINO_MENU_ENCODER

  #include <pcint.h> //https://github.com/neu-rah/PCINT
  #include "../menu.h"

  namespace Menu {

    template<uint8_t pinA,uint8_t pinB>
    class encoderIn {
    public:
      volatile int pos=0;
      //int pinA,pinB;
      //encoderIn<pinA,pinB>(int a,int b):pinA(a),pinB(b) {}
      void begin() {
        pinMode(pinA, INPUT);
        digitalWrite(pinA,1);
        pinMode(pinB, INPUT);
        digitalWrite(pinB,1);
        //attach pin change handlers
        PCattachInterrupt<pinA>(mixHandler((void(*)(void*))encoderInUpdateA,this), CHANGE);
        PCattachInterrupt<pinB>(mixHandler((void(*)(void*))encoderInUpdateB,this), CHANGE);
      }
      //PCint handlers
      static void encoderInUpdateA(class encoderIn<pinA,pinB> *e);
      static void encoderInUpdateB(class encoderIn<pinA,pinB> *e);
    };

    //PCint handlers
    template<uint8_t pinA,uint8_t pinB>
    void encoderIn<pinA,pinB>::encoderInUpdateA(class encoderIn<pinA,pinB> *e) {
      if (digitalRead(pinA)^digitalRead(pinB)) e->pos--;
      else e->pos++;
    }
    template<uint8_t pinA,uint8_t pinB>
    void encoderIn<pinA,pinB>::encoderInUpdateB(class encoderIn<pinA,pinB> *e) {
      if (digitalRead(pinA)^digitalRead(pinB)) e->pos++;
      else e->pos--;
    }

    //emulate a stream based on encoderIn movement returning +/- for every 'sensivity' steps
    //buffer not needer because we have an accumulator
    template<uint8_t pinA,uint8_t pinB>
    class encoderInStream:public Stream {
    public:
      encoderIn<pinA,pinB> &enc;//associated hardware encoderIn
      int sensivity;
      int oldPos=0;
      encoderInStream(encoderIn<pinA,pinB> &enc,int sensivity):enc(enc), sensivity(sensivity) {}
      inline void setSensivity(int s) {sensivity=s;}
      int available(void) {return abs(enc.pos-oldPos)/sensivity;}
      int peek(void) override {
        int d=enc.pos-oldPos;
        if (d<=-sensivity)return options->navCodes[downCmd].ch;
        if (d>=sensivity) return options->navCodes[upCmd].ch;
        return -1;
      }
      int read() override {
        int d=enc.pos-oldPos;
        if (d<=-sensivity) {
          oldPos-=sensivity;
          return options->navCodes[downCmd].ch;
        }
        if (d>=sensivity) {
          oldPos+=sensivity;
          return options->navCodes[upCmd].ch;
        }
        return -1;
      }
      void flush() {oldPos=enc.pos;}
      size_t write(uint8_t v) {oldPos=v;return 1;}

    };

  }//namespace Menu
#endif
