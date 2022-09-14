set(TAKINA_DIR ${THIRD_PARTY_DIR}/takina)
set(XXHASH_DIR ${THIRD_PARTY_DIR}/xxHash)

include_directories(${XXHASH_DIR})

file(GLOB XXHASH_SRC ${XXHASH_DIR}/*.c)

file(GLOB_RECURSE LB_SRC
  algo/*.cc
  http/*.cc
  router/*.cc
  ${TAKINA_DIR}/*.cc  
  ${XXHASH_SRC}
)

set(LB_APP_SRC
  app/main.cc
  ${LB_SRC}
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(LB_LIB lb_config)
else ()
  set(LB_LIB lb)
endif ()

GenLib(${LB_LIB} ${LB_SRC})

set(KANON_LIB kanon_base kanon_net)
GenApplication(load-balancer LB_APP_SRC KANON_LIB)
