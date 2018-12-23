# Pastelyst

Pastelyst is an easy to use and expandable web paste tool built with [Cutelyst](https://github.com/cutelyst/cutelyst) and [KDE Frameworks 5 Syntax Highlighting](https://github.com/KDE/syntax-highlighting).

Pastelyst is almost completed and on production at [paste.cutelyst.org](https://paste.cutelyst.org/). You can grab the code and deploy your own paste tool. And please don't forget to contribute to adding these missing features if you have time:

 * Extra database support
 * URL shortener
 * User authentication
 * [And some more](https://github.com/cutelyst/pastelyst/issues/8)

The list is short and the code is clean, go test, break, hack, and have fun!

## Dependencies
 * Grantlee
 * KDE Frameworks 5 Syntax Highlighting
 * Cutelyst 2.0.0 or newer with enabled Grantlee plugin

## Configuration
Create an INI file like pastelyst.conf with:

    [Cutelyst]
    DatabasePath = /var/tmp/my_site_data/pastelyst.sqlite
    production = true

    social = true
    download = true
    clipboard = true

Where:

 * DatabasePath is the place where sqlite database will be placed
 * production when true will preload the theme templates, which is a lot faster but if you are customizing the theme you will need to reload the process
 * social adds social media share buttons for pastes if enabled
 * download adds download button for pastes if enabled
 * clipboard adds copy to clipboard button for pastes if enabled

## Running
You can run it with cutelyst-wsgi or uWSGI, both have similar command line options, and you should look at their documentation to know their options, the simplest one:

    cutelyst-wsgi2 --application path/to/libPastelyst.so --http-socket :3000 --ini pastelyst.conf --chdir parent_of_root_dir --static-map /static=root/static

The chdir needs to point to the parent of the root directory that came from this project. The option --static-map is used to serve the static files.

Now point your browser to http://localhost:3000
