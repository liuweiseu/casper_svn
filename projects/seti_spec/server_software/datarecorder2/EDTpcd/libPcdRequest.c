/* #pragma ident "@(#)libPcdRequest.c	1.1 12/24/01 EDT" */

#include <jni.h>
#include "PcdRequest.h"
#include <stdio.h>

#include "edt_bitfile.h"

char instr[128];
static edt_board_desc_table edt_boards ;
static edt_bitfile_desc_table edt_bitfiles ;

/*
 * Class:     PcdRequest
 * Method:    PcdGetConfigInfo
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_PcdRequest_PcdGetConfigInfo
  (JNIEnv *env, jobject obj, jstring pcd_bitfiles)
{
    int ret ;
    char *bitfile_list ;
    char *board_list ;
    char *cfg_filename = (char *)(*env)->GetStringUTFChars(env, pcd_bitfiles, 0);
    jstring foo ;

    ret = edt_probe_boards(edt_boards) ;
    if (ret) return NULL ;

    board_list = get_board_list(edt_boards) ;

    ret = edt_read_bitfile_config(cfg_filename, edt_bitfiles) ;
    if (ret) return NULL ;

    bitfile_list = get_bitfile_list(edt_bitfiles) ;
    strcat(bitfile_list, board_list) ;
    
    return (*env)->NewStringUTF(env, bitfile_list) ;
}

/*
 * Class:     PcdRequest
 * Method:    PcdGetIntfcStrings
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_PcdRequest_PcdGetIntfcStrings
  (JNIEnv *env, jobject obj, jstring pciStr)
{
    static char *pciName = NULL ;
    static int callCount = 0 ;
    static edt_bitfile_desc *bitfileDesc ;
    jstring retStr = NULL ;
    char str[128] ;

    if (pciName == NULL)
    {
    	pciName = (char *)(*env)->GetStringUTFChars(env, pciStr, 0);
	bitfileDesc = edt_get_bitfile_desc(pciName, edt_bitfiles);
    }

    if (callCount < bitfileDesc->intfc_bitfile_count)
    {
	char tmpstr[32] ;
	sprintf(tmpstr, "%s:", bitfileDesc->intfc_bitfile_names[callCount]);
	/* sprintf(str, "%-20s  %s", tmpstr, */
	sprintf(str, "%-20s  %-55s", tmpstr,
			       bitfileDesc->intfc_bitfile_comments[callCount]);
        retStr = (*env)->NewStringUTF(env, str) ;
	
	++ callCount ;
    }
    else
    {
    	callCount = 0 ;
	pciName = NULL ;
        retStr = NULL ;
    }

    return retStr ;
}

static int currentBoard = -1 ;

/*
 * Class:     PcdRequest
 * Method:    PcdGetBoardInfoStrings
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_PcdRequest_PcdGetBoardInfoStrings
  (JNIEnv *env, jobject obj)
{
    static char *pciName = NULL ;
    static edt_bitfile_desc *bitfileDesc ;
    jstring retStr = NULL ;
    char str[128] ;

    ++ currentBoard ;

    while (currentBoard < EDT_MAX_BOARDS && *edt_boards[currentBoard].board_name
     && strcmp(edt_boards[currentBoard].board_name, "pcd") != 0)
    {
         ++ currentBoard ;
    }

    if (currentBoard < EDT_MAX_BOARDS)
    {
	if (*edt_boards[currentBoard].board_name)
	{
		sprintf(str, "PCD unit %d PCI Xilinx %s flash %s",
				edt_boards[currentBoard].unit_no,
				edt_boards[currentBoard].pci_xilinx_type,
				edt_boards[currentBoard].pci_flash_name);
		retStr = (*env)->NewStringUTF(env, str) ;
	}
	else
	{
		currentBoard = -1 ;
	}
    }
    return retStr ;
}


/*
 * Class:     PcdRequest
 * Method:    PcdSetInterfaceBitfile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_PcdRequest_PcdSetInterfaceBitfile
  (JNIEnv *env, jobject obj, jstring bitfileName)
{
    char *bitfile = (char *)(*env)->GetStringUTFChars(env, bitfileName, 0);
    edt_boards[currentBoard].intfc_bitfile = bitfile ;
}

/*
 * Class:     PcdRequest
 * Method:    PcdSaveConfigScript
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_PcdRequest_PcdSaveConfigScript
  (JNIEnv *env, jobject obj)
{
    return create_pcdload_script(edt_boards) ;
}
