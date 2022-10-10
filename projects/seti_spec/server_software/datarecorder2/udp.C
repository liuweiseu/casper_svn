#include "dr2.h"
#include "globals.h"

//======================================================
void InitEdt(int dsi) {
//======================================================

//    if(Verbose) {
//        sprintf(MemBuf[dsi].Message, "Configuring EDT device %d... ", dsi);
//        //WriteLog(MemBuf[i].Message, ToUser);
//    }
//
    MemBuf[dsi].EdtDevice = edt_open(dsi);
//    if(!MemBuf[dsi].EdtDevice) {
//        EdtErrExit("Could open EDT device ");
//    }
//
//    if(edt_configure_ring_buffers(MemBuf[dsi].EdtDevice,
//                                  MemBuf[dsi].Size,
//                                  MemBuf[dsi].Num,
//                                  EDT_READ,
//                                  NULL) == -1) {
//        EdtErrExit("Could not configure EDT ring buffers ");
//    } else {
//        if(Verbose) {
//            strcat(MemBuf[dsi].Message, "\t\tOK\n");
//            WriteLog(MemBuf[dsi].Message, ToUser);
//        }
//    }
}


//=======================================================
//void  EdtCntl( EdtDev * edt_p, int action) {
//=======================================================
//
//    unsigned old_reg_val, new_reg_val;
//    int retval;
//
//    if (action == edt_arm) {
//        old_reg_val = edt_reg_read(edt_p, PCD_FUNCT);
//        new_reg_val = edt_reg_or(edt_p, PCD_FUNCT, 0x08);
//        fprintf(stderr, "arming : old val = %08x  new val = %08x\n", old_reg_val, new_reg_val);
//    } else if (action == edt_disarm) {
//        old_reg_val = edt_reg_read(edt_p, PCD_FUNCT);
//        new_reg_val = edt_reg_and(edt_p, PCD_FUNCT, 0xf7);
//        fprintf(stderr, "disarming : old val = %08x  new val = %08x\n", old_reg_val, new_reg_val);
//    } else if (action == edt_abort_current_buf) {
//        retval = edt_abort_dma(edt_p);
//        if(!retval) {
//            fprintf(stderr, "aborting current ring buffer segment\n");
//        } else {
//            edt_perror("Could not abort DMA");
//            ErrExit("Could not abort DMA");
//        }
//    }
//}
//
//
//=======================================================
//bool  EdtIsOpen(int dsi) {
//=======================================================
//
//    if(MemBuf[dsi].EdtDevice == NULL) {
//        return(false);
//    } else {
//        return(true);
//    }
//}
//
//=======================================================
//void  CloseEdt(int dsi) {
//=======================================================
//
//#if 0
//    if(Verbose) {
//        sprintf(MemBuf[dsi].Message, "MEM[%d] : Closing EDT device\n", dsi);
//        WriteLog(MemBuf[dsi].Message, ToUser);
//    }o
//#endif
//
//    edt_stop_buffers(MemBuf[dsi].EdtDevice);
//    EdtCntl(MemBuf[dsi].EdtDevice, edt_disarm);
//    edt_close(MemBuf[dsi].EdtDevice);
//    MemBuf[dsi].EdtDevice = NULL;
//}
//
//=======================================================
//void EdtErrExit(char * Msg) {
//=======================================================
//    edt_perror(Msg);
//    exit(1);
//}

