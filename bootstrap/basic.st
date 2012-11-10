*
* Little Smalltalk, version 3
* basic methods needed for execution, including
*   object creation
*   block creation, execution and return
*
RawClass MetaObject Class Class Dependencies
Class Object
Class Block Object context argCount argLoc bytePointer
Class Boolean Object
Class    True Boolean
Class    False Boolean
Class Class Object name instanceSize methods superClass variables
Class Context Object linkLocation method arguments temporaries
Class Zebra Object
Class Method Object text message bytecodes literals stackSize temporarySize class watch protocol
Class Smalltalk Object
Class Switch Object const notdone
Class Symbol Object
Class UndefinedObject Object
Class CPointer Object
Class ObjectMemory Object
*
Methods MetaObject 'all'
    dependencies
        ^ Dependencies
|
    dependencies: anObject
        Dependencies <- anObject
|
    doInit
        Dependencies <- Dictionary new
]
*
Methods Object 'aspect'
    dependents
    | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [^ List new].
        ^ dependencies 
|
    addDependent: anObject
        (Object dependencies at: self ifAbsentPut: [List new])
            add: anObject
|
    removeDependent: anObject
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [^anObject].
        dependencies removeKey: anObject ifAbsent: [].
        dependencies size < 1 ifTrue: [
            Object dependencies removeKey: self ifAbsent: []
        ].
        ^anObject
|
    changed
        self changed: self
|
    changed: aParameter
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [nil].
        dependencies notNil ifTrue: [
            dependencies do: [ :dep | dep update: aParameter ]
        ]
|
    update: aParameter
        nil
|
    broadcast: aSymbol
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [nil].
        dependencies notNil ifTrue: [
            dependencies do: [:dep | dep perform: aSymbol]
        ]
|
    broadcast: aSymbol with: anObject
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [nil].
        dependencies notNil ifTrue: [
            dependencies do: [:dep | dep perform: aSymbol with: anObject]
        ]
|
    perform: selector
        | args |
        (selector isMemberOf: Symbol) ifTrue: [
            (self respondsTo: selector) ifTrue: [
                args <- Array new: 1; at: 1 put: self.
                smalltalk perform: selector withArguments: args
            ]
        ] 
|
    perform: selector with: arg
        | args |
        (selector isMemberOf: Symbol) ifTrue: [
            (self respondsTo: selector) ifTrue: [
                args <- Array new: 2; at: 1 put: self; at: 2 put: arg.
                smalltalk perform: selector withArguments: args
            ]
        ] 
]
*
Methods Block 'initialization'
    checkArgumentCount: count
        ^ (argCount = count)
            ifTrue: [ true ]
            ifFalse: [ smalltalk error:
                'wrong number of arguments passed to block'.
                false ]
|
    blockContext: ctx
        context <- ctx
|
    value
        ^ (self checkArgumentCount: 0)
            ifTrue: [ context returnToBlock: bytePointer ]
|
    value: x
        ^ (self checkArgumentCount:  1)
            ifTrue: [ context at: argLoc put: x.
                  context returnToBlock: bytePointer ]
|
    value: x value: y
        ^ (self checkArgumentCount: 2)
            ifTrue: [ context at: argLoc put: x.
                  context at: argLoc + 1 put: y.
                  context returnToBlock: bytePointer ]
|
    value: x value: y value: z
        ^ (self checkArgumentCount:  3)
            ifTrue: [ context at: argLoc put: x.
                  context at: argLoc + 1 put: y.
                  context at: argLoc + 2 put: z.
                  context returnToBlock: bytePointer ]
|
    whileTrue: aBlock
        ( self value ) ifTrue:
            [ aBlock value. 
                self whileTrue: aBlock ]
|
    whileTrue
        self whileTrue: []
|
    whileFalse: aBlock
        [ self value not ] whileTrue: aBlock
|
    on: anException do: aBlock
        scheduler currentProcess recordHandler: aBlock for: anException.
        ^ self valueAndResumeOnUnwind
|
    valueAndResumeOnUnwind
        ^ self value
]
Methods Boolean 'all'
    ifTrue: trueBlock
        ^ self ifTrue: trueBlock ifFalse: []
|
    ifFalse: falseBlock
        ^ self ifTrue: [] ifFalse: falseBlock
|
    ifFalse: falseBlock ifTrue: trueBlock
        ^ self ifTrue: trueBlock
            ifFalse: falseBlock
|
    and: aBlock
        ^ self ifTrue: aBlock ifFalse: [ false ]
