# Pastelyst

Pastelyst is an easy to use and expandable web paste tool built with [Cutelyst](https://github.com/cutelyst/cutelyst) and [KDE Frameworks 5 Syntax Highlighting](https://github.com/KDE/syntax-highlighting). It uses less than 3 MB of RAM and renders quickly using a SQLite DB for the storage.

Pastelyst is almost feature complete and on production at [paste.cutelyst.org](https://paste.cutelyst.org/). You can grab the code and deploy your own paste tool now. And please don't forget to contribute to adding these extra features if you have time:

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

The chdir needs to point to the parent of the root directory that came from this project. The option `--static-map` is used to serve the static files.

Now point your browser to [http://localhost:3000](http://localhost:3000)

## API Usage Guidelines
Pastelyst also provides an API. Using the API, you can create your own applications based on Pastelyst or use Pastelyst from terminal. For details see [Creating RESTful applications with Qt and Cutelyst](https://dantti.wordpress.com/2018/05/17/creating-restful-applications-with-qt-and-cutelyst/), [Understanding And Using REST APIs](https://www.smashingmagazine.com/2018/01/understanding-using-rest-api/), and [POST Form Data with cURL](https://davidwalsh.name/curl-post-file).

### List pastes

```bash
curl -H "Content-Type: application/json" -X POST  https://paste.cutelyst.org/api/json/list/1
```

```json
{"result":{"count":15,"pages":0,"pastes":["hW7BwGCNS","7B6G5HJNT","l_Zo9VaRT","f_9z4b-_T","YnsDOFIRR","pu0h5ftsT"
```

```bash
curl -H "Content-Type: application/json" -X POST  https://paste.cutelyst.org/api/json/list/2
```

```json
{"result":{"count":15,"pages":0,"pastes":[]}}
```

### Show pastes

```bash
curl -H "Content-Type: application/json" -X POST  https://paste.cutelyst.org/api/json/show/pu0h5ftsT
```

```json
{"result":{"data":"#include <iostream>\r\n\r\nint main() {\r\n  std::cout << \"hello\\n\";\r\n  return 0;\r\n}","id":"pu0h5ftsT","language":"C++","timestamp":1503821618,"title":"Test"}}
```

```bash
curl -H "Content-Type: application/json" -X POST  https://paste.cutelyst.org/api/json/show/STIdv05VREeht/1234567890
```

```json
{"result":{"data":"my hidden text is here.","id":"STIdv05VREeht","language":"text","timestamp":1547050001,"title":"API test 1"}}
```

### Create pastes

```bash
curl -X POST http://localhost:3000/api/json/create -F 'data=echo "Hello World!"' -F 'language=Bash' -F 'title=API Test'
```

```json
{"result":{"id":"m2TvYu2sQ"}}
```

```bash
curl -X POST http://localhost:3000/api/json/create -F 'data=echo "Hello World!"' -F 'language=Bash' -F 'title=API Test' -F 'password=1234567890'
```

```json
{"result":{"id":"tMQD52pdRCe3K"}}
```

## History
Pastelyst was initially developed by Daniel Nicoletti and announced on August 25, 2017 on [cutelyst.org](https://cutelyst.org/2017/08/25/announcing-sticklyst-leveraging-kde-frameworks-on-the-web). Sticklyst, which was chosen as the project name, was later replaced by Pastelyst.

It was inspired by [a Perl Catalyst application](http://paste.scsys.co.uk/) to benefit KDE Frameworks 5 Syntax Highlighting. Thanks to some helps from KDE community and Volker Krause it used without GUI widgets.

- on 7 Sep 2018 Pastelyst 0.3.0 released
- on 20 Mar 2018 Pastelyst 0.2.0 released
- on 25 Aug 2017 Pastelyst 0.1.0 released
