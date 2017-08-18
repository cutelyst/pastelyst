file(GLOB_RECURSE Sticklyst_SRCS *.cpp *.h)

set(Sticklyst_SRCS
    ${Sticklyst_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(Sticklyst SHARED ${Sticklyst_SRCS})

# Link to Cutelyst
target_link_libraries(Sticklyst
    KF5::SyntaxHighlighting
    Cutelyst::Core
    Cutelyst::View::Grantlee
    Cutelyst::Utils::Sql
    Qt5::Core
    Qt5::Network
    Qt5::Gui
)

