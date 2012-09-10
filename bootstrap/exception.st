Class Exception Object message
Class ExceptionHandler Object handledException handlerBlock
Class Error Exception

Methods MetaException 'all'
    with: aString
    | ex |
    ex <- super new.
    ex message: aString.
    ^ ex
]

Methods Exception
    signal
    | context |
        handler <- scheduler currentProcess exceptionHandlerFor: self.
        handler notNil
            ifTrue: [
                (handler at: 1) handlerBlock value: self.
                <56 (handler at: 2)>
            ]
            ifFalse: [
                smalltalk error: 'Unhandled Exception "', message, '"'
            ]
|
    signalWith: aString
        message <- aString.
        self signal
|
    message: aString
        message <- aString
|
    message
        ^ message
]

Methods MetaExceptionHandler 'creation'
    for: anException with: aBlock
    | handler |
    handler <- super new.
    handler handledException: anException.
    handler handlerBlock: aBlock.
    ^ handler
]

Methods ExceptionHandler 'all'
    handledException
        ^ handledException
|
    handledException: anException
        handledException <- anException
|
    handlerBlock
        ^ handlerBlock
|
    handlerBlock: aBlock
        handlerBlock <- aBlock
|
    handles: anException
        ^ anException isKindOf: handledException
]

