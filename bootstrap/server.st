*
* Little Smalltalk, version 3
* Written by Tim Budd, Oregon State University, July 1988
*
*  methods for the unix front end - single process version
*
* initialization code
* this is executed once, by the initial image maker
*
*
Class Server Object socket handlers
Class Handler Object
Class TestHandler Handler
Class ClassHandler Handler
*
Methods Smalltalk 'doit'
    error: aString
        " print a message, and remove current process "
        stderr print: aString.
        scheduler currentProcess; trace; terminate.
]
*    
Methods Handler 'all'
    get: request from: socket
    socket send: (  '<html>',
                    '<body>',
                    '<h1>Hello from Tumbleweed</h1>',
                    '<button onclick="GetAsyncData();">Test</button>',
                    '<script language=javascript type="text/javascript">',
                    'var req_fifo;', Char cr, Char lf,
                    'function GetAsyncData() {', Char cr, Char lf,
                    '    url = "/test";', Char cr, Char lf,
                    '    req_fifo = new XMLHttpRequest();', Char cr, Char lf,
                    '    req_fifo.abort();', Char cr, Char lf,
                    '    req_fifo.onreadystatechange = GotAsyncData;', Char cr, Char lf,
                    '    req_fifo.open("POST", url, true);', Char cr, Char lf,
                    '    req_fifo.send(null);', Char cr, Char lf,
                    '  }', Char cr, Char lf,
                    '  function GotAsyncData() {', Char cr, Char lf,
                    '    if (req_fifo.readyState != 4 || req_fifo.status != 200) {', Char cr, Char lf,
                    '      return;', Char cr, Char lf,
                    '    }', Char cr, Char lf,
                    '    document.getElementById("data").innerHTML=', Char cr, Char lf,
                    '      req_fifo.responseText;', Char cr, Char lf,
                    '    return;', Char cr, Char lf,
                    '  }', Char cr, Char lf,
                    '</script>',
                    '<div id="data"></div>',
                    '</body></html>')
]
Methods TestHandler 'all'
    post: request from: socket
    | response |
    response <- '<ul>'.
    (classes select: [ :x | x isMeta not ]) do: [ :x |
        response <- (response, '<li>', (x printString), '</li>')
    ].
    response <- (response, '</ul>').
    socket send: response
]
Methods ClassHandler 'all'
    get: request from: socket
    socket send: '<html><body><h1>Class Handler</h1></body></html>'
]
*
Methods Server 'creation'
    new
    | result |
    socket <- Socket  new.
    handlers <- Dictionary new.
    result <- socket bindTo: 8090.
    (result = 0)
        ifFalse: [ 'Failed to bind socket' print. socket shutdown. ^ false ]
|
    run
    | client doit result clientsock |
    doit <- true.
    [ doit ] whileTrue: [
        socket listen.
        result <- 11.
        [ result = 11 ] whileTrue: [
            client <- socket accept.
            result <- client at: 3
        ].
        ('Accepted connection: ', (client printString)) print.
        clientsock <- Socket new: (client at: 1).
        ('Client: ', (clientsock printString)) print.
        [ self handle: clientsock ] fork
    ]
|
    addHandler: aHandler for: aString
    handlers at: aString put: aHandler
|
    handle: socket
    | data request result read httprequest urireg handler methodname method args |
    Module require: 'trex'.
    data <- ByteArray new: 255.
    request <- String new: 0.
    result <- Array new: 4.
    result at: 4 put: 11.
    ('Handling socket: ', (socket printString)) print.
    [ (result at: 4) = 11 ] whileTrue:
      [ result <- socket read: data.
        ('Read: ', (result printString)) print.
        read <- result at: 1.
        ((read < 255) and: [read > 0]) ifTrue: [ 
            result at: 4 put: 11.
            data at: (read + 1) put: 0 asCharacter
        ].
        request <- request combineWith: (data asString) 
      ].
    request grow: (0 asCharacter).
    httprequest <- HTTPRequest new: request; parseRequest.
    handlers binaryDo: [ :for :handler |
        ('Checking handler: ', for, ' against: ', (httprequest uri)) print.
        urireg <- TRex new: for.
        (urireg match: (httprequest uri)) ifTrue: [
            methodname <- ((httprequest method lowercase), ':from:') asSymbol.
            (handler respondsTo: methodname)
                ifTrue: [
                    args <- Array new: 3; at: 1 put: handler; at: 2 put: request; at: 3 put: socket.
                    method <- handler class methodNamed: methodname.
                    method executeWith: args
                ]
        ]
    ].
    socket shutdown
]

Methods Scheduler 'get commands'
    initialize  | string |
        Module require: 'sockets'.
        [ Server new; 
            addHandler: (Handler new) for: '/'; 
            addHandler: (TestHandler new) for: '/test'; 
            addHandler: (ClassHandler new) for: '/class/.*'; 
            run ] fork
]
Methods String 'all'
    print
        stdout print: self
|
    printNoReturn
        stdout printNoReturn: self
]
Methods UndefinedObject 'initial image'
    createGlobals   | aBlock |
        " create global variables in initial image "
        true <- True new.
        false <- False new.
        smalltalk <- Smalltalk new.
        files <- Array new: 15.
        stdin <- File new; name: 'stdin'; mode: 'r'; open.
        stdout <- File new; name: 'stdout'; mode: 'w'; open.
        stderr <- File new; name: 'stderr'; mode: 'w'; open.
        editor <- 'vi'.
        " create a dictionary of classes "
        classes <- Dictionary new.
        symbols binaryDo: [:x :y | 
            (y isKindOf: Class)
                ifTrue: [ classes at: x put: y ] ].
        scheduler <- Scheduler new.
        " create the initial system process "
        " note the delayed recursive call "
        aBlock <- [ files do: [:f | f notNil ifTrue: [ f open ]].
                   systemProcess <- aBlock newProcess.
                   scheduler run ].
        systemProcess <- aBlock newProcess.
|
    initialize
        " initialize the initial object image "
        self createGlobals.
        File new;
            name: 'serverSystemImage';
            open: 'w';
            saveImage;
            close.
]