|
    or: aBlock
        ^ self ifTrue: [ true ] ifFalse: aBlock
]
*
Methods Class 'creation'
    new     
    | newObj |
        newObj <- < 22 < 58 instanceSize > self >.
    (newObj isKindOf: Class) ifTrue: [
      newObj initialize ]
    ifFalse: [
      newObj new ].
    ^ newObj
|
  subClass: aSymbol
    ^ self subClass: aSymbol instanceVariableNames: (Array new: 0)
|
  subClass: aSymbol instanceVariableNames: v
    ^ self subClass: aSymbol instanceVariableNames: v classVariableNames: (Array new: 0)
|
    subClass: aSymbol instanceVariableNames: v classVariableNames: cv
    | newClass newMetaClass |
    newMetaClass <- Class new.
    newMetaClass name: ('Meta', aSymbol asString); superClass: self class; variables: cv.
    newClass <- newMetaClass new.
    newClass name: aSymbol; superClass: self; variables: v.
    aSymbol assign: newClass.
    classes at: aSymbol put: newClass.
    ^ newClass 
|
    initialize
        superClass <- Object.
        instanceSize <- 0.
        methods <- Dictionary new
|
    methods
        ^ methods
|
    methodNamed: name
        (methods includesKey: name)
            ifTrue: [ ^ methods at: name ].
        (superClass notNil)
            ifTrue: [ ^ superClass methodNamed: name ].
        ^ nil
|
  protocols
    | protocols |
    protocols <- Set new.
    (methods) do: [ :m | protocols add: (m protocol) ].
    ^ protocols
|
  methodsFor: protocol
    ^ methods select: [ :m | m protocol = protocol ]
|
    name
        ^ name
|
    name: aString
        name <- aString
|
    instanceSize
        ^ instanceSize
|
    printString
        ^ name asString
|
    respondsTo  | theSet |
        theSet <- Dictionary new.
        self upSuperclassChain: 
            [:x | theSet addAll: x methods ].
        ^ theSet
|
    subClasses
        ^ classes inject: List new
            into: [:x :y | (y superClass == self)
                        ifTrue: [ x add: y]. x ]
|
  viewSubClassTree: indent
    classes do: [ :obj |
      ((obj isKindOf: Class) and: [ obj superClass == self])
        ifTrue: [
          (1 to: indent) do: [:ignore | ' ' printNoReturn ].
          obj print.
          obj viewSubClassTree: indent + 4 ] ]
|
  viewSubClassTree
    self viewSubClassTree: 0
|
    superClass
        ^ superClass
|
    superClass: aClass
        superClass <- aClass
|
    upSuperclassChain: aBlock
        aBlock value: self.
        (superClass notNil)
            ifTrue: [ superClass upSuperclassChain: aBlock ]
|
    variables
        ^ variables
|
    variables: nameArray
        variables <- nameArray.
        instanceSize <- superClass instanceSize + nameArray size
|
    watch: name | m |
        m <- self methodNamed: name.
        (m notNil) 
            ifTrue: [ ^ m watch: 
                [:a | ('executing ', name) print. a print] ]
            ifFalse: [ ^ 'no such method' ]
|
  isMeta
    (name asString size < 4) ifTrue: [ ^ false ].
    ^ (name asString from: 1 to: 4) = 'Meta'
]
Methods Context 'all'
    at: key put: value
        temporaries at: key put: value
|
    method: m
        method <- m
|
    arguments: a
        arguments <- a
|
    temporaries: t
        temporaries <- t
|
    returnToBlock: bytePtr
        " change the location we will return to, to execute a block"
        <28 self bytePtr>
|
    copy
        ^ super copy temporaries: temporaries copy
|
    blockReturn
        <18 self>
            ifFalse: [ ^ smalltalk error: 
                'incorrect context for block return']
]


Methods False 'all'
    ifTrue: trueBlock ifFalse: falseBlock
        ^ falseBlock value
|
    not
        ^ true
|
    xor: aBoolean
        ^ aBoolean
|
    printString
        ^ 'false'
]
Methods Method 'all'
    compileWithClass: aClass
        ^ <39 aClass text self>
|
    name
        ^ message
|
    message: aSymbol
        message <- aSymbol
|
    printString
        ^ message asString
|
    signature
        ^ class asString,' ', message asString
|
    text
        ^ (text notNil)
            ifTrue: [ text ]
            ifFalse: [ 'text not saved']
|
    text: aString
        text <- aString
