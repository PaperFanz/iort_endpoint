set(COMPONENT_SRCS "msg_mqtt.c" "msg_mqtt.h" "msg_queue.c" "msg_queue.h" "msg.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")


register_component()

if(CONFIG_EXAMPLE_EMBEDDED_CERTS)
target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif()
