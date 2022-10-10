#include "dr2.h"
#include "globals.h"

//=======================================================
void MySignals(int action) {
//=======================================================
// handle SIGINT and SIGTERM signals. First we block
// these signals.  All created threads will inherit
// a mask with these blocks in place.  After thread
// creation, unblock these signals.  The main() thread
// will thus be selected to recieve them.

    if(action == INIT) {
        sigemptyset(&SigBlockMask);
        sigaddset(&SigBlockMask, SIGINT);
        sigaddset(&SigBlockMask, SIGTERM);
        sigaddset(&SigBlockMask, SIGALRM);
        if(pthread_sigmask(SIG_BLOCK, &SigBlockMask, &SigOriginalMask)) ErrExit("MySignals : pthread sigmask error \n");
        SigAction.sa_handler  = SigHandler;
        sigaction(SIGINT, &SigAction, NULL);
        sigaction(SIGTERM, &SigAction, NULL);
        sigaction(SIGALRM, &SigAction, NULL);
        return;
    }

    if(action == UNBLOCK) {
        if(pthread_sigmask(SIG_SETMASK, &SigOriginalMask, NULL)) ErrExit("MySignals : pthread sigmask error \n");
        return;
    }
}

//=======================================================
void SigHandler(int sig) {
//=======================================================
// Important: SIGALRM is used by the synthesizer control
// thread and should be used nowhere else in the datarecorder.

    psignal(sig, "Recieved signal");
    if (sig == SIGALRM) fprintf(stdout, "Synth not responding\n");
    SayToExit();
}

