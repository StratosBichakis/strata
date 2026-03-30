// threebees.cpp STK tutorial program

#include "Plucked.h"
#include "BiQuad.h"
#include "RtAudio.h"
#include "Messager.h"
#include "Voicer.h"
#include "SKINImsg.h"
#include "stk-config.h"
#include <cmath>

#include <algorithm>
using std::min;

using namespace stk;
#define RAWWAVES_PATH STK_RAWWAVES_DIR "/rawwaves"
// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
  Voicer voicer;
  BiQuad hpf;
  Messager messager;
  Skini::Message message;
  int counter;
  bool haveMessage;
  bool done;

  // Default constructor.
  TickData()
    : counter(0), haveMessage(false), done( false )
  {
    setupHPF(32.0, 0.707);
  }

  // Standard formula for a Second-Order High-Pass Filter
  void setupHPF(StkFloat freq, StkFloat Q) {
    StkFloat w0 = 2.0 * M_PI * freq / Stk::sampleRate();
    StkFloat alpha = sin(w0) / (2.0 * Q);
    StkFloat cosW0 = cos(w0);

    StkFloat b0 =  (1.0 + cosW0) / 2.0;
    StkFloat b1 = -(1.0 + cosW0);
    StkFloat b2 =  (1.0 + cosW0) / 2.0;
    StkFloat a0 =   1.0 + alpha;
    StkFloat a1 =  -2.0 * cosW0;
    StkFloat a2 =   1.0 - alpha;

    hpf.setCoefficients(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0);
  }
};

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks

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
    if ( value2 == 0.0 ) // velocity is zero ... really a NoteOff
      data->voicer.noteOff( value1, 64.0 );
    else { // a NoteOn
      data->voicer.noteOn( value1, value2 );
    }
    break;

  case __SK_NoteOff_:
    data->voicer.noteOff( value1, value2 );
    break;

  case __SK_ControlChange_:
    data->voicer.controlChange( (int) value1, value2 );
    break;

  case __SK_AfterTouch_:
    data->voicer.controlChange( 128, value1 );

  case __SK_PitchChange_:
    data->voicer.setFrequency( value1 );
    break;

  case __SK_PitchBend_:
    data->voicer.pitchBend( value1 );

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
  StkFloat sample;

  int counter, nTicks = (int) nBufferFrames;

  while ( nTicks > 0 && !data->done ) {

    if ( !data->haveMessage ) {
      data->messager.popMessage( data->message );
      if ( data->message.type > 0 ) {
        data->counter = (long) (data->message.time * Stk::sampleRate());
        data->haveMessage = true;
      }
      else
        data->counter = DELTA_CONTROL_TICKS;
    }

    counter = min( nTicks, data->counter );
    data->counter -= counter;

    for ( int i=0; i<counter; i++ ) {
      sample = 0.5*data->voicer.tick();
      sample = data->hpf.tick( sample );

      *samples++ = sample;
      *samples++ = sample;

      nTicks--;
    }


    if ( nTicks == 0 ) break;

    // Process control messages.
    if ( data->haveMessage ) processMessage( data );
  }

  return 0;
}

int main()
{
  // Set the global sample rate and rawwave path before creating class instances.
  Stk::setSampleRate( atof(RTAUDIO_SAMPLE_RATE) );
  Stk::setRawwavePath( RAWWAVES_PATH );

  int i;
  TickData data;
  RtAudio dac;
  Instrmnt *instrument[32];
  for ( i=0; i<32; i++ ) instrument[i] = 0;

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
    // Define and load the BeeThree instruments
    for ( i=0; i<32; i++ )
      instrument[i] = new Plucked(50.0);
  }
  catch ( StkError & ) {
    goto cleanup;
  }

  // "Add" the instruments to the voicer.
  for ( i=0; i<32; i++ )
    data.voicer.addInstrument( instrument[i] );

  if ( data.messager.startSocketInput() == false )
    goto cleanup;

  if ( dac.startStream() ) {
    std::cout << dac.getErrorText() << std::endl;
    goto cleanup;
  }

  // Block waiting until callback signals done.
  while ( !data.done )
    Stk::sleep( 100 );

  // Shut down the callback and output stream.
  dac.closeStream();

 cleanup:
  for ( i=0; i<32; i++ ) delete instrument[i];

  return 0;
}
