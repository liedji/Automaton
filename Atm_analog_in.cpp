#include "Atm_analog_in.hpp"

Atm_analog_in& Atm_analog_in::begin( int attached_pin, int samplerate /* = 50 */ ) {
  // clang-format off
  const static STATE_TYPE state_table[] PROGMEM = {
    /*              ON_ENTER    ON_LOOP  ON_EXIT  EVT_TIMER   ELSE */
    /* IDLE   */          -1,        -1,      -1,    SAMPLE,    -1,
    /* SAMPLE */  ACT_SAMPLE,        -1,      -1,        -1,  IDLE,
  };
  // clang-format on
  MACHINE::begin( state_table, ELSE );
  pin = attached_pin;
  timer.set( samplerate );
  return *this;
}

Atm_analog_in& Atm_analog_in::range( int toLow, int toHigh ) {
  _toLow = toLow;
  _toHigh = toHigh;
  return *this;    
}

int Atm_analog_in::read_sample() {
  return analogRead( pin );
}

int Atm_analog_in::_avg() {
  uint16_t v = read_sample();
  avg_buf_total = avg_buf_total + v - avg_buf[avg_buf_head];
  avg_buf[avg_buf_head] = v;
  if ( avg_buf_head + 1 >= avg_buf_size ) {
    avg_buf_head = 0;
  } else {
    avg_buf_head++;
  }
  return avg_buf_total / avg_buf_size;
}

int Atm_analog_in::sample() {
  return avg_buf_size > 0 ? _avg() : read_sample();
}

int Atm_analog_in::state( void ) {
  if ( _toHigh ) {
    return map( sample(), 0, 1023, _toLow, _toHigh ); 
  } else {
    return sample();
  }
}

Atm_analog_in& Atm_analog_in::average( uint16_t* v, uint16_t size ) {
  avg_buf = v;
  avg_buf_size = size;
  avg_buf_head = 0;
  avg_buf_total = 0;
  for ( uint16_t i = 0; i < avg_buf_size; i++ ) {
    avg_buf[i] = read_sample();
    avg_buf_total += avg_buf[i];
  }
  return *this;
}

int Atm_analog_in::event( int id ) {
  switch ( id ) {
    case EVT_TIMER:
      return timer.expired( this );
  }
  return 0;
}

void Atm_analog_in::action( int id ) {
  switch ( id ) {
    case ACT_SAMPLE:
      v_previous = v_sample;
      v_sample = sample();
      return;
  }
}

Atm_analog_in& Atm_analog_in::trace( Stream& stream ) {
#ifndef TINYMACHINE
  setTrace( &stream, atm_serial_debug::trace,
            "EVT_TIMER\0ELSE\0"
            "IDLE\0SAMPLE" );

#endif
  return *this;
}
