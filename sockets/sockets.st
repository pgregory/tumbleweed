Class Socket Object sktFFI socketID

Methods MetaSocket 'testing'
    test: server on: port service: service protocol: protocol
    | sock result data |
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
]
*
Methods Socket 'initialization'
  new
    sktFFI <- FFI new: 'sockets'.
    socketID <- (sktFFI call: 'Socket' returning: #int taking: #(string) as: #('tcp')) at: 1
]
*
Methods Socket 'all'
  socketID
    ^ socketID
|
  connect: server
    ^ self connect: server using: 'http'
|
  connect: server using: service
    ^ self connect: server using: service with: 'tcp'
|
  connect: server using: service with: protocol
    | result args |
    args <- Array new: 4.
    args at: 1 put: socketID; at: 2 put: server; at: 3 put: service; at: 4 put: protocol.
    result <- (sktFFI call: 'Connect' returning: #int taking: #(int string string string) as: args) at: 1.
    ^ result
|
  send: data
    | result args |
    args <- Array new: 4.
    args at: 1 put: socketID; at: 2 put: data; at: 3 put: (data size); at: 4 put: 0.
    result <- (sktFFI call: 'Send' returning: #int taking: #(int string int int) as: args) at: 1.
    ^ result
|
  read: buffer
    | result args |
    args <- Array new: 3.
    args at: 1 put: socketID; at: 2 put: buffer; at: 3 put: (buffer size).
    result <- (sktFFI call: 'Read' returning: #int taking: #(int string int) as: args) at: 1.
    (result < (buffer size)) ifTrue:
      [ buffer at: (result + 1) put: (0 asCharacter) ].
    ^ result
]
    
    
        

