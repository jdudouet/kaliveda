BUILD_KALIVEDA_MODULE(identification
	PARENT ${KVSUBPROJECT}
        KVMOD_DEPENDS geometry signals
	EXTRA_LIBS KVMultiDetexp_events KVMultiDetidentification
)
