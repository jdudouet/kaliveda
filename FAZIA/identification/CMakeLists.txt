BUILD_KALIVEDA_MODULE(identification
	PARENT ${KVSUBPROJECT}
#        KVMOD_DEPENDS geometry signals
        KVMOD_DEPENDS signals
        EXTRA_LIBS KVMultiDetexp_events KVMultiDetidentification
)
