Class Exception Object

Methods MetaException 'all'
    signal
    | context |
        context <- scheduler currentProcess exceptionHandlerFor: self class.
        context notNil
            ifTrue: [
                context handlerBlock value: self
            ]
]

