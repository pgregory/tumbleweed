Class HTTPRequest Object request requestLine method uri version
*
Methods MetaHTTPRequest 'creation'
    new: request
    | newobj |
    newobj <- super new.
    newobj init: request.
    ^ newobj
]
*
Methods HTTPRequest 'creation'
    init: request
    request <- request
]
*
Methods HTTPRequest 'parsing'
    parseRequest
    | lines words |
    lines <- request words: [ :x | ((x = (Char lf)) or: [ x = (Char cr)]) not ].
    ( lines size > 0 ) ifTrue: [
        requestLine <- lines at: 1.
        words <- requestLine words: [ :x | (x = $ ) not ].
        (words includesKey: 1) ifTrue: [
            method <- words at: 1.
            (words includesKey: 2) ifTrue: [
                uri <- words at: 2.
                (words includesKey: 3) ifTrue: [
                    version <- words at: 3
                ]
            ]
        ]    
    ]
]
Methods HTTPRequest 'access'
    requestLine
    ^ requestLine
|
    method
    ^ method
|
    uri
    ^ uri
|
    version
    ^ version
]

