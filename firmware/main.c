/* Name: main.c
 * Project: hid-key, a very simple keyboard
 * Author: Takanobu MURAGUCHI
 * Creation Date: 2020-04-17
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.

We use VID/PID 0x046D/0xC00E which is taken from a Logitech mouse. Don't
publish any hardware using these IDs! This is for demonstration only!
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "usb_hid_keys.h"


// current data (2 buffers)
static volatile uint8_t fbk_data[9][2];
// old data 
static volatile uint8_t fbk_data_old[9];

// fbk data index
static volatile uint8_t fbk_data_index;


static volatile uint8_t fbk_data_diff;

#define DDRFBK  DDRC
#define PORTFBK PORTC
#define PINFBK  PINC

#define FBK_RST  PC4
#define FBK_CKEB PC5

// 72 keys on FBK
const uint8_t fbk_hid[72] 
PROGMEM = {KEY_F8  , KEY_ENTER, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ZENKAKUHANKAKU, KEY_RIGHTSHIFT, KEY_BACKSLASH, KEY_BACKSPACE,
	   KEY_F7  , 0xce/*@*/, KEY_SEMICOLON, KEY_SEMICOLON , KEY_MINUS         , KEY_SLASH     , KEY_EQUAL    , KEY_GRAVE    ,
	   KEY_F6  , KEY_O    , KEY_L        , KEY_K         , KEY_DOT           , KEY_COMMA     , KEY_P        , KEY_0        ,
	   KEY_F5  , KEY_I    , KEY_U        , KEY_J         , KEY_M             , KEY_N         , KEY_9        , KEY_8        ,
	   KEY_F4  , KEY_Y    , KEY_G        , KEY_H         , KEY_B             , KEY_V         , KEY_7        , KEY_6        ,
	   KEY_F3  , KEY_T    , KEY_R        , KEY_D         , KEY_F             , KEY_C         , KEY_5        , KEY_4        ,
	   KEY_F2  , KEY_W    , KEY_S        , KEY_A         , KEY_X             , KEY_Z         , KEY_E        , KEY_3        ,
	   KEY_F1  , KEY_ESC  , KEY_Q        , KEY_LEFTCTRL  , KEY_LEFTSHIFT     , KEY_LEFTALT   , KEY_1        , KEY_2        ,
	   KEY_HOME, KEY_UP   , KEY_RIGHT    , KEY_LEFT      , KEY_DOWN          , KEY_SPACE     , KEY_DELETE   , KEY_INSERT   };

void fbk_init();
void fbk_get_data();



void fbk_init()
{
  DDRFBK  = 1<<FBK_RST | 1<<FBK_CKEB;
  PORTFBK = 1<<FBK_RST | 0<<FBK_CKEB;
  for (int l=0;l<9;l++){
    fbk_data_old[l]=0;
    fbk_data[l][0]=0;
    fbk_data[l][1]=0;
  }
  fbk_data_index=0;
  fbk_data_diff=0;
}
void fbk_get_data(){
  // De-assert reset
  PORTFBK = 1<<FBK_RST | 0<<FBK_CKEB;
  _delay_us(90);
    
  // De-assert reset
  PORTFBK = 0<<FBK_RST | 0<<FBK_CKEB;

  for(int l=0;l<9;l++){
    _delay_us(90);
    // first nibble
    fbk_data[l][fbk_data_index]  = (PINFBK&0xf);
    // CKEB = 1
    PORTFBK = 0<<FBK_RST | 1<<FBK_CKEB;
    _delay_us(90) ;   

    // second nibble
    fbk_data[l][fbk_data_index] |= ((PINFBK&0xf)<<4);
    // CKEB = 0
    PORTFBK = 0<<FBK_RST | 0<<FBK_CKEB;

    // check difference
    if(fbk_data_diff==0 && (fbk_data[l][0]==fbk_data[l][1]) && (fbk_data[l][0]!=fbk_data_old[l]))
      fbk_data_diff=1;
  }
  // Assert Reset
  PORTFBK = 1<<FBK_RST | 0<<FBK_CKEB;
  // toggle fbk_data_index
  fbk_data_index=((fbk_data_index+1)&1);
}


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
const char usbDescriptorHidReport[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] 
  PROGMEM = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION  
};







