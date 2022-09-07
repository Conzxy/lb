set(TAKINA_DIR ${THIRD_PARTY_DIR}/takina)

file(GLOB_RECURSE LB_SRC
  algo/*.cc
  http/*.cc
  router/*.cc
  ${TAKINA_DIR}/*.cc  
)

set(LB_APP_SRC
  app/main.cc
  ${LB_SRC}
)

GenLib(lb ${LB_SRC})

set(KANON_LIB kanon_base kanon_net)
GenApplication(load-balancer LB_APP_SRC KANON_LIB)
