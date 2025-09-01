#include "FPlayer.h"

namespace stk {
    FPlayer::FPlayer() {
        env_ = new ADSR;
        env_->setAttackTime(0.8);
        env_->setDecayTime(0.0);
        env_->setReleaseTime(0.2);

        std::string filePath = "../weaving_player.wav";
        file_ = new FileLoop(filePath);
        file_->setRate(1.0);
    }

    FPlayer::~FPlayer() {
        delete env_;
        delete file_;
    }

    void FPlayer::noteOn(StkFloat frequency, StkFloat amplitude) {
        this->keyOn();
    }


    void FPlayer::keyOn(void) {
        env_->keyOn();
    }

    void FPlayer::keyOff(void) {
        env_->keyOff();
    }

    void FPlayer::noteOff(StkFloat amplitude) {
        this->keyOff();
    }
}
