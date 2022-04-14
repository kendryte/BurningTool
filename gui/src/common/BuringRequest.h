#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QString>

class BuringRequest {
  public:
	virtual ~BuringRequest(){};

	QString systemImageFile;

	enum DeviceKind
	{ K510, };

	static class BurningProcess *reqeustFactory(KBCTX scope, const BuringRequest *request);

  protected:
	virtual enum DeviceKind getKind() const = 0;
};

class K510BuringRequest : public BuringRequest {
  protected:
	enum DeviceKind getKind() const { return K510; }

  public:
	QString comPort;
};
