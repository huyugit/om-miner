#ifndef MS_ERROR_H
#define MS_ERROR_H

/***********************************************************************************
 *
 * error format
 * 31 ...... 24 | 23 ........ 0
 *  error type  |   error num
 ***********************************************************************************/
#define MS_ERROR_TYPE_SHIFT		24
#define MS_ERROR_TYPE_MASK		0xff000000
#define MS_ERROR_NUM_MASK		0x00ffffff

#define MS_SET_ERROR(type, num)			(((type) << MS_ERROR_TYPE_SHIFT) | (num))

#define MS_SET_HW_ERR_CODE(class, type, num)	(((class) << 16) | ((type) << 8) | (num))
#define MS_SET_SW_ERR_CODE(class, type, num)	((1 << 24) | ((class) << 16) | ((type) << 8) | (num))

/*hash board error type*/
#define ERR_TYPE_TMP_HIGH 1
#define ERR_TYPE_TMP_LOW 2

/*fan error type*/
#define ERR_TYPE_I_HIGH 1
#define ERR_TYPE_U_HIGH 2
#define ERR_TYPE_SPEED_LOW 3
#define ERR_TYPE_FAN_FAULT 4

/*power error type*/
#define ERR_TYPE_RET_LESS 1
#define ERR_TYPE_PSU_I_HIGH 2
#define ERR_TYPE_PSU_I_LOW 3
#define ERR_TYPE_PSU_U_HIGH 4
#define ERR_TYPE_PSU_P_LOW 5
#define ERR_TYPE_PSU_P_HIGH 6
#define ERR_TYPE_PSU_SPD_LOW 7
#define ERR_TYPE_PSU_TMP_HIGH 8

/*software error type*/
#define ERR_TYPE_PTR_NULL 1





/*********************************************************
 * error type 
 ********************************************************/
#define MS_ERROR_TYPE_HARDWARE	0x01
#define MS_ERROR_TYPE_SOFTWARE	0x02

#define MS_GET_ERROR_TYPE(error) 		(((error) & MS_ERROR_TYPE_MASK) >> MS_ERROR_TYPE_SHIFT)
#define MS_GET_ERROR_NUM(error)			((error) & MS_ERROR_NUM_MASK)

/****************************************************************************************
 * define errors
 ***************************************************************************************/
#define MS_ERROR_NO_ERROR						MS_SET_ERROR(0x00, 0x00000000)
#define MS_ERROR_BOARD_FAN_I_TOO_HIGH			MS_SET_ERROR(MS_ERROR_TYPE_HARDWARE, 0x00000001)
#define MS_ERROR_BOARD_PSU						MS_SET_ERROR(MS_ERROR_TYPE_HARDWARE, 0x00000002)
#define MS_ERROR_BOARD_TEMP_TOO_HIGH			MS_SET_ERROR(MS_ERROR_TYPE_HARDWARE, 0x00000003)
#define MS_ERROR_BOARD_FAN_U_TOO_HIGH			MS_SET_ERROR(MS_ERROR_TYPE_HARDWARE, 0x00000004)
#define MS_ERROR_BOARD_FAN_SPEED 				MS_SET_ERROR(MS_ERROR_TYPE_HARDWARE, 0x00000005)


#define MS_ERROR_NO_PSU_MODULE					MS_SET_ERROR(MS_ERROR_TYPE_SOFTWARE, 0x00000001)


#endif /* end of MS_ERROR_H */
