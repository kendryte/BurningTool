#include "private-types.h"
#include "canaan-burn/canaan-burn.h"

struct usb_settings usb_default_settings = {
	.vid = KBURN_USB_DEFAULT_VID,
	.pid = KBURN_USB_DEFAULT_PID,
};

CREATE_GETTER_SETTER(FilterVid, vid)
CREATE_GETTER_SETTER(FilterPid, pid)
