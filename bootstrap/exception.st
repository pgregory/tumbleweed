Class Exception Object message

Methods MetaException 'all'
    signal
    | context |
        context <- scheduler currentProcess exceptionHandlerFor: self.
        context notNil
            ifTrue: [
                context handlerBlock value: self
            ]
|
    signal: aString
        message <- aString.
        self signal
|
    message
        ^ message
]

