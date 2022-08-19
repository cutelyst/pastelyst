file(GLOB_RECURSE Pastelyst_SRCS *.cpp *.h)

set(Pastelyst_SRCS
    ${Pastelyst_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(Pastelyst SHARED ${Pastelyst_SRCS})

# Link to Cutelyst
target_link_libraries(Pastelyst
#    KF5::SyntaxHighlighting
    KF5SyntaxHighlighting
    Cutelyst::Core
    Cutelyst::View::Cutelee
    Cutelyst::Utils::Sql
    Cutelyst::Utils::Pagination
    Cutelyst::Authentication
    Cutelyst::Session
    Cutelee::Templates
    Qt::Core
    Qt::Network
    Qt::Sql
)

install(TARGETS Pastelyst
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
)

install(DIRECTORY ../root
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/pastelyst
)
