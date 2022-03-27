#---list of datasets
set(ignore_datasets INDRA_e416a INDRA_e475s INDRA_e494s_woVAMOS INDRA_e503_woVAMOS t10_02)
set(ignore_dirs INDRA2ROOT)
set(ignore_geo_dirs geometry_E416 geometry_E475S geometry_E494S) 
set(ignore_id_dirs identification_E416 identification_E475S identification_E494S identification_camp5 identification_E613)

#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})

BUILD_KALIVEDA_SUBPROJECT(IGNORE_DIRS ${ignore_dirs} ${ignore_geo_dirs} ${ignore_id_dirs} ${ignore_datasets})

if(USE_INDRA2ROOT)
#---conversion of old INDRA data
   add_subdirectory(INDRA2ROOT)
endif(USE_INDRA2ROOT)
