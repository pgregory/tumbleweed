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
    socket send: '<html><body><h1>Hello from Tumbleweed</h1></body></html>'
]
Methods TestHandler 'all'
    get: request from: socket
    | response |
    response <- '<html><body><h1>This is a Test Handler</h1><ul>'.
    ((classes select: [ :x | x isMeta not ]) sort: [ :x :y | (x printString) < (y printString) ] ) do: [ :x |
        response <- (response, '<li>', (x printString), '</li>')
    ].
    response <- (response, '</ul></body></html>').
    socket send: response
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
    | clientsock doit |
    doit <- true.
    [ doit ] whileTrue: [
        socket listen.
        clientsock <- socket accept.
        self handle: clientsock.
        clientsock shutdown
    ]
|
    addHandler: aHandler for: aString
    handlers at: aString put: aHandler
|
    handle: socket
    | data request result httprequest handler methodname method args |
    data <- ByteArray new: 255.
    request <- String new: 0.
    result <- 1.
    [ result > 0 ] whileTrue:
      [ result <- socket read: data.
        (result < 255) ifTrue: [ result <- 0 ].
        request <- request combineWith: (data asString) ].
    request grow: (0 asCharacter).
    httprequest <- HTTPRequest new: request; parseRequest.
    (handlers includesKey: (httprequest uri)) ifTrue: [
        handler <- handlers at: (httprequest uri).
        methodname <- ((httprequest method lowercase), ':from:') asSymbol.
        (handler respondsTo: methodname)
            ifTrue: [
                args <- Array new: 3; at: 1 put: handler; at: 2 put: request; at: 3 put: socket.
                method <- handler class methodNamed: methodname.
                method executeWith: args
            ]
    ]
]

Methods Scheduler 'get commands'
    initialize  | string |
        Module require: 'sockets'.
        Server new; 
            addHandler: (Handler new) for: '/'; 
            addHandler: (TestHandler new) for: '/test'; 
            run
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
