/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCMutableBuffer.h
	Description: 	
			This file provide a data buffer based on memery pool.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2016-06-13: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_MUTABLE_BUFFER_H
#define _AMC_MUTABLE_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>

#ifndef BOOL
#define BOOL	long
#define FALSE	(0)
#define TRUE	(!0)
#endif

#ifndef NULL
#define NULL	((void*)0)
#endif

#define AMC_BUFFER_OFFSET_BEGINNING		(0)
#define AMC_BUFFER_LENGTH_TO_END		(0x7FFFFFFF)


typedef enum {
	AMC_BUFFER_SUCCESS = 0,
	AMC_BUFFER_SYSTEM_API_ERROR,
	AMC_BUFFER_MUTEX_ERROR,
	AMC_BUFFER_MEMPOOL_ERROR,
	AMC_BUFFER_PARA_ERROR,
	AMC_BUFFER_OFFSET_EXCEED_LEN,

	/* SHOULD place in the end */
	AMCMutableBufferError_Types
} AMCMutableBufferError_t;


struct AMCMutableBuffer;

/*********************************************************************
 * Function:    AMCMutableBuffer_New()
 * Author:      Andrew Chang
 * Input:       size per mempool unit (do not need address alignment)
 *              initialize mempool size (total in bytes)
 *              pointer to return error number
 *              should use lock? specity TRUE to use lock
 * Return:      A new mutable data buffer object.
 */
struct AMCMutableBuffer *AMCMutableBuffer_New (size_t unitSize, size_t initSize, AMCMutableBufferError_t *pErrOut, BOOL threadSafe);


/*********************************************************************
 * Function:    AMCMutableBuffer_Destroy()
 * Author:      Andrew Chang
 * Input:       buffer object to destroy
 * Return:      error number
 */
AMCMutableBufferError_t AMCMutableBuffer_Destroy (struct AMCMutableBuffer *buffer);


/*********************************************************************
 * Function:    AMCMutableBuffer_StrError()
 * Author:      Andrew Chang
 * Input:       error number
 * Return:      error string
 * Description:
 *      Return a string to identify error.
 */
const char *AMCMutableBuffer_StrError (AMCMutableBufferError_t error);


/*********************************************************************
 * Function:    AMCMutableBuffer_Dump()
 * Author:      Andrew Chang
 * Input:       buffer object
 *              file descripter
 * Return:      error number
 * Description:
 *      Dump all data in buffer into specified file descriptor. If fd
 *  is 1(stdout) or 2(stderr), you will see readable hex data. Other-
 *  wise, raw bytes will be directly output to fd.
 */
AMCMutableBufferError_t AMCMutableBuffer_Dump (struct AMCMutableBuffer *buffer, int fd);


/*********************************************************************
 * Function:    AMCMutableBuffer_FramentSize()
 * Author:      Andrew Chang
 * Input:       buffer object
 * Return:      fragment size
 * Description:
 *      Return fragment size in mutable buffer. After several insert or
 *  delete operation, there would be a lot of unfragmented memory pieces
 *  in buffer object. This function is to determine how many.
 */
size_t AMCMutableBuffer_FramentSize (struct AMCMutableBuffer *buffer);


/*********************************************************************
 * Function:    AMCMutableBuffer_Defragment()
 * Author:      Andrew Chang
 * Input:       buffer object
 * Return:      error number
 * Description:
 *      Defragment the data buffer object. This function will compact
 *  all data framents together and free unused mempool units.
 */
AMCMutableBufferError_t AMCMutableBuffer_Defragment (struct AMCMutableBuffer *buffer);


/*********************************************************************
 * Function:    AMCMutableBuffer_CheckAllData()
 * Author:      Andrew Chang
 * Input:       buffer object
 *              callback for buffer object to invoke
 *              data buffer for function to store in
 *              size of given data buffer
 *              argument passed in callback
 * Return:      error number
 *
 * Callback:    each_segment_cb
 * Input:       data buffer (the same as byteBuff)
 *              data offset in buffer object
 *              data length for current data piece
 *              given argument
 *              a bool calue telling whether this is the last pirce of whole buffer
 * Return:      N/A
 *
 * Description:
 *      This function goes through the whole buffer object, copies all 
 *  data into given data storage, and then invoke this callback.
 *      When data reaches the last buffer piece, the "isLastSegment"
 *  parameter will be set as TRUE. Programmers could take special
 *  operations by this value if necessary.
 */
