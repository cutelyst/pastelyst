file(GLOB_RECURSE Pastelyst_SRCS *.cpp *.h)

set(Pastelyst_SRCS
    ${Pastelyst_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(Pastelyst SHARED ${Pastelyst_SRCS})

# Link to Cutelyst
target_link_libraries(Pastelyst
    KF5::SyntaxHighlighting
    Cutelyst::Core
    Cutelyst::View::Cutelee
    Cutelyst::Utils::Sql
    Cutelyst::Utils::Pagination
    Cutelyst::Authentication
    Cutelyst::Session
    Cutelee5::Templates
    Qt5::Core
    Qt5::Network
    Qt5::Sql
)
