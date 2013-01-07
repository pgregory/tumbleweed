RawClass MetaSocket Class MetaObject eagain
Class Socket Object socketID lastError

Methods MetaSocket 'creation'
  new: id
    | newobj |
    newobj <- super new.
    newobj init: id.
    ^ newobj
]
*
Methods MetaSocket 'testing'
  test: server on: port service: service protocol: protocol
    | sock data |
    'Creating socket: ' print.
    sock <- Socket new.
    'Connecting socket: ' print.
    sock connect: server using: service with: protocol.
    'Sending request: ' print.
    sock send: ('GET / HTTP/1.0', Char cr, Char lf, Char cr, Char lf).
    'Reading response: ' print.
    [ (data <- sock read: 255) isNil ] whileFalse:
      [ data printNoReturn ].
    'Done: ' print
|
  test1
    self test: 'www.google.com' on: 80 service: 'http' protocol: 'tcp'
|
  testserver
    | sock result data readData response clientsock |
    'Creating socket: ' print.
    sock <- Socket new.
    'Binding socket: ' print.
    result <- sock bindTo: 8090.
    (result == 0) 
      ifTrue: [ 'Listening: ' print ]
      ifFalse: [ ('Could not bind to socket: (', result, ')') print. ^ false ].
    sock listen.
    'Accepting: ' print.
    [ true ] whileTrue: [
      result <- sock accept.
      ((result at: 1) < 0) ifTrue: [
        scheduler yield
      ] ifFalse: [
        clientsock <- Socket new: (result at: 1).
        [ self handleConnection: clientsock ] fork
      ]
    ].
    'Shutting down server' print.
    sock close.
    'Done: ' print
|
  handleConnection: clientsock
    | data result readData response | 
    result <- false.
    data <- ''.
    [ result ] whileFalse: [ 
      readData <- clientsock read: 255.
      ( ( readData at: 2 ) isNil ) ifTrue: [ 
        ( ( readData at: 1 ) == 0 ) ifTrue: [
          result <- true.
          clientsock close
        ]
      ] ifFalse: [ 
        data <- (data, (readData at: 2)).
        ( ( data findSubstring: ('', Char cr, Char lf, Char cr, Char lf) ) > 0 ) ifTrue: [ 
          response <- data value.
          clientsock send: ((response asString), Char cr, Char lf).
          data <- ''
        ]
      ]
    ]
]
*
Methods MetaSocket 'constants'
  eagain
  | sktFFI |
    (eagain isNil) ifTrue: [
      sktFFI <- FFI new: 'modules/sockets/sockets'.
      eagain <- (sktFFI call: 'eagain' returning: #int taking: #() as: #()) at: 1.
      sktFFI close
    ].
    ^ eagain
]
*
Methods Socket 'initialization'
  new
    " TODO: error checking "
    self init: <socketsNew 'tcp'>
|
  init: id
    socketID <- id.
    lastError <- 0
]
*
Methods Socket 'all'
  socketID
    ^ socketID
|
  lastError
    ^ lastError
|
  shutdown
    ^ <socketsShutdown socketID>
|
  close
    ^ <socketsClose socketID>
|
  connect: server
    ^ self connect: server using: 'http'
|
  connect: server using: service
    ^ self connect: server using: service with: 'tcp'
|
  connect: server using: service with: protocol
    | result |
    result <- <socketsConnect socketID server service protocol>.
    ^ result
|
  send: data
    | result |
    result <- <socketsSend socketID data 0>.
    ^ result
|
  read: size
    | data |
    data <- <socketsRead socketID size>.
    ^ data
|
  bindTo: service with: protocol
    | result |
    result <- <socketsBind socketID service protocol>.
    ^ result
|
  bindTo: port
    | result |
    result <- <socketsBindToPort socketID port>.
    ^ result
|
  listen
    | result |
    result <- <socketsListen socketID 0>.
    ^ result
|
  accept
    | result |
    result <- <socketsAccept socketID>.
    "^ (Socket new: (result at: 1))"
    ^ result
|
  printString
    ^ ('Socket: ', socketID)
]
    
    
        