AMCMutableBufferError_t AMCMutableBuffer_CheckAllData (struct AMCMutableBuffer *buffer, 
											void (*each_segment_cb)(void *data, size_t offset, size_t length, void *arg, BOOL isLastSegment),
											void *byteBuff, size_t byteBuffLen, void *arg);


/*********************************************************************
 * Function:    AMCMutableBuffer_GetLen()
 * Author:      Andrew Chang
 * Input:       buffer object
 * Return:      data size in buffer object
 * Description:
 *      This function returns the pure data length storing  in the 
 *  buffer object.
 */
size_t AMCMutableBuffer_GetLen(struct AMCMutableBuffer *buffer);


/*********************************************************************
 * Function:    AMCMutableBuffer_Append()
 * Author:      Andrew Chang
 * Input:       buffer object
 * Return:      data size in buffer object
 * Description:
 *      This function returns the pure data length storing  in the 
 *  buffer object.
 */
AMCMutableBufferError_t AMCMutableBuffer_Append (struct AMCMutableBuffer *buffer, const void *data, size_t dataLen);


/*********************************************************************
 * Function:    AMCMutableBuffer_Delete()
 * Author:      Andrew Chang
 * Input:       buffer object
 *              data offset to start delete from
 *              data length to delete with
 *              a parameter to tell deleted data length
 * Return:      error number
 */
AMCMutableBufferError_t AMCMutableBuffer_Delete (struct AMCMutableBuffer *buffer, size_t offset, size_t len, size_t *pDeleteLenOut);


/*********************************************************************
 * Function:    AMCMutableBuffer_DeleteAll()
 * Author:      Andrew Chang
 * Input:       buffer object
 * Return:      error number
 * Description:
 *      This function clean all data in the buffer object. If you really
 *  to do this, I suggest you use this function. This function operates
 *  much more quickly than the "Delete" one.
 */
AMCMutableBufferError_t AMCMutableBuffer_DeleteAll (struct AMCMutableBuffer *buffer);


/*********************************************************************
 * Function:    AMCMutableBuffer_DeleteAll()
 * Author:      Andrew Chang
 * Input:       buffer object
 *              byte buffer to storage read data
 *              data offset to start read from
 *              data length to read with. specity AMC_BUFFER_LENGTH_TO_END to read all if possible
 *              a data pointer to return actual read data length
 *              parameter to specify whether to delete data after reading
 * Return:      error number
 * Description:
 *      This function read data from "offset" with "len" in buffer object
 *  to given "dataOut". If "shouldRemove" is TRUE, corresponding data will
 *  be removed from buffer object after reading.
 */
AMCMutableBufferError_t AMCMutableBuffer_Read (struct AMCMutableBuffer *buffer, void *dataOut, size_t offset, size_t len, size_t *pLenOut, BOOL shouldRemove);


/*********************************************************************
 * Function:    AMCMutableBuffer_Insert()
 * Author:      Andrew Chang
 * Input:       buffer object
 *              data offset to start insert from
 *              binary data to insert
 *              data length to insert
 * Return:      error number
 * Description:
 *      This function will insert given data in specified position. This
 *  operation may add 3 unfragmented data pieces at most.
 */
AMCMutableBufferError_t AMCMutableBuffer_Insert (struct AMCMutableBuffer *buffer, size_t offset, const void *data, size_t dataLen);


size_t AMCMutableBuffer_GetUnitSize (struct AMCMutableBuffer *buffer);
size_t AMCMutableBuffer_GetUnitCount (struct AMCMutableBuffer *buffer);
size_t AMCMutableBuffer_GetMemoryUsage (struct AMCMutableBuffer *buffer);


#endif	/* EOF */
