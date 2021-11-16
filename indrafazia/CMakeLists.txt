#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})

#---set paths to find all INDRA headers
KALIVEDA_SET_INCLUDE_DIRS(KVIndra MODULES ${KVIndra_MOD_LIST})

#---set paths to find all FAZIA headers
KALIVEDA_SET_INCLUDE_DIRS(FAZIA MODULES ${FAZIA_MOD_LIST})

BUILD_KALIVEDA_SUBPROJECT()
