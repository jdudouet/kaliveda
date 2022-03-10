#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})

if(WITH_PROTOBUF)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/protobuf)
    BUILD_KALIVEDA_SUBPROJECT()
else(WITH_PROTOBUF)
    BUILD_KALIVEDA_SUBPROJECT(IGNORE_DIRS protobuf daq_cec)
endif(WITH_PROTOBUF)