|
    display
        ('Method ', message) print.
        'text' print.
        text print.
        'literals' print.
        literals print.
        'bytecodes' print.
        bytecodes class print.
        bytecodes do: [:x |
            (x printString, ' ', (x quo: 16), ' ', (x rem: 16))
                print ]
|
    executeWith: arguments
        ^ ( Context new ; method: self ; 
            temporaries: ( Array new: temporarySize) ;
            arguments: arguments )
           returnToBlock: 1
|
    watch: aBlock
        watch <- aBlock
|
    watchWith: arguments
        " note that we are being watched "
        text print.
        watch value: arguments.
        ^ self executeWith: arguments
|
  protocol
    ^ protocol
|
  protocol: aString
    protocol <- aString
]
Methods Object 'all'
    assign: name value: val
        ^ name assign: val
|
    == aValue
        ^ <21 self aValue>
|
    ~~ aValue
        ^ (self == aValue) not
|
    = aValue
        ^ self == aValue
|
    ~= avalue
        ^ self ~~ aValue
|
    asString
        ^ self printString
|
    basicAt: index
        ^ <25 self index>
|
    basicAt: index put: value
        ^ <31 self index value>
|
    basicSize
        ^ <12 self>
|
    class
        ^ <11 self>
|
    copy
        ^ self shallowCopy
|
    deepCopy    | newObj |
        newObj <- self class new.
        (1 to: self basicSize) do: 
            [:i | newObj basicAt: i put: (self basicAt: i) copy].
        ^ newObj
|
    display
        ('(Class ', self class, ') ' , self printString ) print
|
    hash
        ^ <13 self>
|
    isMemberOf: aClass
        ^ self class == aClass
|
    isNil
        ^ false
|
    isKindOf: aClass
        self class upSuperclassChain:
            [:x | (x == aClass) ifTrue: [ ^ true ] ].
        ^ false
|
    new
        " default initialization protocol"
        ^ self
|
    notNil
        ^ true
|
    print
        self printString print 
|
    printString
        ^ self class printString
|
    respondsTo: message
        self class upSuperclassChain: 
            [:c | (c methodNamed: message) notNil
                    ifTrue: [ ^ true ]].
        ^ false
|
    shallowCopy | newObj |
        newObj <- self class new.
        (1 to: self basicSize) do: 
            [:i | newObj basicAt: i put: (self basicAt: i) ].
        ^ newObj
]
Methods Smalltalk 'all'
    perform: message withArguments: args ifError: aBlock    
            | receiver method |
        receiver <- args at: 1 ifAbsent: [ ^ aBlock value ].
        method <- receiver class methodNamed: message.
        ^ method notNil 
            ifTrue: [ method executeWith: args ]
            ifFalse: aBlock
|
    perform: message withArguments: args
        ^ self perform: message withArguments: args
            ifError: [ self error: 'cant perform' ]
|
    watch
        ^ <5>
]
Methods True 'all'
    ifTrue: trueBlock ifFalse: falseBlock
        ^ trueBlock value
|
    not
        ^ false
|
    xor: aBoolean
        ^ aBoolean not
|
    printString
        ^ 'true'
]
Methods Switch 'all'
    key: value
        const <- value.
        notdone <- true.
|
    ifMatch: key do: block
        (notdone and: [ const = key ])
            ifTrue: [ notdone <- false. block value ]
|
    else: block
        notdone ifTrue: [ notdone <- false. block value ]
]
Methods Symbol 'all'
        apply: args
        ^ self apply: args ifError: [ 'does not apply' ]
|
        apply: args ifError: aBlock
        ^ smalltalk perform: self withArguments: args ifError: aBlock
|
    assign: value
        <27 self value>. ^ value
|
    asString
        " catenation makes string and copy automatically "
        ^ <24 self ''>
|
    copy
        ^ self
|
    printString
        ^ '#' , self asString
|
    respondsTo
        ^ classes inject: Set new
            into: [:x :y | ((y methodNamed: self) notNil)
                        ifTrue: [ x add: y]. x]
|
    value
        ^ <87 self>
]
Methods UndefinedObject 'all'
    isNil
        ^ true
|
    notNil
        ^ false
|
    printString
        ^ 'nil'
]
Methods Object 'errors'
    message: m notRecognizedWithArguments: a
        | msg |
        msg <- 'Message ', (m printString), ' not recognized on ', (self class printString), ' with arguments [', (a printString), ']'.
        Error with: msg; signal
]
*
Methods MetaCPointer 'creation'
  new
    ^ <6>
]
Methods CPointer 'all'
  asString
    ^ <141 self>
|
  printString
    ^ self asString
]
