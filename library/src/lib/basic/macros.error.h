#pragma once

#define default_log(err, msg) debug_print(KBURN_LOG_ERROR, msg " - %d", (int)err)

#define _IfErrorReturn3(checker, action, before_return)                                                                                              \
	__extension__({                                                                                                                                  \
		kburn_err_t _err = (kburn_err_t)action;                                                                                                      \
		if (!checker(_err)) {                                                                                                                        \
			before_return(_err, #action);                                                                                                            \
			return _err;                                                                                                                             \
		}                                                                                                                                            \
		_err;                                                                                                                                        \
	})

#define IfErrorReturn(...)                                                                                                                           \
	__VaridicMacro_Helper3(, ##__VA_ARGS__, _IfErrorReturn3(__VA_ARGS__), _IfErrorReturn3(__VA_ARGS__, default_log),                                 \
						   _IfErrorReturn3(kburn_not_error, __VA_ARGS__, default_log), )

#define kburn_not_error(e) (e == KBurnNoErr)
