// controlbee.cpp STK tutorial program using socket input

#include "LBass.h"
#include "RtAudio.h"
#include "RtWvOut.h"
#include "Messager.h"
#include "SKINImsg.h"
#include <math.h>
#include <algorithm>

#include "stk-config.h"
#define RAWWAVES_PATH STK_RAWWAVES_DIR "/rawwaves"

using std::min;

using namespace stk;

// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
  Instrmnt *instrument;
  // RtWvOut *output;
  Messager messager;
  Skini::Message message;
  int counter;
  bool haveMessage;
  bool done;

  // Default constructor.
  TickData()
    : instrument(0), counter(0), haveMessage(false), done( false ) {}
};

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks

void usage(void) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: latelybass file\n";
  std::cout << "    where file = a SKINI scorefile.\n\n";
  exit(0);
}

// The processMessage() function encapsulates the handling of control
// messages.  It can be easily relocated within a program structure
// depending on the desired scheduling scheme.
void processMessage( TickData* data )
{
  StkFloat value1 = data->message.floatValues[0];
  StkFloat value2 = data->message.floatValues[1];

  switch( data->message.type ) {

  case __SK_Exit_:
    data->done = true;
    return;

  case __SK_NoteOn_:
      if (data->message.channel == 2) {
        if ( value2 == 0.0 ) // velocity is zero ... really a NoteOff
          data->instrument->noteOff( 0.5 );
        else { // a NoteOn
          value1 += 12;
          StkFloat frequency = 220.0 * pow( 2.0, (value1 - 57.0) / 12.0 );
          data->instrument->noteOn( frequency, value2 * ONE_OVER_128 );
        }
      }
    break;

  case __SK_NoteOff_:
    data->instrument->noteOff( value2 * ONE_OVER_128 );
    break;

  case __SK_ControlChange_:
    data->instrument->controlChange( (int) value1, value2 );
    break;

  case __SK_AfterTouch_:
    data->instrument->controlChange( 128, value1 );

  } // end of switch

  data->haveMessage = false;
  return;
}

// This tick() function handles sample computation and scheduling of
// control updates.  It will be called automatically when the system
// needs a new buffer of audio samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
  TickData *data = (TickData *) dataPointer;
  StkFloat *samples = (StkFloat *) outputBuffer;
  int counter, nTicks = (int) nBufferFrames;

  while ( nTicks > 0 && !data->done ) {
    if ( !data->haveMessage ) {
      data->messager.popMessage( data->message );
      if ( data->message.type > 0 ) {
            double tempDouble = data->message.time;
            // if (tempDouble < 0)     {
              // tempDouble = - tempDouble;
              // tempDouble = tempDouble - data->output->getTime();
            // }
            if (tempDouble < 0) {
               printf("Bad News Here!!!  Backward Absolute Time Required.\n");
               tempDouble = 0.0;
            }
        // data->counter = (long) (tempDouble * Stk::sampleRate());
        data->counter = (long) (data->message.time * Stk::sampleRate());
        data->haveMessage = true;
      }
      else
        data->counter = DELTA_CONTROL_TICKS;
    }

    counter = min( nTicks, data->counter );
    data->counter -= counter;

    for ( int i=0; i<counter; i++ ) {
      StkFloat temp = 0.2 * data->instrument->tick();
      for(int ch=0; ch<2;ch++){
        *samples++ = temp;
      }
      nTicks--;
      // data->output->tick(.0);
    }
    if ( nTicks == 0 ) break;

    // Process control messages.
    if ( data->haveMessage ) processMessage( data );
  }

  return 0;
}

int main( int argc, char *argv[] )
{
  if ( argc != 2 ) usage();
  // Set the global sample rate and rawwave path before creating class instances.
  Stk::setSampleRate( 48000.0 );
  Stk::setRawwavePath( RAWWAVES_PATH );

  TickData data;
  RtAudio dac;
  // RtWvOut output;

  // data.output = &output;
  // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  if ( dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&data ) ) {
    std::cout << dac.getErrorText() << std::endl;
    goto cleanup;
  }

  try {
    // Define and load the BeeThree instrument
    data.instrument = new LatelyBass();
  }
  catch ( StkError & ) {
    goto cleanup;
  }

  if ( data.messager.setScoreFile( argv[1] ) == false )
    goto cleanup;
#if false
  if ( data.messager.startSocketInput() == false )
    goto cleanup;
#endif
  if ( dac.startStream() ) {
    std::cout << dac.getErrorText() << std::endl;
    goto cleanup;
  }

  // Block waiting until callback signals done.
  while ( !data.done )
    Stk::sleep( 100 );
  
  // Shut down the output stream.
  dac.closeStream();

 cleanup:
  delete data.instrument;

  return 0;
}
