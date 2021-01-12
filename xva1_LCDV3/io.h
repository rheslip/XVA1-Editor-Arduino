
// Copyright 2020 Rich Heslip
//
// Author: Rich Heslip 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// I/O pin definitions for XVA1/XFM2 esp32 dev board

#ifndef IO_H_
#define IO_H_

// SPI pins
//#define MISO 12
//#define MOSI 11
//#define SCLK 13


// SD card chip select pin
//#define SD_CS 15

// encoder pins
#define ENC_A   5
#define ENC_B   4
#define ENC_SW  36

// 34,35,36,26,21,22,13,14
#define P1ENC_A 21
#define P1ENC_B 22
#define P1_SW   3  // RXD0 input - have to assign it after calling serial init
#define P2ENC_A 26
#define P2ENC_B 14
#define P2_SW   1
#define P3ENC_A 34
#define P3ENC_B 35
#define P3_SW   2
#define P4ENC_A 32
#define P4ENC_B 13
#define P4_SW   12


// potentiometer A/D input ports
#define VOLUMEPOT 39

// MIDI serial port pins
#define MIDIRX 27
#define MIDITX 14

// UART2 pins to FPGA serial port
#define RXD2 16
#define TXD2 17




#endif // IO_H_
