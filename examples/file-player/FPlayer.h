#ifndef STK_FPLAYER_H
#define STK_FPLAYER_H

#include "ADSR.h"
#include "FileLoop.h"
#include "Instrmnt.h"

namespace stk {
    /***************************************************/
    /*! \class FPlayer

    by Stratos Bichakis 2025
    */
    /***************************************************/
    class FPlayer : public Instrmnt {
    public:
        FPlayer(void);

        ~FPlayer(void);

        void noteOn(StkFloat frequency, StkFloat amplitude);

        //! Start envelopes toward "on" targets.
        void keyOn(void);

        //! Start envelopes toward "off" targets.
        void keyOff(void);

        //! Stop a note with the given amplitude (speed of decay).
        void noteOff(StkFloat amplitude);

        //! Compute and return one output sample.
        StkFloat tick(unsigned int channel = 0);

        //! Fill a channel of the StkFrames object with computed outputs.
        /*!
          The \c channel argument must be less than the number of
          channels in the StkFrames argument (the first channel is specified
          by 0).  However, range checking is only performed if _STK_DEBUG_
          is defined during compilation, in which case an out-of-range value
          will trigger an StkError exception.
        */
        StkFrames &tick(StkFrames &frames, unsigned int channel = 0);

    private:
        ADSR *env_;
        FileLoop *file_;
    };

    inline StkFloat FPlayer::tick(unsigned int) {
        lastFrame_[0] = env_->tick() * file_->tick();;
        return lastFrame_[0];
    }

    inline StkFrames &FPlayer::tick(StkFrames &frames, unsigned int channel) {
        unsigned int nChannels = lastFrame_.channels();
#if defined(_STK_DEBUG_)
        if ( channel > frames.channels() - nChannels ) {
            oStream_ << "FPlayer::tick(): channel and StkFrames arguments are incompatible!";
            handleError( StkError::FUNCTION_ARGUMENT );
        }
#endif

        StkFloat *samples = &frames[channel];
        unsigned int j, hop = frames.channels() - nChannels;
        if (nChannels == 1) {
            for (unsigned int i = 0; i < frames.frames(); i++, samples += hop)
                *samples++ = tick();
        } else {
            for (unsigned int i = 0; i < frames.frames(); i++, samples += hop) {
                *samples++ = tick();
                for (j = 1; j < nChannels; j++)
                    *samples++ = lastFrame_[j];
            }
        }

        return frames;
    }
}
#endif
