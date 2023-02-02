
byte ArduinoFDCClass::readNibbles(byte track, byte side, byte mode, int read_delay, byte *buffer)
{
  byte res = S_OK;
  byte driveType = m_driveType[m_currentDrive];

#ifdef DEBUG
pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN,LOW);
#endif
  // do some sanity checks
  if( !m_initialized )
    return S_NOTINIT;
  else if( track>=geometry[driveType].numTracks || side>1 )
    return S_NOHEADER;

  // if motor is not running then turn it on now
  bool turnMotorOff = false;
  if( !motorRunning() )
    {
      turnMotorOff = true;
      motorOn();
    }

  // assert DRIVE_SELECT
  driveSelect(LOW);
  // search track 
  interrupts();
  step_to_track0(); 
  step_tracks(driveType,track);  // relative stepping, track byte here, isn't it integer ?
  //noInterrupts(); // don 10 lines later

  // get MFM bit length (in processor cycles, motor must be running for this)
  byte bitLength = getBitLength();

  res = wait_index_hole();
	  
    if( bitLength==0 )
    res = S_NOTREADY;
  else
    {
      // wait delay ms
      delay(read_delay);
      // set up timer
      TCCRA = 0;
      TCCRB = bit(CS0); // falling edge input capture, prescaler 1, no output compare
      TCCRC = 0;
      
      // reading data is time sensitive so we can't have interrupts
      noInterrupts();

      // find the requested sector
      // res = find_sector(driveType, bitLength, track, side, sector);
      
      // if we found the sector then read the data
          // wait for data sync mark and read data
          //res= read_data_gcr(bitLength, buffer, 5150, false) ;
 #ifdef try_flux  // this is buggy
       short int i;
       i=0;
       while ( i < BUFF_SIZE ) {
       while ( ICF == TIFR) {};
       buffer[i++] =TIFR-ICF ;
       ICF = TIFR;
       }
       Serial.print("flux collected: ");
       Serial.print(buffer[0],HEX);
       Serial.println(buffer[1],HEX);
       Serial.println(buffer[2],HEX);
#else
            Serial.print("readNibbles, scalling read_data_gcr, mode,bitlen=");
               Serial.println(mode);
               Serial.println(bitLength);
               Serial.flush();
          res= read_data_gcr(bitLength, buffer, BUFF_SIZE, false, mode) ;
          //res= read_data_gcr(bitLength, buffer, 512 , false, mode) ;
               Serial.print("read_data return:"); 
               Serial.println(res); 
#endif
      // interrupts are ok again
      interrupts();

      // stop timer
      TCCRB = 0;
    }

  // de-assert DRIVE_SELECT
   driveSelect(HIGH);

  // if we turned the motor on then turn it off again
  if( turnMotorOff ) {
	 motorOff();
  }

  return res;
}
  
