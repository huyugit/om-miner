/*************************************************************************
        > File Name: pca9536.h
        > Author: gzh
        > Mail: zhihua.ge@bitfily.com 
        > Created Time: 2017年12月12日 星期二 09时33分42秒
 ************************************************************************/
#ifndef __PCA9536_H__
#define __PCA9536_H__
#define PCA9536_PIN_NUM		0x4
#define PCA9536_PIN1_MASK	0x1
#define PCA9536_PIN2_MASK	0x2
#define PCA9536_PIN3_MASK	0x4
#define PCA9536_PIN4_MASK	0x8

#define ERR_PCA9536_REG1_GET	0x1
#define ERR_PCA9536_REG1_SET	0x2

int pca9536_pin_status_get(int *pPinStatus);
int pca9536_pin_status_set(int pinstatus);

#endif

