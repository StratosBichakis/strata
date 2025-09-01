/***************************************************/
/*! \class LatelyBass
    \brief STK Lately Bass FM synthesis instrument.

    This class implements 3 carriers and a common
    modulator, also referred to as algorithm 2 of
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


#include "LBass.h"

namespace stk {

LatelyBass :: LatelyBass( void )
  : FM()
{
  // Concatenate the STK rawwave path to the rawwave files
  waves_[0] = new FileLoop( (Stk::rawwavePath() + "sinewave.raw").c_str(), true );
  waves_[1] = new FileLoop( (Stk::rawwavePath() + "sinewave.raw").c_str(), true );
  waves_[2] = new FileLoop( (Stk::rawwavePath() + "sineblnk.raw").c_str(), true );
  waves_[3] = new FileLoop( (Stk::rawwavePath() + "sinewave.raw").c_str(), true );

  curve_.resize( nOperators_ );
  for (int j=0; j<nOperators_; j++ ) {
    curve_[j] = new Asymp();
  }
  curve_[0]->setTau(0.813);
  curve_[1]->setTau(0.813);
  curve_[2]->setTau(0.813);
  curve_[3]->setTau(0.764);

  curve_[0]->setTime(2.279);
  curve_[1]->setTime(2.279);
  curve_[2]->setTime(0.145);
  curve_[3]->setTime(0.218);


  this->setRatio( 0, 0.5 );
  this->setRatio( 1, 0.5 );
  this->setRatio( 2, 0.9996 );
  this->setRatio( 3, 1.000 );

  gains_[0] = fmGains_[99];
  gains_[1] = fmGains_[71];
  gains_[2] = fmGains_[74];
  gains_[3] = fmGains_[79];

  adsr_[0]->setAllTimes( 0.001, 2.278, 0.0, 0.01 );
  adsr_[1]->setAllTimes( 0.001, 2.278, 0.0, 0.01 );
  adsr_[2]->setAllTimes( 0.001, 0.144, 0.0, 0.01 );
  adsr_[3]->setAllTimes( 0.001, 0.217, 0.016, 5.152 );

  twozero_.setGain( 1.0 );
}  

LatelyBass :: ~LatelyBass( void )
{
}

void LatelyBass :: setFrequency( StkFloat frequency )
{
#if defined(_STK_DEBUG_)
  if ( frequency <= 0.0 ) {
    oStream_ << "LatelyBass::setFrequency: argument is less than or equal to zero!";
    handleError( StkError::WARNING ); return;
  }
#endif

  baseFrequency_ = frequency;
  waves_[0]->setFrequency( baseFrequency_ * ratios_[0] );
  waves_[1]->setFrequency( baseFrequency_ * ratios_[1] );
  waves_[2]->setFrequency( ( baseFrequency_ * ratios_[2] ) - 0.22 );
  waves_[3]->setFrequency( baseFrequency_ * ratios_[3] );
}

void LatelyBass :: noteOn( StkFloat frequency, StkFloat amplitude )
{
  gains_[0] = fmGains_[99];
  gains_[1] = fmGains_[71];
  gains_[2] = fmGains_[74];
  gains_[3] = fmGains_[79];

  this->setFrequency( frequency );
  this->keyOn();

  for (int j=0; j<nOperators_; j++ ) {
    curve_[j]->setValue(1);
    curve_[j]->keyOff();
  }
}

} // stk namespace
