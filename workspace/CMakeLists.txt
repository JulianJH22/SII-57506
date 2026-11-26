CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT (TENIS)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

SET(COMMON_SRCS
    src/MundoServidor.cpp
    src/MundoCliente.cpp
    src/Esfera.cpp
    src/Plano.cpp
    src/Raqueta.cpp
    src/Vector2D.cpp
)

ADD_EXECUTABLE(bot src/bot.cpp)
ADD_EXECUTABLE(logger src/logger.cpp)
ADD_EXECUTABLE(cliente src/Cliente.cpp ${COMMON_SRCS})
ADD_EXECUTABLE(servidor src/Servidor.cpp ${COMMON_SRCS})

TARGET_LINK_LIBRARIES(cliente glut GL GLU)
TARGET_LINK_LIBRARIES(servidor glut GL GLU pthread)
