Class Exception Object message

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
        context <- scheduler currentProcess exceptionHandlerFor: self.
        context notNil
            ifTrue: [
                context handlerBlock value: self
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

