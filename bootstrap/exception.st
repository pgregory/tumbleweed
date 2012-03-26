Class Exception Object

Methods MetaException 'all'
    signal
    | context |
        context <- scheduler currentProcess exceptionHandlerFor: self class.
        context notNil
            ifTrue: [
                'Exception handled' print.
                context handlerBlock value: self.
                scheduler currentProcess trace
            ]
]

