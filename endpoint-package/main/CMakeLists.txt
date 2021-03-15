
set(SRCS
    src/msg_mqtt.c
    src/msg_queue.c
    src/msg.c
)

set(INCS
    inc/msg_mqtt.h
    inc/msg_queue.h
)

set(COMPONENT_SRCS 
    ${INCS}
    ${SRCS}
)

set(COMPONENT_ADD_INCLUDEDIRS
    inc
)

register_component()

if(CONFIG_EXAMPLE_EMBEDDED_CERTS)
    target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif()
