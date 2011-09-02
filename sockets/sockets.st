Class Socket Object

Methods MetaSocket 'creation'
    test: server on: port service: service protocol: protocol
    | sock skt args result data get |
    args <- Array new: 4.
    args at: 1 put: 'tcp'.
    skt <- FFI new: 'sockets'.
    'Creating socket: ' print.
    sock <- (skt call: 'Socket' returning: #int taking: #(string) as: args) at: 1.
    sock print.
    " TODO: error checking "
    args at: 1 put: sock; at: 2 put: server; at: 3 put: service; at: 4 put: protocol.
    result <- skt call: 'Connect' returning: #int taking: #(int string string string) as: args.
    result print.
    get <- ('GET / HTTP/1.0', Char cr, Char lf, Char cr, Char lf).
    args at: 2 put: get; at: 3 put: get size; at: 4 put: 0.
    result <- skt call: 'Send' returning: #int taking: #(int string int int) as: args.
    result print.
    data <- ByteArray new: 255.
    args at: 2 put: data; at: 3 put: 255.
    result <- skt call: 'Read' returning: #int taking: #(int string int) as: args.
    (data asString) print
|
    test1
        self test: 'www.google.com' on: 80 service: 'http' protocol: 'tcp'
]
    
