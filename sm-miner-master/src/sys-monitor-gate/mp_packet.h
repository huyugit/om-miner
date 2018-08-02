/*************************************************************************
        > File Name: mp_packet.h
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月20日 星期二 09时33分42秒
 ************************************************************************/
#ifndef __MP_PACKET_H__
#define __MP_PACKET_H__
enum a1_mon_item {
	hash_bd1_temp,		//hash板1温度
	hash_bd2_temp,		//hash板2温度
	hash_bd3_temp,		//hash板3温度
	hash_bd4_temp,		//hash板4温度
	hash_bd5_temp,		//hash板5温度
	hash_bd6_temp,		//hash板6温度
	fan_I,				//风扇电流
	fan_V,				//风扇电压
	fan1_fault,			//风扇1错误
	fan2_fault,			//风扇2错误
	fan3_fault,			//风扇3错误
	fan4_fault,			//风扇4错误
	fan5_fault,			//风扇5错误
	fan6_fault,			//风扇6错误
	power1_I,			//电源1电流
	power2_I,			//电源2电流
	power1_V,			//电源1电压
	power2_V,			//电源2电压
	power1_P,			//电源1功率
	power2_P,			//电源2功率

	MONITOR_ITEM_MAX
};

#define ERR_LEVEL_0		0	//正常
#define	ERR_LEVEL_1		1	//级别1，轻微故障，红色LED灯慢闪
#define ERR_LEVEL_2		2	//级别2，紧急故障，红色LED灯快闪
#define ERR_LEVEL_3		3	//级别3，严重故障，红色LED灯常亮
/*
	检测项结果数据格式
*/
typedef struct {
	char item_id;		//检测项ID
	char error_level;	//错误级别
}MST_MON_ITEM;


#endif

