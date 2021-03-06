RawClass MetaSocket Class MetaObject eagain
Class Socket Object sktFFI socketID

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
    | sock result data args |
    args <- Array new: 4.
    args at: 1 put: 'tcp'.
    'Creating socket: ' print.
    sock <- Socket new.
    'Connecting socket: ' print.
    sock connect: server using: service with: protocol.
    'Sending request: ' print.
    sock send: ('GET / HTTP/1.0', Char cr, Char lf, Char cr, Char lf).
    data <- ByteArray new: 255.
    'Reading response: ' print.
    result <- 1.
    [ result > 0 ] whileTrue:
      [ result <- sock read: data.
        (data asString) printNoReturn ].
    'Done: ' print
|
  test1
    self test: 'www.google.com' on: 80 service: 'http' protocol: 'tcp'
|
  testserver
    | sock result args data clientsock |
    args <- Array new: 4.
    args at: 1 put: 'tcp'.
    'Creating socket: ' print.
    sock <- Socket new.
    'Binding socket: ' print.
    result <- sock bindTo: 8090.
    (result = 0) 
      ifTrue: [ 'Listening: ' print ]
      ifFalse: [ 'Could not bind to socket: ' print. ^ false ].
    sock listen.
    data <- ByteArray new: 255.
    'Accepting: ' print.
    [ true ] whileTrue: [
      result <- sock accept.
      clientsock <- Socket new: (result at: 1).
      result <- clientsock read: data.
      clientsock send: '<html><body><h1>Hello from Tumbleweed</h1></body></html>'.
      clientsock shutdown
    ].
    'Shutting down server' print.
    sock shutdown.
    'Done: ' print
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
    sktFFI <- FFI new: 'modules/sockets/sockets'.
    " TODO: error checking "
    socketID <- (sktFFI call: 'Socket' returning: #int taking: #(string intOut) as: #('tcp', 0)) at: 1
|
  init: id
    socketID <- id
]
*
Methods Socket 'all'
  socketID
    ^ socketID
|
  shutdown
    | args result |
    args <- Array new: 2.
    args at: 1 put: socketID; at: 2 put: 0.
    result <- (sktFFI call: 'Shutdown' returning: #int taking: #(int intOut) as: args) at: 1.
    ^ result
|
  connect: server
    ^ self connect: server using: 'http'
|
  connect: server using: service
    ^ self connect: server using: service with: 'tcp'
|
  connect: server using: service with: protocol
    | result args |
    args <- Array new: 5.
    args at: 1 put: socketID; at: 2 put: server; at: 3 put: service; at: 4 put: protocol.
    args at: 5 put: 0.
    result <- (sktFFI call: 'Connect' returning: #int taking: #(int string string string intOut) as: args) at: 1.
    ^ result
|
  send: data
    | result args |
    args <- Array new: 5.
    args at: 1 put: socketID; at: 2 put: data; at: 3 put: (data size); at: 4 put: 0.
    args at: 5 put: 0.
    result <- (sktFFI call: 'Send' returning: #int taking: #(int string int int intOut) as: args) at: 1.
    ^ result
|
  read: buffer
    | result args |
    args <- Array new: 4.
    args at: 1 put: socketID; at: 2 put: buffer; at: 3 put: (buffer size); at: 4 put: 0.
    result <- (sktFFI call: 'Read' returning: #int taking: #(int string int intOut) as: args) at: 1.
    ^ result
|
  bindTo: service with: protocol
    | result args |
    args <- Array new: 4.
    args at: 1 put: socketID; at: 2 put: service; at: 3 put: protocol; at: 4 put: 0.
    result <- (sktFFI call: 'Bind' returning: #int taking: #(int string string intOut) as: args) at: 1.
    ^ result
|
  bindTo: port
    | result args |
    args <- Array new: 3.
    args at: 1 put: socketID; at: 2 put: port; at: 3 put: 0.
    result <- (sktFFI call: 'BindToPort' returning: #int taking: #(int int intOut) as: args) at: 1.
    ^ result
|
  listen
    | result args |
    args <- Array new: 3.
    args at: 1 put: socketID; at: 2 put: 5; at: 3 put: 0.
    result <- (sktFFI call: 'Listen' returning: #int taking: #(int int intOut) as: args) at: 1.
    ^ result
|
  accept
    | result args |
    args <- Array new: 2.
    args at: 1 put: socketID; at: 2 put: 0.
    result <- sktFFI call: 'Accept' returning: #int taking: #(int intOut) as: args.
    "^ (Socket new: (result at: 1))"
    ^ result
|
  printString
    ^ ('Socket: ', socketID)
]
    
    
        

