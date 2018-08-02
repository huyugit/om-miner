#ifndef SAG6400_CHECK_H
#define SAG6400_CHECK_H
#include "can.h"
#include "IPwrCheck.h"


class sag6400Check : public IPwrCheck
{
public:
	sag6400Check();

	virtual bool autoCheckModule(void);
private:
	bool processAutoIdentify(CanRxMsg& msg);
	void sendLoginRequest();
};
#endif
