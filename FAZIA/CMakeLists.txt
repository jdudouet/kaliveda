if(WITH_PROTOBUF)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/protobuf)
else(WITH_PROTOBUF)
    set(ignored ${ignored} protobuf daq_cec)
endif(WITH_PROTOBUF)

#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})

BUILD_KALIVEDA_SUBPROJECT()
