#ifndef ELTEK_CHECK_H
#define ELTEK_CHECK_H
#include "can.h"
#include "IPwrCheck.h"


class EltekCheck : public IPwrCheck
{
public:
	EltekCheck();

	virtual bool autoCheckModule(void);
private:
	bool processAutoIdentify(CanRxMsg& msg);
	void sendLoginRequest();
};
#endif