typedef struct{
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycode[6];
}keyboard_report_t;

static keyboard_report_t keyboard_report;

volatile static uchar LED_state= 0xff;
static uchar    idleRate;   /* repeat rate for keyboards */



usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  usbRequest_t    *rq = (void *)data;
  
  /* The following requests are never used. But since they are required by
   * the specification, we implement them in this example.
   */
  if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
    DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */

    switch(rq->bRequest){
    case USBRQ_HID_GET_REPORT:
      usbMsgPtr = (void *)&keyboard_report;
      keyboard_report.modifier = 0;
      keyboard_report.keycode[0] = 0;
      return sizeof(keyboard_report);
    case USBRQ_HID_SET_REPORT:
      return (rq->wLength.word==1?USB_NO_MSG:0);
    case USBRQ_HID_GET_IDLE:
      usbMsgPtr = &idleRate;
      return 1;
    case USBRQ_HID_SET_IDLE:
      idleRate = rq->wValue.bytes[1];
      return 0;
    }
  }
  return 0;
}

#define NUM_LOCK 1
#define CAPS_LOCK 2
#define SCROLL_LOCK 4

usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len){
  if (data[0]==LED_state)
    return 1;
  else
    LED_state = data[0];
  //if(LED_state & CAPS_LOCK)
  //  PORTB |= 1 << PB0;
  //else
  //  PORTB &= ~(1 << PB0);
  return 1;
}

void buildReport();
void buildReport(){

  // keycode 6 bytes
  int key_len=0;
  for (int l=0;(l<9)&&(key_len<6);l++){
    for (int s=0;(s<8)&&(key_len<6);s++){
      if (fbk_data[l][0]&(1<<s)) {
	keyboard_report.keycode[key_len] = (uint8_t)pgm_read_byte(&fbk_hid[(l<<3)+s]);
	key_len++;
      }
    }
  }
  for (int e=key_len;e<6;e++){
    keyboard_report.keycode[e] = 0x00;
  }

  // modifier
  
  // RIGHT SHIFT
  if ( fbk_data[0][0]&(1<<5))
    keyboard_report.modifier = KEY_MOD_RSHIFT;
  else
    keyboard_report.modifier =  0x00;
  // LEFT CTRL
  if ( fbk_data[7][0]&(1<<3))
    keyboard_report.modifier |= KEY_MOD_LCTRL;
  // LEFT SHIFT
  if ( fbk_data[7][0]&(1<<4))
    keyboard_report.modifier |= KEY_MOD_LSHIFT;
  // LEFT ALT
  if ( fbk_data[7][0]&(1<<5))
    keyboard_report.modifier |= KEY_MOD_LALT;
  
}





/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
uchar   i;

    wdt_enable(WDTO_1S);
    /* If you don't use the watchdog, replace the call above with a wdt_disable().
     * On newer devices, the status of the watchdog (on/off, period) is PRESERVED
     * OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    //odDebugInit();
    //DBG1(0x00, 0, 0);       /* debug output: main starts */
    DDRB |= 1<<PB0;
    fbk_init();
    
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    //DBG1(0x01, 0, 0);       /* debug output: main loop starts */
    for(;;){                /* main event loop */
      //DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
	fbk_get_data();
	//if ( (fbk_data[2]&0x1))
	//  PORTB |= 1 << PB0;
	//else
	//  PORTB &= ~(1<<PB0);
        usbPoll();

	if(fbk_data_diff){
	  PORTB|=1;
	}else{
	  PORTB&=~(1);
	}
	if(usbInterruptIsReady()){
	  if (fbk_data_diff==1){
	    buildReport();
	    usbSetInterrupt((void *)&keyboard_report, sizeof(keyboard_report));
	    for (int l=0;l<9;l++) {
	      fbk_data_old[l]=fbk_data[l][0];
	    }
	    fbk_data_diff=0;
	  }
            /* called after every poll of the interrupt endpoint */
        }
    }
}

/* ------------------------------------------------------------------------- */
