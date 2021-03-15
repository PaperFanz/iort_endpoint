
set(SRCS
    src/msg_mqtt.c
    src/msg_queue.c
    src/msg.c
)

set(INCS
    inc/msg_mqtt.h
    inc/msg_queue.h
)

set(LIBS
    lib/adc_func.c
    lib/debug.c
    lib/epd.c
    lib/epdgl.h
    lib/font.c
    lib/qrcode.c

    lib/adc_func.h
    lib/debug.h
    lib/epdgl.c
    lib/epd.h
    lib/font.h
    lib/qrcode.h
)

set(COMPONENT_SRCS
    ${LIBS}
    ${INCS}
    ${SRCS}
)

set(COMPONENT_ADD_INCLUDEDIRS
    inc
    lib
)

register_component()

if(CONFIG_EXAMPLE_EMBEDDED_CERTS)
    target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif()
