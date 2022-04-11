#pragma once

#define _CREATE_GETTER_SETTER(Subsystem, Field, config_field)                                     \
	extern KBCTX scope;                                                                           \
	void kburnSet##Subsystem##Field(KBCTX scope, typeof(subsystem_settings.config_field) Field) { \
		debug_trace_function("%" PRId64, (int64_t)Field);                                         \
		subsystem_settings.config_field = Field;                                                  \
	}                                                                                             \
	typeof(subsystem_settings.config_field) kburnGet##Subsystem##Field(KBCTX scope) {             \
		return subsystem_settings.config_field;                                                   \
	}
