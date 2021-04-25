// This code was taken from: https://code.google.com/archive/p/arduino-xmodem
// (https://code.google.com/archive/p/arduino-xmodem)
// which was released under GPL V3:
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
// -----------------------------------------------------------------------------

#ifndef XMODEM_H
#define XMODEM_H

typedef enum {
	Crc,
	ChkSum	
} transfer_t;


class XModem {
	private:
		//delay when receive bytes in frame - 7 secs
		static const int receiveDelay;
		//retry limit when receiving
		static const int rcvRetryLimit;
		//holds readed byte (due to dataAvail())
		int byte;
		//expected block number
		unsigned char blockNo;
		//extended block number, send to dataHandler()
		unsigned long blockNoExt;
		//retry counter for NACK
		int retries;
		//buffer
		char buffer[133];
		//repeated block flag
		bool repeatedBlock;

		int  (*recvChar)(int);
                void (*sendData)(const char *data, int len);
		bool (*dataHandler)(unsigned long number, char *buffer, int len);
		unsigned short crc16_ccitt(char *buf, int size);
		bool dataAvail(int delay);
		int dataRead(int delay);
		void dataWrite(char symbol);
		bool receiveFrameNo(void);
		bool receiveData(void);
		bool checkCrc(void);
		bool checkChkSum(void);
		bool receiveFrames(transfer_t transfer);
		bool sendNack(void);
		void init(void);
		
		bool transmitFrames(transfer_t);
		unsigned char generateChkSum(const char *buffer, int len);
		
	public:
		static const unsigned char NACK;
		static const unsigned char ACK;
		static const unsigned char SOH;
		static const unsigned char EOT;
		static const unsigned char CAN;
	
		XModem(int (*recvChar)(int), void (*sendData)(const char *data, int len));
		XModem(int (*recvChar)(int), void (*sendData)(const char *data, int len), 
  			        bool (*dataHandler)(unsigned long, char*, int));
		bool receive();
		bool transmit();
		
	
		
};


#endif
