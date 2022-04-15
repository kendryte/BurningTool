#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QString>

class BurningRequest {
  public:
	virtual ~BurningRequest(){};

	QString systemImageFile;

	enum DeviceKind
	{ K510, };

	static class BurningProcess *reqeustFactory(KBCTX scope, const BurningRequest *request);

  protected:
	virtual enum DeviceKind getKind() const = 0;
};

class K510BurningRequest : public BurningRequest {
  protected:
	enum DeviceKind getKind() const { return K510; }

  public:
	QString comPort;
};
