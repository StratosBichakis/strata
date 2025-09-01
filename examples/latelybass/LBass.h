#ifndef STK_LBASS_H
#define STK_LBASS_H

#include "FM.h"
#include "Asymp.h"

namespace stk {

/***************************************************/
/*! \class LBass
    \brief STK Lately Bass FM synthesis instrument.

    This class implements 2 modulators modulating 
    another modulator and a carrier,
    also referred to as algorithm 2 of
    the TX81Z.

    \code
    Algorithm 2 is :
                        /->3 -\+->2->1 -> Out
                        \->4 -/
    \endcode

    Control Change Numbers: 
       - Operator 4 (feedback) Gain = 2
       - Operator 3 Gain = 4
       - LFO Speed = 11
       - LFO Depth = 1
       - ADSR 2 & 4 Target = 128

    The basic Chowning/Stanford FM patent expired
    in 1995, but there exist follow-on patents,
    mostly assigned to Yamaha.  If you are of the
    type who should worry about this (making
    money) worry away.

    by Perry R. Cook and Gary P. Scavone, 1995--2023.
*/
/***************************************************/

class LatelyBass : public FM
{
 public:
  //! Class constructor.
  /*!
    An StkError will be thrown if the rawwave path is incorrectly set.
  */
  LatelyBass( void );

  //! Class destructor.
  ~LatelyBass( void );

  //! Set instrument parameters for a particular frequency.
  void setFrequency( StkFloat frequency );

  //! Start a note with the given frequency and amplitude.
  void noteOn( StkFloat frequency, StkFloat amplitude );

  //! Compute and return one output sample.
  StkFloat tick( unsigned int channel = 0 );

  //! Fill a channel of the StkFrames object with computed outputs.
  /*!
    The \c channel argument must be less than the number of
    channels in the StkFrames argument (the first channel is specified
    by 0).  However, range checking is only performed if _STK_DEBUG_
    is defined during compilation, in which case an out-of-range value
    will trigger an StkError exception.
  */
  StkFrames& tick( StkFrames& frames, unsigned int channel = 0 );

 protected:
 std::vector<Asymp *> curve_;
};

inline StkFloat LatelyBass :: tick( unsigned int )
{
  StkFloat temp;

  if ( modDepth_ > 0.0 )  {
    temp = 1.0 + ( modDepth_ * vibrato_.tick() * 0.15 );
    waves_[0]->setFrequency( baseFrequency_ * temp * ratios_[0] );
    waves_[1]->setFrequency( baseFrequency_ * temp * ratios_[1] );
    waves_[2]->setFrequency((baseFrequency_ * temp * ratios_[2])-0.22 );
    waves_[3]->setFrequency( baseFrequency_ * temp * ratios_[3] );
  }

  waves_[3]->addPhaseOffset( waves_[3]->lastOut() * 0.1 ); 
  temp =  curve_[3]->tick() * gains_[3] * adsr_[3]->tick() * waves_[3]->tick();
  twozero_.tick(temp);
  temp += curve_[2]->tick() * gains_[2] * adsr_[2]->tick() * waves_[2]->tick();

  waves_[1]->addPhaseOffset( temp );
  temp = curve_[1]->tick() * gains_[1] * adsr_[1]->tick() * waves_[1]->tick();

  waves_[0]->addPhaseOffset( temp );
  temp = curve_[0]->tick() * gains_[0] * adsr_[0]->tick() * waves_[0]->tick();

  lastFrame_[0] = temp;
  return lastFrame_[0];
}

inline StkFrames& LatelyBass :: tick( StkFrames& frames, unsigned int channel )
{
  unsigned int nChannels = lastFrame_.channels();
#if defined(_STK_DEBUG_)
  if ( channel > frames.channels() - nChannels ) {
    oStream_ << "LatelyBass::tick(): channel and StkFrames arguments are incompatible!";
    handleError( StkError::FUNCTION_ARGUMENT );
  }
#endif

  StkFloat *samples = &frames[channel];
  unsigned int j, hop = frames.channels() - nChannels;
  if ( nChannels == 1 ) {
    for ( unsigned int i=0; i<frames.frames(); i++, samples += hop )
      *samples++ = tick();
  }
  else {
    for ( unsigned int i=0; i<frames.frames(); i++, samples += hop ) {
      *samples++ = tick();
      for ( j=1; j<nChannels; j++ )
        *samples++ = lastFrame_[j];
    }
  }

  return frames;
}

} // stk namespace

#endif
