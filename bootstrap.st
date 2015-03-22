RawClass Class MetaClass Object name instanceSize methods superClass variables
Class  nil
RawClass MetaObject Class Class Dependencies
Class Object 
Class HTTPRequest Object request requestLine method uri version
Class Widget Object handle
Class WidgetContainer Widget children
Class CPointer Object
Class Panel Object browser widget
RawClass MetaNode Class MetaObject errorException
Class Node Object parent children name
Class Magnitude Object
Class Method Object text message bytecodes literals stackSize temporarySize class watch protocol
Class Menu WidgetContainer
Class Number Magnitude
Class ListControl Widget
Class NullQueen Object
Class Random Object
Class Collection Magnitude
Class IndexedCollection Collection
Class Dictionary IndexedCollection hashTable
RawClass MetaTestCase Class MetaObject failedCheckSignal
Class TestCase Object selector
Class StringTest TestCase string emptyString subcollection3ElementsSorted nonEmpty5ElementsSorted unsortedCollection indexInNonEmptyArray arrayWithCharacters nonEmpty1element withoutEqualElements sameAtEndAndBegining elementInNonEmpty collectionNotIncluded notIn
Class ButtonControl Widget
Class TestResult Object startTime stopTime Time testName failures errors
Class LabelControl Widget
Class HBox WidgetContainer
Class ObjectMemory Object
Class Boolean Object
Class Class Object name instanceSize methods superClass variables
RawClass MetaFFI Class MetaObject libraries
Class FFI Object handle name functions
Class Dialog WidgetContainer
Class List Collection links
Class Set List
Class Switch Object const notdone
Class Array IndexedCollection
Class ByteArray Array
Class ExternalData Object members
Class EDTeset ExternalData
Class Float Number
Class Fraction Number top bottom
Class DefinitionPanel Panel text commands
Class Exception Object message
Class Error Exception
Class nil 
Class SetTest TestCase
Class Smalltalk Object
Class ExceptionHandler Object handledException handlerBlock
Class NodeException Exception
Class AddSubClassDialog Object dialog parentClass nameControl ivnamesControl
Class Fill Widget
Class ItemControl Widget
Class UndefinedObject Object
Class TestSuite Object name testCases
Class TestConversions TestCase
Class Char Magnitude value
Class VBox WidgetContainer
Class Zebra Object
Class ArrayTest TestCase
Class Process Object stack stackTop linkPointer
Class Symbol Object
Class String ByteArray
Class Browser Object panels dialog currentClass currentMethod
Class NavigationPanel Panel classList
RawClass MetaFLTK Class MetaObject fltkFFI
Class FLTK Object
Class EmployeeRecord Object name idNumber position salary
Class False Boolean
Class Semaphore Object count processList
Class True Boolean
Class Split WidgetContainer
Class Rectangle Object top left bottom right
RawClass MetaIUP Class MetaObject theInstance
Class IUP Object iup
Class Interval Collection lower upper step
Class TextControl Widget
Class Integer Number
Class LongInteger Integer negative digits
Class NodeTest TestCase nodeA nodeB
Class Block Object context argCount argLoc bytePointer
Class EDTest nil
Class Morph Node bounds color alpha is
Class Frame Morph
Class World Frame
Class File Object name number mode
Class ButtonPanel Panel buttons
Class FailedCheckSignal Exception
RawClass MetaTime Class MetaMagnitude ClockAtStartup
Class Time Magnitude
Class TestRunner Object
Class Test nil
Class Context Object linkLocation method arguments temporaries
Class Circle Object center radius
Class Scheduler Object notdone processList currentProcess
Class Link Object key value nextLink
Class EmployeeDatabase Object records
RawClass MetaModule Class MetaObject modules
Class Module Object name root
Class Queen Object row column neighbor
Class Workspace Object dialog commands
Class Point Magnitude x y
RawClass MetaFLTKWindow Class MetaObject
Class FLTKWindow Object handle
Methods HTTPRequest

    version
    ^ version


]
Methods HTTPRequest
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
Methods HTTPRequest
    init: request
    request <- request


]
Methods HTTPRequest

    uri
    ^ uri


]
Methods HTTPRequest

    method
    ^ method


]
Methods HTTPRequest
    requestLine
    ^ requestLine


]
Methods WidgetContainer

  children
    ^ children


]
Methods WidgetContainer

  addChild: child
    children add: child.
    IUP instance IupAppend: (child handle) to: (self handle).
    ^ self


]
Methods WidgetContainer
  init
    super new.
    children <- List new.
    ^ self 


]
Methods CPointer

  printString
    ^ self asString


]
Methods CPointer
  asString
    ^ <141 self>


]
Methods Panel
  init: browser
    browser <- browser.


]
Methods Panel

  widget
    ^ widget


]
Methods Node
  init: aString
    children <- Dictionary new.
    parent <- nil.
    name <- aString


]
Methods Node

  children
    ^ children


]
Methods Node

  parent
    ^ parent


]
Methods Node

  addChild: aNode  
    children at: (aNode name) put: aNode.
    aNode setParent: self


]
Methods Node

  errorException
    ^ errorException


]
Methods Node

  removeChild: aNode
    ( children includesKey: (aNode name) ) ifFalse: [
      errorException signalWith: ('Child "', (aNode name), '" is not a member')
    ].
    children removeKey: (aNode name)


]
Methods Node

  setParent: aNode
    (parent isNil) ifFalse: [
      parent removeChild: self
    ].
    parent <- aNode


]
Methods Node

  name
    ^ name


]
Methods Magnitude

    >= value
        ^ value <= self


]
Methods Magnitude

    isChar
        ^ false


]
Methods Magnitude
    <= value
        ^ (self < value) or: [ self = value ]


]
Methods Magnitude

    max: value
        ^ (self < value)
            ifTrue: [ value ]
            ifFalse: [ self ]


]
Methods Magnitude

    between: low and: high
        ^ (low <= self) and: [ self <= high ]


]
Methods Magnitude

    > value
        ^ (value < self)


]
Methods Magnitude

    < value
        ^ (self <= value) and: [ self ~= value ]


]
Methods Magnitude

    ~= value
        ^ (self = value) not


]
Methods Magnitude

    min: value
        ^ (self < value)
            ifTrue: [ self ]
            ifFalse: [ value ]


]
Methods Magnitude

    = value
        ^ (self == value)


]
Methods Method
disassemble: indent at: initPC for: initCount | pc low high count needCR |
  pc <- initPC.
  count <- initCount.
  [ count > 0 ] whileTrue: [
    " Default, terminate line with CR "
    needCR <- true.

    " Show PC and indent listing of line "
    ((pc rem: 20) = 0) ifTrue: [
      low <- String input.
      ((low size > 0) and: [ (low at: 1) = $q ]) ifTrue: [
        self error: 'Disassembly halted'
      ]
    ].
    (pc - 1) print.
    (1 to: indent) do: [:x| ' ' print].

    " Fetch basic opcode fields "
    high <- bytecodes at: pc.
    pc <- pc + 1.
    count <- count - 1.
    low <- high rem: 16.
    high <- high quo: 16.
    (high = 0) ifTrue: [
      high <- low.
      low <- bytecodes at: pc.
      pc <- pc + 1.
      count <- count - 1
    ].

    (high = 1) ifTrue: [
      'PushInstance ' print.
      low print
    ].

    (high = 2) ifTrue: [
      'PushArgument ' print.
      low print
    ].

    (high = 3) ifTrue: [
      'PushTemporary ' print.
      low print
    ].

    (high = 4) ifTrue: [
      'PushLiteral ' print.
      low print
    ].

    (high = 5) ifTrue: [
      'PushConstant ' print.
      ((low >= 0) and: [low < 10]) ifTrue: [
        low print
      ].
      (low = 10) ifTrue: [
        'nil' print
      ].
      (low = 11) ifTrue: [
        'true' print
      ].
      (low = 12) ifTrue: [
        'false' print
      ]
    ].

    (high = 6) ifTrue: [
      'AssignInstance ' print.
      low print
    ].

    (high = 7) ifTrue: [
      'AssignTemporary ' print.
      low print
    ].

    (high = 8) ifTrue: [
      'MarkArguments ' print.
      low print
    ].

    (high = 9) ifTrue: [
      'SendMessage ' print.
      (literals at: (low+1)) print
    ].

    (high = 10) ifTrue: [
      'SendUnary ' print.
      (low = 0) ifTrue: [ 'isNil' print ].
      (low = 1) ifTrue: [ 'notNil' print ]
    ].

    (high = 11) ifTrue: [
      'SendBinary ' print.
      (low = 0) ifTrue: [ '<' print ].
      (low = 1) ifTrue: [ '<=' print ].
      (low = 2) ifTrue: [ '+' print ]
    ].

    (high = 12) ifTrue: [
      'PushBlock' print. Char newline print.
      low <- bytecodes at: pc.
      pc <- pc + 1.
      low <- low + ((bytecodes at: pc) * 256).
      pc <- pc + 1.
      high <- low - (pc-1).
      self disassemble: indent+4 at: pc for: high.
      count <- count - (high + 2).
      pc <- pc + high.
      needCR <- false
    ].

    (high = 13) ifTrue: [
      'DoPrimitive ' print.
      high <- bytecodes at: pc.
      pc <- pc + 1.
      count <- count - 1.

      " VM generic "
      (high = 1) ifTrue: [ '=' print ].
      (high = 2) ifTrue: [ 'class' print ].
      (high = 3) ifTrue: [ 'putchar' print ].
      (high = 4) ifTrue: [ 'size' print ].
      (high = 5) ifTrue: [ 'at:put:' print ].
      (high = 6) ifTrue: [ 'newProcess' print ].
      (high = 7) ifTrue: [ 'new' print ].
      (high = 8) ifTrue: [ 'block invoke' print ].
      (high = 9) ifTrue: [ 'getchar' print ].
      (high = 10) ifTrue: [ 'SmallInt +' print ].
      (high = 11) ifTrue: [ 'SmallInt /' print ].
      (high = 12) ifTrue: [ 'SmallInt %' print ].
      (high = 13) ifTrue: [ 'SmallInt <' print ].
      (high = 14) ifTrue: [ 'SmallInt =' print ].
      (high = 15) ifTrue: [ 'SmallInt *' print ].
      (high = 16) ifTrue: [ 'SmallInt -' print ].
      (high = 18) ifTrue: [ 'debug' print ].
      (high = 19) ifTrue: [ 'error' print ].
      (high = 20) ifTrue: [ 'ByteArray new' print ].
      (high = 21) ifTrue: [ 'String at:' print ].
      (high = 22) ifTrue: [ 'String at:put:' print ].
      (high = 23) ifTrue: [ 'String clone' print ].
      (high = 24) ifTrue: [ 'Array at:' print ].
      (high = 25) ifTrue: [ 'Integer /' print ].
      (high = 26) ifTrue: [ 'Integer %' print ].
      (high = 27) ifTrue: [ 'Integer +' print ].
      (high = 28) ifTrue: [ 'Integer *' print ].
      (high = 29) ifTrue: [ 'Integer -' print ].
      (high = 30) ifTrue: [ 'Integer <' print ].
      (high = 31) ifTrue: [ 'Integer =' print ].
      (high = 32) ifTrue: [ 'SmallInt asInteger' print ].
      (high = 33) ifTrue: [ 'Integer asSmallInt' print ].

      " UNIX port specific "
      (high = 100) ifTrue: [ 'UNIX fopen' print ].
      (high = 101) ifTrue: [ 'UNIX fgetc' print ].
      (high = 102) ifTrue: [ 'UNIX fputc' print ].
      (high = 103) ifTrue: [ 'UNIX fclose' print ].
      (high = 104) ifTrue: [ 'UNIX fileout' print ].
      (high = 105) ifTrue: [ 'UNIX edit' print ]
    ].

    (high = 15) ifTrue: [
      'DoSpecial ' print.
      (low = 1) ifTrue: [ 'selfReturn' print ].
      (low = 2) ifTrue: [ 'stackReturn' print ].
      (low = 3) ifTrue: [ 'blockReturn' print ].
      (low = 4) ifTrue: [ 'duplicate' print ].
      (low = 5) ifTrue: [ 'popTop' print ].
      (low = 6) ifTrue: [
        'branch ' print.
        high <- bytecodes at: pc.
        pc <- pc + 1.
        high <- high + ((bytecodes at: pc) * 256).
        pc <- pc + 1.
        count <- count - 2.
        high print
      ].
      (low = 7) ifTrue: [
        'branchIfTrue ' print.
        high <- bytecodes at: pc.
        pc <- pc + 1.
        high <- high + ((bytecodes at: pc) * 256).
        pc <- pc + 1.
        count <- count - 2.
        high print
      ].
      (low = 8) ifTrue: [
        'branchIfFalse ' print.
        high <- bytecodes at: pc.
        pc <- pc + 1.
        high <- high + ((bytecodes at: pc) * 256).
        pc <- pc + 1.
        count <- count - 2.
        high print
      ].
      (low = 11) ifTrue: [
        'sendToSuper ' print.
        low <- bytecodes at: pc.
        pc <- pc + 1.
        count <- count - 1.
        (literals at: low+1) print
      ].
      (low = 12) ifTrue: [ 'breakpoint' print ]
    ].
    needCR ifTrue: [
      Char newline print
    ]
  ]





]
Methods Method

    printString
        ^ message asString


]
Methods Method

    watch: aBlock
        watch <- aBlock


]
Methods Method
    compileWithClass: aClass
        ^ <39 aClass text self>


]
Methods Method

    signature
        ^ class asString,' ', message asString


]
Methods Method
disassemble
  self disassemble: 1 at: 1 for: (bytecodes size)



]
Methods Method

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


]
Methods Method

    watchWith: arguments
        " note that we are being watched "
        text print.
        watch value: arguments.
        ^ self executeWith: arguments


]
Methods Method

  protocol: aString
    protocol <- aString


]
Methods Method

    text
        ^ (text notNil)
            ifTrue: [ text ]
            ifFalse: [ 'text not saved']


]
Methods Method

    text: aString
        text <- aString


]
Methods Method

  protocol
    ^ protocol


]
Methods Method

    message: aSymbol
        message <- aSymbol


]
Methods Method

    name
        ^ message


]
Methods Method

    executeWith: arguments
        ^ ( Context new ; method: self ; 
            temporaries: ( Array new: temporarySize) ;
            arguments: arguments )
           returnToBlock: 1


]
Methods MetaChar
  cr
    ^ 13 asCharacter


]
Methods MetaChar
    new: aValue
    | newobj |
    newobj <- super new.
    newobj value: aValue.
    ^ newobj


]
Methods MetaChar

  lf 
    ^ 10 asCharacter


]
Methods Menu
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupMenu.
    super handle: (result at: 1).
    ^self


]
Methods Number

    + value
        ^ (self maxgen: value) + (value maxgen: self)


]
Methods Number

    \\ value
        " remainder after integer division "
        ^ self - (self // value * value)


]
Methods Number

    negative
        ^ self < 0


]
Methods Number

    squared
        ^ self * self


]
Methods Number
    @ v
        ^ Point new; x: self; y: v


]
Methods Number

    strictlyPositive
        ^ self > 0


]
Methods Number

    negated
        ^ 0 - self


]
Methods Number
    isNumber
        ^ true


]
Methods Number

    ceiling     | i |
        i <- self truncated.
        ^ ((self positive) and: [ self ~= i ])
            ifTrue: [ i + 1 ]
            ifFalse: [ i ]


]
Methods Number

    raisedTo: x | y |
        x negative 
            ifTrue: [ ^ 1 / (self raisedTo: x negated) ].
        x isShortInteger 
            ifTrue: [ (x = 0) ifTrue: [ ^ 1 ].
                  y <- (self raisedTo: (x quo: 2)) squared.
                  x odd ifTrue: [ y <- y * self ].
                  ^ y ]
                "use logrithms to do exponeneation"
            ifFalse: [ ^ ( x * self ln ) exp ]


]
Methods Number

    rem: value
        ^ self - ((self quo: value) * value)


]
Methods Number

    reciprocal
        ^ 1 / self


]
Methods Number

    sqrt
        ^ (self negative)
            ifTrue: [ smalltalk error: 'sqrt of negative']
            ifFalse: [ self raisedTo: 0.5 ]


]
Methods Number

    trucateTo: value
        ^ (self / value) trucated * value


]
Methods Number
printWidth: width base: base | res n dig wasNeg wide |
        res <- ''.
        (self negative) ifTrue: [
                wasNeg <- true.
                wide <- width-1.
                n <- self negated
        ] ifFalse: [
                wasNeg <- false.
                wide <- width.
                n <- self
        ].
        [true] whileTrue: [
                res <- ((n rem: base) asDigit) + res.
                n <- n quo: base.
                (n = 0) ifTrue: [
                        ((res size)+1) to: wide do: [:ignore|
                                res <- '0' + res
                        ].
                        wasNeg ifTrue: [ res <- '-' + res ].
                        ^res
                ]
        ]


]
Methods Number

    abs
        ^ (self < 0)
            ifTrue: [ 0 - self ]
            ifFalse: [ self ]


]
Methods Number

    / value
        ^ (self maxgen: value) / (value maxgen: self)


]
Methods Number

    ln
        ^ self asFloat ln


]
Methods Number

    copy
        ^ self


]
Methods Number

    quo: value
        ^ (self maxgen: value) quo: (value maxgen: self)


]
Methods Number

    < value
        ^ (self maxgen: value) < (value maxgen: self)


]
Methods Number

    roundTo: value
        ^ (self / value ) rounded * value


]
Methods Number

    * value
        ^ (self maxgen: value) * (value maxgen: self)


]
Methods Number

    floor       | i |
        i <- self truncated.
        ^ ((self negative) and: [ self ~= i ])
            ifTrue: [ i - 1 ]
            ifFalse: [ i ]


]
Methods Number

    fractionalPart
        ^ self - self truncated


]
Methods Number

    exp
        ^ self asFloat exp


]
Methods Number

    maxgen: value
        (self isNumber and: [ value isNumber ])
            ifFalse: [ ^ smalltalk error: 
                'arithmetic on non-numbers' ].
        ^ (self generality > value generality)
            ifTrue: [ self ]
            ifFalse: [ value coerce: self ]


]
Methods Number

    to: value by: step
        ^ Interval new; lower: self; upper: value; step: step


]
Methods Number

    isInteger
        ^ self isLongInteger or: [ self isShortInteger ]


]
Methods Number

    positive
        ^ self >= 0


]
Methods Number

    - value
        ^ (self maxgen: value) - (value maxgen: self)


]
Methods Number

    // value
        " integer division, truncate towards negative infinity"
        " see quo: "
        ^ (self / value) floor


]
Methods Number

    log: value
        ^ self ln / value ln


]
Methods Number

    sign
        ^ (self = 0) ifTrue: [ 0 ]
            ifFalse: [ self / self abs ]


]
Methods Number

    = value
        ^ value isNumber
            ifTrue: [ (self maxgen: value) = (value maxgen: self) ]
            ifFalse: [ false ]


]
Methods Number

    to: value
        ^ Interval new; lower: self; upper: value; step: 1


]
Methods ListControl

  setActionCallback: aBlock
    ^ super setCallbackFor: 'ACTION' as: aBlock returning: #int taking: #(cObject string int int)


]
Methods ListControl
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupList.
    super handle: (result at: 1).
    ^self


]
Methods ListControl

  count
    | result |
    result <- IUP instance IupGetAttribute: 'COUNT' on: handle.
    ^ result at: 1


]
Methods MetaFLTKWindow
new 
   | newWindow args result |
  newWindow <- super new.
  args <- Array new: 2.
  args at: 1 put: 340; at: 2 put: 180.
  result <- (FLTK fltkFFI) call: 'createWindow' returning: #cObject taking: #(int int) as: args.
  newWindow handle: (result at: 1).
  ^ newWindow 




]
Methods NullQueen

    checkRow: row column: column
        " we can't attack anything "

        ^ false


]
Methods NullQueen

    next
        ^ true


]
Methods NullQueen

    result
        ^ List new


]
Methods NullQueen
    first
        ^ true


]
Methods Random

    next: value | list |
        " return a list of random numbers of given size "
        list <- List new.
        value timesRepeat: [ list add: self next ].
        ^ list


]
Methods Random

    randInteger: value
        ^ 1 + (<3> rem: value)


]
Methods Random
    between: low and: high
        " return random number in given range "
        ^ (self next * (high - low)) + low


]
Methods Random

    next
        " convert rand integer into float between 0 and 1 "
        ^ (<3> rem: 1000) / 1000


]
Methods Random

    set: value
        " set seed for random number generator "
        <55 value>


]
Methods Widget
 
  handle: h
    handle <- h


]
Methods Widget

  at: name
    | result |
    result <- IUP instance IupGetAttribute: name on: (self handle).
    ^ result at: 1


]
Methods Widget

  handle
    ^ handle


]
Methods Widget

  setCallbackFor: action as: aBlock returning: type taking: args
  | cb |
  cb <- <183 type args aBlock>.
  ^ IUP instance IupSetCallbackOn: (self handle) for: (action asString) as: cb


]
Methods Widget
  init
    handle <- 0.
    ^ self


]
Methods Widget

  at: name put: value
  IUP instance IupStoreAttribute: name on: (self handle) as: value


]
Methods Dictionary

    removeKey: aKey ifAbsent: exceptionBlock
        ^ (self includesKey: aKey)
            ifTrue: [ self basicRemoveKey: aKey ]
            ifFalse: exceptionBlock


]
Methods Dictionary
    new
        hashTable <- Array new: 39


]
Methods Dictionary

    hash: aKey
        ^ 3 * ((aKey hash) rem: ((hashTable size) quo: 3))


]
Methods Dictionary

    display
        self binaryDo: [:x :y | (x printString , ' -> ', 
                    y printString ) print ]


]
Methods Dictionary

    removeKey: aKey
        ^ self removeKey: aKey
            ifAbsent: [ smalltalk error: 'remove key not found']


]
Methods Dictionary

    basicRemoveKey: aKey        | hashPosition link |
        hashPosition <- self hash: aKey.
        ((hashTable at: hashPosition + 1) = aKey)
            ifTrue: [ hashTable at: hashPosition + 1 put: nil.
                  hashTable at: hashPosition + 2 put: nil]
            ifFalse: [ link <- hashTable at: hashPosition + 3.
                (link notNil)
                    ifTrue: [ hashTable at: hashPosition + 3
                            put: (link removeKey: aKey) ]]


]
Methods Dictionary

    at: aKey ifAbsent: exceptionBlock   | hashPosition  link |

        hashPosition <- self hash: aKey.
        ((hashTable at: hashPosition + 1) = aKey)
            ifTrue: [ ^ hashTable at: hashPosition + 2].
        link <- hashTable at: hashPosition + 3.
        ^ (link notNil)
            ifTrue: [ link at: aKey ifAbsent: exceptionBlock ]
            ifFalse: exceptionBlock


]
Methods Dictionary

    at: aKey ifAbsentPut: aBlock
        ^ self at: aKey ifAbsent: [self at: aKey put: aBlock value]


]
Methods Dictionary

    at: aKey put: aValue            | hashPosition link |

        hashPosition <- self hash: aKey.
        ((hashTable at: hashPosition + 1) isNil)
           ifTrue: [ hashTable at: hashPosition + 1 put: aKey ].
        ((hashTable at: hashPosition + 1) = aKey)
           ifTrue: [ hashTable at: hashPosition + 2 put: aValue ]
           ifFalse: [ link <- hashTable at: hashPosition + 3.
            (link notNil)
                ifTrue: [ link at: aKey put: aValue ]
                ifFalse: [ hashTable at: hashPosition + 3
                    put: (Link new; key: aKey; value: aValue)]].
        ^ aValue


]
Methods Dictionary

    includesKey: aKey
        " look up, but throw away result "
        self at: aKey ifAbsent: [ ^ false ].
        ^ true


]
Methods Dictionary

    binaryDo: aBlock
        (1 to: hashTable size by: 3) do:
            [:i | (hashTable at: i) notNil
                ifTrue: [ aBlock value: (hashTable at: i)
                        value: (hashTable at: i+1) ].
                  (hashTable at: i+2) notNil
                ifTrue: [ (hashTable at: i+2) 
                        binaryDo: aBlock ] ]


]
Methods StringTest

    aValue
        ^elementInNonEmpty


]
Methods StringTest

    anIndex
        ^2


]
Methods StringTest
    setUp
        string <- 'Hi, I am a String'.
        emptyString <- ''.
        subcollection3ElementsSorted <- 'bcd'.
        nonEmpty5ElementsSorted <- 'a' , subcollection3ElementsSorted , 'e'.
        unsortedCollection <- 'azsbe'.
        indexInNonEmptyArray <- #(1 3 2 ).
        arrayWithCharacters <- #($a $b $c ).
        nonEmpty1element <- 'a'.
        withoutEqualElements <- 'abcde'.
        sameAtEndAndBegining <- 'abca'.
        elementInNonEmpty <- $c.
        collectionNotIncluded <- notIn asString , notIn asString.
        notIn <- $z


]
Methods StringTest
    testAtPut
        self nonEmpty at: self anIndex put: self aValue.
        self should: [ (self nonEmpty at: self anIndex) = self aValue ] description: 'read value after at:put:'


]
Methods StringTest

    anotherValue
        ^ self nonEmpty at: 1


]
Methods StringTest

    testEqualSignOfIdenticalCollectionObjects
        self should: [self empty = self empty] description: 'self empty = self empty'.
        self should: [self nonEmpty = self nonEmpty] description: 'self nonEmpty = self nonEmpty'


]
Methods StringTest
    testEqualSignIsTrueForEmptyButNonIdenticalCollections
        self should: [self empty = self empty copy] description: 'self empty = self empty copy'.
        self should: [self empty copy = self empty] description: 'self empty copy = self empty'.
        self should: [self empty copy = self empty copy] description: 'self empty copy = self empty copy'


]
Methods StringTest
    testAt
        self should: [ (string at: 1) = $H ] description: 'string at: 1 = $H'


]
Methods StringTest

    testAtPutTwoValues
        self nonEmpty at: self anIndex put: self aValue.
        self nonEmpty at: self anIndex put: self anotherValue.
        self should: [ (self nonEmpty at: self anIndex) = self anotherValue ] description: 'read value after multiple at:put:'


]
Methods StringTest
    empty
        ^emptyString


]
Methods StringTest
    testComparing
        self should: ['foo' < 'foo:'] description: 'foo < foo:'.
        self should: ['foo' < 'fooBar'] description: 'foo < fooBar'.
        self should: ['foo' <= 'foo:'] description: 'foo <= foo:'.
        self should: ['foo' <= 'fooBar'] description: 'foo <= fooBar'.
        self should: ['foo:' > 'foo'] description: 'foo: > foo'.
        self should: ['fooBar' > 'foo'] description: 'fooBar > foo'.
        self should: ['foo:' >= 'foo'] description: 'foo: >= foo'.
        self should: ['fooBar' >= 'foo'] description: 'fooBar >= foo'.


]
Methods StringTest

    testEqualSignIsTrueForNonIdenticalButEqualCollections
        self should: [self nonEmpty = self nonEmpty copy] description: 'self nonEmpty = self nonEmpty copy'.
        self should: [self nonEmpty copy = self nonEmpty] description: 'self nonEmpty copy = self nonEmpty'.
        self should: [self nonEmpty copy = self nonEmpty copy] description: 'self nonEmpty copy = self nonEmpty copy'


]
Methods StringTest

    nonEmpty
        ^nonEmpty5ElementsSorted


]
Methods ButtonControl
  init: aString
    | iup result |
    iup <- IUP instance.
    result <- iup IupButton: aString.
    super handle: (result at: 1).
    ^ self


]
Methods ButtonControl

  setActionCallback: aBlock
    ^ super setCallbackFor: 'ACTION' as: aBlock returning: #int taking: #(cObject)


]
Methods ButtonControl

  init
    ^ self init: 'UNNAMED'


]
Methods TestResult

        error: aString in: aTestCase
            errors add: (self makeRecord: aString in: aTestCase)


]
Methods TestResult

        errors
            ^ errors


]
Methods TestResult

        stop
            stopTime <- <4>


]
Methods TestResult
        makeRecord: aString in: aTestCase
            | record |
            record <- Array new: 2.
            record at: 1 put: aTestCase.
            record at: 2 put: aString.
            ^ record


]
Methods TestResult

        run: aTestResult
            testCases do: [:each | each run: aTestResult]


]
Methods TestResult

        failure: aString in: aTestCase
            failures add: (self makeRecord: aString in: aTestCase)


]
Methods TestResult
        start
            startTime <- <4>


]
Methods TestResult

        failures
            ^ failures


]
Methods TestResult
        setTest: aTest
          testName <- aTest name.
          failures <- List new.
          errors <- List new


]
Methods LabelControl
  init: aString
    | iup result |
    iup <- IUP instance.
    result <- iup IupLabel: aString.
    super handle: (result at: 1).
    ^ self


]
Methods LabelControl

  init
    ^ self init: 'UNNAMED'


]
Methods HBox
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupHbox.
    super handle: (result at: 1).
    ^ self


]
Methods TestCase

        shouldnt: aBlock description: aString
            aBlock value ifTrue: [self failedCheckSignal signalWith: aString]


]
Methods TestCase

        shouldnt: aBlock
            self shouldnt: aBlock or: 'No message supplied'


]
Methods TestCase

        should: aBlock description: aString
            aBlock value ifFalse: [self failedCheckSignal signalWith: aString]


]
Methods TestCase

        performTest
            | args |
            ^ [self respondsTo: selector] ifTrue: 
                [   args <- Array new: 1.
                    args at: 1 put: self.
                    smalltalk perform: selector withArguments: args
                ] ifFalse: [ false ]


]
Methods TestCase

        run: aTestResult
            self setUp.
            [ self performTest ] on: FailedCheckSignal do: [:ex | aTestResult failure: (ex message) in: self].
            self tearDown


]
Methods TestCase
        setUp
            "Run whatever code you need to get ready for the test to run."
            nil


]
Methods TestCase
        run
            self setUp.
            self performTest.
            "Should really do valueNowOrOnUnwindDo: here"
            self tearDown


]
Methods TestCase
            setSelector: aSymbol
            selector <- aSymbol


]
Methods TestCase

        tearDown
            "Release whatever resources you used for the test."
            nil


]
Methods TestCase
 
        selector
            ^ selector


]
Methods TestCase

        should: aBlock
            self should: aBlock or: 'No message supplied'


]
Methods MetaException
    with: aString
    | ex |
    ex <- super new.
    ex message: aString.
    ^ ex


]
Methods MetaTestRunner
    all
        | suite result |
        TestCase subClasses do: [ :t |
            suite <- t suite.
            result <- suite run.
            ('Test: ', (suite name)) printNoReturn.
            (' : ', (suite testCases size), ' tests') printNoReturn.
            (', ', (result errors size), ' errors') printNoReturn.
            (', ', (result failures size), ' failures') print.
            [(result errors size) > 0] ifTrue: [
                (result errors) do: [ :error |
                    error print
                ]
            ].
            [(result failures size) > 0] ifTrue: [
                (result failures) do: [ :failure |
                    failure print
                ]
            ]
        ]


]
Methods Boolean

    or: aBlock
        ^ self ifTrue: [ true ] ifFalse: aBlock


]
Methods Boolean

    ifFalse: falseBlock
        ^ self ifTrue: [] ifFalse: falseBlock


]
Methods Boolean
    ifTrue: trueBlock
        ^ self ifTrue: trueBlock ifFalse: []


]
Methods Boolean

    ifFalse: falseBlock ifTrue: trueBlock
        ^ self ifTrue: trueBlock
            ifFalse: falseBlock


]
Methods Boolean

    and: aBlock
        ^ self ifTrue: aBlock ifFalse: [ false ]


]
Methods Class

  methodsFor: protocol
    ^ methods select: [ :m | m protocol = protocol ]


]
Methods Class

    subClass: aSymbol instanceVariableNames: v classVariableNames: cv
    | newClass newMetaClass metaClassSymbol|
    newMetaClass <- Class new.
    newMetaClass name: ('Meta', aSymbol asString); superClass: self class; variables: cv.
    newClass <- newMetaClass new.
    newClass name: aSymbol; superClass: self; variables: v.
    aSymbol assign: newClass.
    classes at: aSymbol put: newClass.
    metaClassSymbol <- ('Meta', aSymbol asString) asSymbol.
    metaClassSymbol assign: newMetaClass.
    classes at: metaClassSymbol put: newMetaClass.
    ^ newClass 





]
Methods Class

    readMethods
        [ smalltalk inquire: 'Add a method (yn) ? ' ]
            whileTrue: [ self addMethod ]


]
Methods Class

    fileOutMethodsOn: aFile
        " file out all methods "
        methods do: [:m |
            aFile print: 'Methods ', name.
            aFile print: m text.
            aFile print: ']'
            ]


]
Methods Class

    printString
        ^ name asString


]
Methods Class

    editMethod: name    | m |
        m <- self methodNamed: name.
        (m notNil)
            ifTrue: [ self doEdit: m ]
            ifFalse: [ superClass notNil
                    ifTrue: [ superClass editMethod: name ]
                    ifFalse: [ 'no such method' print ] ]


]
Methods Class

    watch: name | m |
        m <- self methodNamed: name.
        (m notNil) 
            ifTrue: [ ^ m watch: 
                [:a | ('executing ', name) print. a print] ]
            ifFalse: [ ^ 'no such method' ]


]
Methods Class

    readInstanceVariables
        self variables:
            ((smalltalk getPrompt: 'Instance Variables? ')
            words: [:x | x isAlphabetic ])


]
Methods Class

  viewSubClassTree: indent
    classes do: [ :obj |
      ((obj isKindOf: Class) and: [ obj superClass == self])
        ifTrue: [
          (1 to: indent) do: [:ignore | ' ' printNoReturn ].
          obj print.
          obj viewSubClassTree: indent + 4 ] ]


]
Methods Class
    new     
    | newObj |
        newObj <- < 22 < 58 instanceSize > self >.
    (newObj isKindOf: Class) ifTrue: [
      newObj initialize ]
    ifFalse: [
      newObj new ].
    ^ newObj


]
Methods Class

    superClass: aClass
        superClass <- aClass


]
Methods Class

    methods
        ^ methods


]
Methods Class

    upSuperclassChain: aBlock
        aBlock value: self.
        (superClass notNil)
            ifTrue: [ superClass upSuperclassChain: aBlock ]


]
Methods Class

  isMeta
    (name asString size < 4) ifTrue: [ ^ false ].
    ^ (name asString from: 1 to: 4) = 'Meta'


]
Methods Class

    display
        ('Class name: ', name asString)  print.
        (superClass notNil)
            ifTrue: [ ('Superclass: ', superClass ) print ].
        'Instance Variables:' print.
        variables isNil
            ifTrue: [ 'no instance variables ' print ]
            ifFalse: [ variables display ].
        'Subclasses: ' print.
        self subClasses display


]
Methods Class

  viewSubClassTree
    self viewSubClassTree: 0


]
Methods Class
fileOutHeaderOn: aFile
    " If meta class has class members, output a Raw meta class "
    self class variables isNil ifFalse: [
        aFile printNoReturn: 
            'RawClass ', self class name , ' ', self class class, ' ', self class superClass.
        self class variables do: [:v | aFile printNoReturn: ' ', v ].
        aFile print: ''
    ].
    " file out class description "
    aFile printNoReturn: 
        'Class ', name , ' ', superClass.
    (variables notNil) ifTrue: [
        variables do: [:v | aFile printNoReturn: ' ', v ]].
    aFile print: ''



]
Methods Class

  subClass: aSymbol
    ^ self subClass: aSymbol instanceVariableNames: (Array new: 0)


]
Methods Class

    doEdit: method
        " edit a method until it compiles correctly "
        [ method text: method text edit.
          (method compileWithClass: self)
            ifTrue: [ ^ true ]
            ifFalse: [ smalltalk inquire: 'edit again (yn) ? ' ]
                ] whileTrue.
        ^ false


]
Methods Class

    instanceSize
        ^ instanceSize


]
Methods Class

    methodNamed: name
        (methods includesKey: name)
            ifTrue: [ ^ methods at: name ].
        (superClass notNil)
            ifTrue: [ ^ superClass methodNamed: name ].
        ^ nil


]
Methods Class

  subClass: aSymbol instanceVariableNames: v
    ^ self subClass: aSymbol instanceVariableNames: v classVariableNames: (Array new: 0)


]
Methods Class
    addMethod   | m |
        m <- Method new; text: ''.
        (self doEdit: m)
            ifTrue: [ methods at: m name put: m ]


]
Methods Class

    viewMethod: methodName  | m |
        m <- self methodNamed: methodName.
        (m notNil) 
            ifTrue: [ m signature print.  m text print ]
            ifFalse: [ 'no such method' print ]


]
Methods Class

    variables
        ^ variables


]
Methods Class
fileOutOn: aFile
    self fileOutHeaderOn: aFile.
    self fileOutMethodsOn: aFile.


]
Methods Class

  protocols
    | protocols |
    protocols <- Set new.
    (methods) do: [ :m | protocols add: (m protocol) ].
    ^ protocols


]
Methods Class

    variables: nameArray
        variables <- nameArray.
        instanceSize <- superClass instanceSize + nameArray size


]
Methods Class

    initialize
        superClass <- Object.
        instanceSize <- 0.
        methods <- Dictionary new


]
Methods Class

    name: aString
        name <- aString


]
Methods Class
    fileOut     | f |
        " file out whole class on class.st "
        f <- File new; name: (name asString,'.st'); mode: 'w'; open.
        self fileOutOn: f.
        f close.


]
Methods Class

    name
        ^ name


]
Methods Class

    respondsTo  | theSet |
        theSet <- Dictionary new.
        self upSuperclassChain: 
            [:x | theSet addAll: x methods ].
        ^ theSet


]
Methods Class

    superClass
        ^ superClass


]
Methods Class

    subClasses
        ^ classes inject: List new
            into: [:x :y | (y superClass == self)
                        ifTrue: [ x add: y]. x ]


]
Methods MetaModule
  require: name
  | newmodule file |
    file <- File new.
    #('modules' '.') do: [ :root |
      ('Trying ', root, '/', name) print.
      file mode: 'r'; name: (root, '/', name, '/', name, '.st').
      (file exists) ifTrue: [
        newmodule <- self load: name from: file.
        newmodule setRoot: (root, '/', name, '/').
        ^ true
      ]
    ].
    ^ false


]
Methods MetaModule

  load: name from: file
  | newmodule |
    file fileIn.
    "TODO: Need to get succes/fail from the fileIn"
    file close.
    newmodule <- Module new.
    newmodule name: name.
    modules isNil ifTrue: [
      modules <- Dictionary new
    ].
    modules at: name put: newmodule.
    ^ newmodule


]
Methods MetaModule
  modules
    ^ modules


]
Methods MetaFLTK
fltkFFI
  (fltkFFI isNil) ifTrue: [
    fltkFFI <- FFI new: 'FLTK'
  ].
  ^ fltkFFI

]
Methods FFI
  init: aString
    self open: aString.
    functions <- Dictionary new.


]
Methods FFI

  openPrim
    ^ <180 name>


]
Methods FFI

  open
    handle <- self openPrim


]
Methods FFI

  add: aString
    | fhandle |
    fhandle <- <181 handle aString>.
    fhandle isNil ifFalse: [ function at: aString put: fhandle ].
    ^ self


]
Methods FFI

  handle
    ^ handle


]
Methods FFI

  call: aString returning: aSymbol taking: argTypes as: arguments
    | fid ret |
    ^ (functions includesKey: aString)
        ifTrue: [ fid <- functions at: aString ]
        ifFalse: [ 
          fid <- <181 handle aString>. 
          fid isNil
            ifFalse: 
              [ <182 fid aSymbol argTypes arguments> ]
            ifTrue: 
              [ nil ]
        ]


]
Methods FFI

  name: aString
    name <- aString


]
Methods FFI

  open: theName
    name <- theName.
    ^ self open


]
Methods FFI

  name
    ^ name


]
Methods FFI

  close
    | result |
    result <- <184 handle>.
    handle <- nil.
    ^ result


]
Methods MetaWorkspace
new
  | newWorkspace |
  newWorkspace <- super new.
  newWorkspace init.
  ^ newWorkspace


]
Methods MetaWorkspace

open
  Workspace new; show.


]
Methods Dialog
  init: child
    | iup result |
    iup <- IUP instance.
    result <- iup IupDialog: (child handle).
    super handle: (result at: 1).
    ^ self


]
Methods Dialog

  show
    IUP instance IupShow: handle.


]
Methods Dialog

  hide
    IUP instance IupHide: handle.


]
Methods Set
    add: value
        (self includes: value)
            ifFalse: [ self addFirst: value ]


]
Methods Switch

    else: block
        notdone ifTrue: [ notdone <- false. block value ]


]
Methods Switch
    key: value
        const <- value.
        notdone <- true.


]
Methods Switch

    ifMatch: key do: block
        (notdone and: [ const = key ])
            ifTrue: [ notdone <- false. block value ]


]
Methods MetaTime

  clockAtStartup
    ^ ClockAtStartup


]
Methods MetaTime

  millisecondsToRun: timedBlock
    | startTime |
    startTime <- self millisecondClock.
    timedBlock value.
    ^self millisecondClock - startTime


]
Methods MetaTime
  millisecondClock
    ^ <7>


]
Methods MetaTime

  update: aspect
    (aspect == #returnFromSnapshot) ifTrue: [
      ClockAtStartup <- <7>
    ]


]
Methods MetaTime
delay: aNumber
  | startTime currentTime |
  startTime <- Time millisecondClock.
  currentTime <- startTime.
  [ (currentTime - startTime) < aNumber ] whileTrue: [
    currentTime <- Time millisecondClock.
    scheduler yield
  ]










]
Methods MetaMorph
  new: aString
    | newObj |
    newObj <- super new: aString.
    ^ newObj


]
Methods ByteArray

    basicAt: index put: value
        ^ ((value isMemberOf: Integer) and: [value between: 0 and: 255])
            ifTrue: [ <32 self index value > ]
            ifFalse: [ value print. smalltalk error: 
                ('assign illegal value to ByteArray: ', (value class))]


]
Methods ByteArray
    asString
        <22 self String>


]
Methods ByteArray

    size: value
        ^ <22 <59 value> ByteArray>


]
Methods ByteArray

    basicAt: index
        ^ <26 self index>


]
Methods MetaFFI

  test
    | ffi args result etype cb |
    etype <- ExternalData new.
    etype members at: 1 put: 'Hello from FFI test'; at: 2 put: 69.
    args <- Array new: 4.
    cb <- <183 #int #(string int) [ :str :num | ('In callback: ', str, ' ', num) print. 555 ] >.
    args at: 1 put: 'Hello from Smalltalk'; at: 2 put: 0; at: 3 put: etype; at: 4 put: cb.
    ffi <- FFI new: 'simple1'.
    result <- ffi call: 'my_print' returning: #int taking: #(string intOut externalOut cObject) as: args.
    (result at: 3) print.
    (result at: 4) members print


]
Methods MetaFFI

  libraries
    ^ libraries


]
Methods MetaFFI
  new: aString
    | newObj |
    newObj <- super new.
    newObj init: aString.
    libraries isNil ifTrue: [
      libraries <- Dictionary new
    ].
    libraries at: aString put: newObj.
    ^newObj


]
Methods MetaFFI

  returnFromSnapshot
    libraries isNil ifFalse: [
    libraries binaryDo: [ :x :ffi |
      ffi open ]
    ]      


]
Methods MetaFFI

  prepareForSnapshot
    libraries isNil ifFalse: [
    libraries binaryDo: [ :x :ffi |
      ffi close ]
    ]


]
Methods MetaFFI
  doInit
    libraries <- Dictionary new


]
Methods Float
    + value
        ^ value isFloat
            ifTrue: [ <110 self value> " floating add " ]
            ifFalse: [ super + value ]


]
Methods Float

    rounded
        ^ (self + 0.5) floor


]
Methods Float

    printString
        ^ <101 self>


]
Methods Float

    truncated   | result f i |
        " truncate to an integer rounded towards zero"
        f <- self. result <- 0.
        [ i <- f integerPart. i > 0] whileTrue:
            [ result <- result + i. f <- f - i ].
        ^ result


]
Methods Float

    new
        ^ smalltalk error: 'cannot create floats with new'


]
Methods Float

    isFloat
        ^ true


]
Methods Float

    coerce: value
        " convert the value into a floating point number "
        ^ value asFloat


]
Methods Float

    / value 
        ^ value isFloat
            ifTrue: [ (value = 0.0)
                    ifTrue: [ smalltalk error:
                        'float division by zero' ]
                    ifFalse: [ <119 self value> ]]
            ifFalse: [ super / value ]


]
Methods Float

    ln
        " natural log of self "
        ^ <102 self>


]
Methods Float

    integerPart | i j |
        i <- <106 self>. j <- i basicAt: 2. i <- i basicAt: 1.
        j < 0 ifTrue: [ ^ 0 ] ifFalse: [ ^ i * (2 raisedTo: j)]


]
Methods Float

    quo: value
        ^ (self / value) truncated


]
Methods Float

    < value
        ^ value isFloat
            ifTrue: [ <112 self value> " floating comparison " ]
            ifFalse: [ super < value ]


]
Methods Float

    generality
        " our numerical generality - used for mixed mode arithmetic"
        ^ 7


]
Methods Float

    * value
        ^ value isFloat
            ifTrue: [ <118 self value> ]
            ifFalse: [ super * value ]


]
Methods Float

    exp
        " return e raised to self "
        ^ <103 self>


]
Methods Float

    - value
        ^ value isFloat
            ifTrue: [ <111 self value> " floating subtract " ]
            ifFalse: [ super - value ]


]
Methods Float

    = value
        ^ value isFloat
            ifTrue: [ <116 self value> ]
            ifFalse: [ super = value ]


]
Methods Fraction

    + f
        f isFraction
            ifTrue: [ ^ ((top * f bottom) + (bottom * f top)) /
                    (bottom * f bottom) ]
            ifFalse:[ ^ super + f ]


]
Methods Fraction

    isFraction
        ^ true


]
Methods Fraction

    raisedTo: x
        ^ (top raisedTo: x) / (bottom raisedTo: x)


]
Methods Fraction

    printString
        ^ top printString, '/', bottom printString


]
Methods Fraction

    truncated
        " convert to an integer rounded towards zero "
        ^ top quo: bottom


]
Methods Fraction

    reciprocal
        ^ bottom / top


]
Methods Fraction

    abs
        ^ top abs / bottom


]
Methods Fraction

    coerce: x
        " coerce a value into being a fraction "

        ^ x asFraction


]
Methods Fraction

    / f
        ^ self * f reciprocal


]
Methods Fraction

    ln
        ^ (top ln) - (bottom ln)


]
Methods Fraction

    asFloat
        " convert to a floating point number "

        ^ top asFloat / bottom asFloat


]
Methods Fraction

    bottom
        ^ bottom


]
Methods Fraction

    with: t over: b
        " initialization "

        top <- t.
        bottom <- b


]
Methods Fraction

    < f
        f isFraction
            ifTrue: [ ^ (top * f bottom) < (bottom * f top) ]
            ifFalse:[ ^ super < f ]


]
Methods Fraction

    generality
        " generality value - used in mixed type arithmetic "
        ^ 5


]
Methods Fraction

    * f
        f isFraction
            ifTrue: [ ^ (top * f top) / (bottom * f bottom) ]
            ifFalse: [ ^ super * f ]


]
Methods Fraction

    top
        ^ top


]
Methods Fraction

    - f
        f isFraction
            ifTrue: [ ^ ((top * f bottom) - (bottom * f top)) /
                    (bottom * f bottom) ]
            ifFalse:[ ^ super - f ]


]
Methods Fraction
    = f
        f isFraction
            ifTrue: [ ^ (top = f top) and: [ bottom = f bottom ] ]
            ifFalse: [ ^ super = f ]


]
Methods MetaCPointer
  new
    ^ <6>


]
Methods MetaByteArray
  new: size
        ^ < 22 < 59 size > self >


]
Methods MetaPanel
  new: browser
    | newPanel |
    newPanel <- super new.
    newPanel init: browser.
    ^ newPanel


]
Methods DefinitionPanel

  buttons
    | hbox |
    hbox <- HBox new.
    #(subclass) do: [ :name |
      hbox addChild: ( ButtonControl new: (name asString); at: 'EXPAND' put: 'HORIZONTAL'; setActionCallback: [ :x | self processCommand: 837 ])
    ].
    ^ hbox


]
Methods DefinitionPanel
  init: browser
    | hbox vbox|
    super init: browser.
    widget <- VBox new; at: 'NMARGIN' put: '5x5'; at: 'GAP' put: '5'.
    text <- TextControl new; at: 'EXPAND' put: 'YES'; at: 'MULTILINE' put: 'YES'; at: 'TABSIZE' put: '4'.
    commands <- Dictionary new.
    text setCallbackFor: 'K_ANY' as: [ :h :c | (self processCommand: c) ] returning: #int taking: #(cObject int).
    widget addChild: self buttons.
    vbox <- VBox new; at: 'EXPAND' put: 'VERTICAL'; at: 'NGAP' put: '5'.
    hbox <- HBox new; at: 'NGAP' put: '5'.
    vbox addChild: self acceptButton.
    vbox addChild: self cancelButton.
    hbox addChild: text.
    hbox addChild: vbox.
    widget addChild: hbox.
    ^ self


]
Methods DefinitionPanel

  showMethod: method on: class
    self clearMethod.
    commands at: 833 put: [ 
      self saveMethod: method on: class 
    ].
    commands at: 835 put: [
      self cancelMethod: method on: class
    ].
    text at: 'SELECTION' put: 'ALL'.
    ((text at: 'SELECTION') isEmpty)
      ifTrue: [
        text insert: (method text)
      ]
      ifFalse: [
        text at: 'SELECTEDTEXT' put: (method text)
      ]


]
Methods DefinitionPanel

  clearMethod
    (commands includesKey: 833) ifTrue: [
      commands removeKey: 833
    ].
    (commands includesKey: 835) ifTrue: [
      commands removeKey: 835
    ].
    text at: 'SELECTION' put: 'ALL'.
    ((text at: 'SELECTION') isEmpty)
      ifFalse: [
        text at: 'SELECTEDTEXT' put: '' 
      ]


]
Methods DefinitionPanel

  cancelButton
    | button |
    button <- ButtonControl new: 'Cancel'; at: 'EXPAND' put: 'HORIZONTAL'.
    button setActionCallback: [ :x | self processCommand: 835 ].
    ^ button


]
Methods DefinitionPanel

  saveMethod: method on: class
    | oldMethod |
    oldMethod <- method text.
    method text: (text at: 'VALUE').
    (method compileWithClass: class)
    ifTrue: [ 
      ^ true 
    ]
    ifFalse: [ 
      method text: oldMethod.
      IUP instance IupMessage: (<152>) title: 'Error'
    ].
    ^ false


]
Methods DefinitionPanel

  subclassOn: class
    | dialog |
    dialog <- AddSubClassDialog new: class.
    dialog show. 


]
Methods DefinitionPanel

  processCommand: c
    ^ (commands includesKey: c) 
      ifTrue: [
        (commands at: c) value.
        -1
      ] 
      ifFalse: [ 
        ('Missing command ', c) print.
        -2
      ]


]
Methods DefinitionPanel

  cancelMethod: method on: class
    self showMethod: method on: class.
    ^ true


]
Methods DefinitionPanel

  acceptButton
    | button |
    button <- ButtonControl new: 'Accept'; at: 'EXPAND' put: 'HORIZONTAL'.
    button setActionCallback: [ :x | self processCommand: 833 ]. 
    ^ button


]
Methods DefinitionPanel

  showClass: class
    commands at: 837 put: [ 
      self subclassOn: class 
    ].


]
Methods ExternalData

  member: index
    ^ members at: index


]
Methods ExternalData

  setMember: index to: anObject
    members at: index put: anObject


]
Methods ExternalData

  members
    ^ members


]
Methods ExternalData
  init
    members <- Array new: (self fields size)


]
Methods ExternalData

  fields
    ^ #((string string)
        (number int))


]
Methods MetaButtonControl
  new: aString
    | newButton |
    newButton <- super new.
    newButton init: aString.
    ^ newButton


]
Methods MetaTestResult
        test: aTest
            ^ self new setTest: aTest


]
Methods MetaLabelControl
  new: aString
    | newLabel |
    newLabel <- super new.
    newLabel init: aString.
    ^ newLabel


]
Methods SetTest
    testEliminateDuplicates
        " test the collection classes a little"
        self should: [#(1 2 3 3 2 4 2) asSet = #(1 2 3 4) asSet] description: '#(1 2 3 3 2 4 2) asSet = #(1 2 3 4) asSet'


]
Methods Smalltalk
    error: aString
        " print a message, and remove current process "
        stderr print: aString.
        scheduler currentProcess; trace; terminate.


]
Methods Smalltalk

    watch
        ^ <5>


]
Methods Smalltalk
    saveImage
        self saveImage: (self getPrompt: 'type image name: ').
        ^ 'done'


]
Methods Smalltalk

    saveImage: name 
        scheduler critical: [
            " first get rid of our own process "
            scheduler removeProcess: scheduler currentProcess.
        File new;
            name: name;
            open: 'wb';
            saveImage;
            close ]


]
Methods Smalltalk

    perform: message withArguments: args
        ^ self perform: message withArguments: args
            ifError: [ self error: 'cant perform' ]


]
Methods Smalltalk
    getPrompt: aString
        | command |
        ^ <151 aString>


]
Methods Smalltalk
fileOut: aString
  | theFile processed block |
  theFile <- File new.
  processed <- Set new.
  theFile mode: 'w'; name: 'bootstrap.st'; open.
  classes binaryDo: [ :name :class | 
    class isMeta ifFalse: [
      self fileOutClassDecl: class On: theFile Track: processed
    ]
  ].
  classes binaryDo: [ :name :class | 
    class fileOutMethodsOn: theFile
  ].
  processed print.
  theFile close.



]
Methods Smalltalk
fileOutClassDecl: aClass On: aFile Track: processed
  (aClass superClass) isNil ifFalse: [
    self fileOutClassDecl: (aClass superClass) On: aFile Track: processed
  ].
  ( processed includes: aClass ) ifFalse: [
    aClass fileOutHeaderOn: aFile.
    processed add: aClass
  ]





]
Methods Smalltalk

    echo
        " enable - disable echo input "
        echoInput <- echoInput not


]
Methods Smalltalk
    perform: message withArguments: args ifError: aBlock    
            | receiver method |
        receiver <- args at: 1 ifAbsent: [ ^ aBlock value ].
        method <- receiver class methodNamed: message.
        ^ method notNil 
            ifTrue: [ method executeWith: args ]
            ifFalse: aBlock


]
Methods Smalltalk

    inquire: aString    | response |
        response <- self getPrompt: aString.
        response isNil
            ifTrue: [ ^ false ].
        ^ 'Yy' includes: (response at: 1 ifAbsent: [])


]
Methods ExceptionHandler

    handledException: anException
        handledException <- anException


]
Methods ExceptionHandler

    handlerBlock: aBlock
        handlerBlock <- aBlock


]
Methods ExceptionHandler

    handlerBlock
        ^ handlerBlock


]
Methods ExceptionHandler
    handledException
        ^ handledException


]
Methods ExceptionHandler

    handles: anException
        ^ anException isKindOf: handledException


]
Methods AddSubClassDialog

  nameControls
    | label editor container |
    container <- HBox new.
    label <- LabelControl new: 'Name'.
    editor <- TextControl new.
    nameControl <- editor.
    container addChild: label; addChild: editor.
    editor at: 'EXPAND' put: 'HORIZONTAL'.
    container at: 'EXPAND' put: 'HORIZONTAL'; at: 'MARGIN' put: '5x5'; at: 'GAP' put: '5'.
    ^ container 


]
Methods AddSubClassDialog

  buttons
    | container ok cancel vnames scname |
    container <- HBox new.
    ok <- ButtonControl new: 'Ok'.
    cancel <- ButtonControl new: 'Cancel'.
    container addChild: (Fill new); addChild: ok; addChild: cancel.
    container at: 'EXPAND' put: 'HORIZONTAL'; at: 'MARGIN' put: '5x5'; at: 'GAP' put: '5'.
    ok setActionCallback: [ :h | 
      'Hello' print.
      vnames <- (ivnamesControl at: 'VALUE').
      scname <- (nameControl at: 'VALUE').
      vnames <- (vnames words: [ :x | (x == $ ) not ]).
      vnames <- (vnames collect: [ :x | x asSymbol ]).
      parentClass subClass: (scname asSymbol) instanceVariableNames: vnames
    ].
    ^ container


]
Methods AddSubClassDialog
  init: parent
    | container |
    parentClass <- parent.
    container <- VBox new.
    container addChild: self nameControls.
    container addChild: self instanceVariableControls.
    container addChild: (Fill new).
    container addChild: self buttons.
    container at: 'MARGIN' put: '5x25'.
    dialog <- Dialog new: container; at: 'SHRINK' put: 'NO'; 
                                     at: 'TITLE' put: ('Add Subclass of ', (parent name));
                                     at: 'MINSIZE' put: '400x200';
                                     at: 'MAXSIZE' put: '800x200'.
    ^ self


]
Methods AddSubClassDialog

  show
    dialog show.


]
Methods AddSubClassDialog

  instanceVariableControls
    | label editor container |
    container <- HBox new.
    label <- LabelControl new: 'Instance Variables'.
    editor <- TextControl new.
    ivnamesControl <- editor.
    container addChild: label; addChild: editor.
    editor at: 'EXPAND' put: 'HORIZONTAL'.
    container at: 'EXPAND' put: 'HORIZONTAL'; at: 'MARGIN' put: '5x5'; at: 'GAP' put: '5'.
    ^ container 


]
Methods MetaString
  new: size
    ^ super new: size+1


]
Methods Fill
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupFill.
    super handle: (result at: 1).
    ^ self


]
Methods ItemControl
  init: aString
    | iup result |
    iup <- IUP instance.
    result <- iup IupItem: aString.
    super handle: (result at: 1).
    ^ self


]
Methods ItemControl

  setActionCallback: aBlock
    ^ super setCallbackFor: 'ACTION' as: aBlock returning: #int taking: #(cObject)


]
Methods ItemControl

  init
    ^ self init: 'UNNAMED'


]
Methods MetaHTTPRequest
    new: request
    | newobj |
    newobj <- super new.
    newobj init: request.
    ^ newobj


]
Methods UndefinedObject
    isNil
        ^ true


]
Methods UndefinedObject

    printString
        ^ 'nil'


]
Methods UndefinedObject

    initialize
        " initialize the initial object image "
        self createGlobals.
        classes binaryDo: [:x :y |
            (y isKindOf: Class) ifTrue: [
                (y respondsTo: #doInit) ifTrue: [
                    y doInit
                ]
            ]
        ].
        File new;
            name: 'systemImage';
            open: 'wb';
            saveImage;
            close.


]
Methods UndefinedObject

    notNil
        ^ false


]
Methods UndefinedObject
    createGlobals   | aBlock |
        " create global variables in initial image "
        true <- True new.
        false <- False new.
        smalltalk <- Smalltalk new.
        files <- Array new: 15.
        stdin <- File new; name: 'stdin'; mode: 'r'; open.
        stdout <- File new; name: 'stdout'; mode: 'w'; open.
        stderr <- File new; name: 'stderr'; mode: 'w'; open.
        editor <- 'vim'.
        " create a dictionary of classes "
        classes <- Dictionary new.
        symbols binaryDo: [:x :y | 
            (y isKindOf: Class) ifTrue: [ 
                    classes at: x put: y
            ] 
        ].
        scheduler <- Scheduler new.
        " create the initial system process "
        " note the delayed recursive call "
        aBlock <- [ files do: [:f | f notNil ifTrue: [ f open ]].
                   systemProcess <- aBlock newProcess.
                   echoInput <- false.
                   scheduler run ].
        systemProcess <- aBlock newProcess.


]
Methods Collection

    asArray     | newArray i |
        newArray <- Array new: self size.
        i <- 0.
        self do: [:x | i <- i + 1. newArray at: i put: x].
        ^ newArray


]
Methods Collection

    printString
        ^ ( self inject: self class printString , ' ('
             into: [:x :y | x , ' ' , y printString]), ' )'


]
Methods Collection

    asByteArray | newArray i |
        newArray <- ByteArray new: self size.
        i <- 0.
        self do: [:x | i <- i + 1. newArray at: i put: x].
        ^ newArray


]
Methods Collection

    display
        self do: [:x | x print ]


]
Methods Collection

    occurrencesOf: anObject
        ^ self inject: 0
               into: [:x :y | (y = anObject) 
                     ifTrue: [x + 1]
                     ifFalse: [x] ]


]
Methods Collection

    asSet
        ^ Set new addAll: self


]
Methods Collection

    sort
        ^ self sort: [:x :y | x < y ]


]
Methods Collection

    asString
        ^ self asByteArray asString


]
Methods Collection
    < coll
        (coll respondsTo: #includes:)
            ifFalse: [ ^ smalltalk error:
                  'collection compared to non collection'].
        self do: [:x | ((self occurrencesOf: x) < 
            (coll occurrencesOf: x))ifFalse: [ ^ false ]].
        coll do: [:x | (self includes: x) ifFalse: [ ^ true ]].
        ^ false


]
Methods Collection

    isEmpty 
        ^ self size = 0


]
Methods Collection

    includes: value
        self do: [:x | (x = value) ifTrue: [ ^ true ] ].
        ^ false


]
Methods Collection

    inject: thisValue into: binaryBlock     | last |
        last <- thisValue.
        self do: [:x | last <- binaryBlock value: last value: x].
        ^ last


]
Methods Collection

    sort: aBlock
        ^ self inject: List new
            into: [:x :y | x add: y ordered: aBlock. x]


]
Methods Collection

    = coll
        self do: [:x | (self occurrencesOf: x) = 
                (coll occurrencesOf: x) ifFalse: [ ^ false ] ].
        ^ true


]
Methods Collection

    size
        ^ self inject: 0 into: [:x :y | x + 1]


]
Methods MetaObject
    dependencies
        ^ Dependencies


]
Methods MetaObject

    dependencies: anObject
        Dependencies <- anObject


]
Methods MetaObject

    doInit
        Dependencies <- Dictionary new


]
Methods TestSuite

        defaultTestResultClass
            ^ TestResult


]
Methods TestSuite
        setName: aString
            name <- aString.
            testCases <- List new 


]
Methods TestSuite

        run: aTestResult
            testCases do: [:each | each run: aTestResult]


]
Methods TestSuite
        run
            | result |
            result <- self defaultTestResult.
            result start.
            [ self run: result ] on: Exception do: [ :ex | result error: 'Error' in: self ].
            result stop.
            ^ result


]
Methods TestSuite

        addTestCase: aTestCase
            testCases add: aTestCase


]
Methods TestSuite

    testCases
        ^ testCases


]
Methods TestSuite

        addTestCases: aCollection
            aCollection do: [:each | self addTestCase: each]


]
Methods TestSuite

        defaultTestResult
            ^ self defaultTestResultClass test: self


]
Methods TestSuite
    name
        ^ name


]
Methods IndexedCollection

    at: aKey
        ^ self at: aKey 
            ifAbsent: [ smalltalk error: ('index to at: illegal ', (aKey printString), '(', (self size), ')') ]


]
Methods IndexedCollection

    asArray 
        ^ Array new: self size ; addAll: self


]
Methods IndexedCollection

    binaryInject: thisValue into: aBlock     | last |
        last <- thisValue.
        self binaryDo: [:i :x | last <- aBlock value: last 
                        value: i value: x].
        ^ last


]
Methods IndexedCollection

    keys
        ^ self binaryInject: Set new 
            into: [:s :i :x | s add: i ]


]
Methods IndexedCollection

    indexOf: aBlock
        ^ self indexOf: aBlock
            ifAbsent: [ smalltalk error: 'index not found']


]
Methods IndexedCollection

    select: aBlock
        ^ self binaryInject: Dictionary new
            into: [:s :i :x | (aBlock value: x)
                    ifTrue: [ s at: i put: x ]. s ]


]
Methods IndexedCollection

    at: index ifAbsent: exceptionBlock
         ^ (self includesKey: index)
            ifTrue: [ self basicAt: index ]
            ifFalse: exceptionBlock


]
Methods IndexedCollection

    do: aBlock
        self binaryDo: [:i :x | aBlock value: x ]


]
Methods IndexedCollection

    atAllPut: anObject
        | to size |
        "Unroll completely for small collections..."
        (size <- self size) = 0 ifTrue: [ ^self ].
        self at: 1 put: anObject. size = 1 ifTrue: [ ^self ].
        self at: 2 put: anObject. size = 2 ifTrue: [ ^self ].
        self at: 3 put: anObject. size = 3 ifTrue: [ ^self ].
        self at: 4 put: anObject. to <- 4.
        "...and use memcpy repeatedly for larger ones."
        [ size > to ] whileTrue: [
            self
                replaceFrom: to + 1
                to: ((to <- to + to) min: size)
                with: self
                startingAt: 1 ].


]
Methods IndexedCollection

    asDictionary
        ^ Dictionary new ; addAll: self


]
Methods IndexedCollection
    addAll: aCollection
        aCollection binaryDo: [:i :x | self at: i put: x ]


]
Methods IndexedCollection

    replaceFrom: start to: stop with: replacementCollection startingAt: repStart 
        | delta maxStop minStop |
        "Replace the items from start to stop with replacementCollection's items
        from repStart to repStart+stop-start"
        minStop <- start - 1.
        maxStop <- self size min: minStop + replacementCollection size.
        (minStop <= stop and: [stop <= maxStop]) 
         ifFalse: 
             [^SystemExceptions.ArgumentOutOfRange 
                 signalOn: stop
                 mustBeBetween: minStop
                 and: maxStop].
        delta <- start - repStart.
        repStart > start
         ifTrue: [
             (start to: stop) do: [:i |
                 self at: i put: (replacementCollection at: i - delta)] ]
         ifFalse: [
             (stop to: start by: -1) do: [:i |
                 self at: i put: (replacementCollection at: i - delta)] ]


]
Methods IndexedCollection

    collect: aBlock
        ^ self binaryInject: Dictionary new
            into: [:s :i :x | s at: i put: (aBlock value: x).  s]


]
Methods IndexedCollection

    indexOf: aBlock ifAbsent: exceptionBlock
        self binaryDo: [:i :x | (aBlock value: x)
                ifTrue: [ ^ i ] ].
        ^ exceptionBlock value


]
Methods IndexedCollection

    values
        ^ self binaryInject: List new
            into: [:s :i :x | s add: x ]


]
Methods TestConversions
    testConversions
        " test a few conversion routines "
        self should: [#abc == #abc asString asSymbol] description: '#abc == #abc asString asSymbol'.
        self should: [$A == $A asInteger asCharacter] description: '$A == $A asInteger asCharacter'.
        self should: [12 = 12 asDigit digitValue] description: '12 = 12 asDigit digitValue'.
        self should: [237 = 237 asString asInteger] description: '237 = 237 asString asInteger'.
        self should: [43 = 43 asFloat truncated] description: '43 = 43 asFloat truncated'.
        self should: [$A == ($A asString at: 1)] description: '$A == ($A asString at: 1)'


]
Methods Char

    isBlank
        ^ value = $   " blank char "


]
Methods Char

    uppercase
        self isLowercase ifTrue: [ ^ Char new: value - 32 ]


]
Methods Char

    lowercase
        self isUppercase ifTrue: [ ^ Char new: value + 32 ]


]
Methods Char

    printString
        ^ '$', self asString


]
Methods Char

    isAlphabetic
        ^ (self isLowercase) or: [ self isUppercase ]


]
Methods Char

    isChar
        ^ true


]
Methods Char

    isDigit
        ^ value between: $0 asInteger and: $9 asInteger


]
Methods Char

    isUppercase
        ^ value between: $A asInteger and: $Z asInteger


]
Methods Char

    asString
        " make ourselves into a string "
        ^ ' ' copy; at: 1 put: self


]
Methods Char
    < aValue
        " can only compare characters to characters "
        ^ aValue isChar
            ifTrue: [ value < aValue asInteger ]
            ifFalse: [ smalltalk error: 'char compared to nonchar']


]
Methods Char

    == aValue
        ^ aValue isChar
            ifTrue: [ value = aValue asInteger ]
            ifFalse: [ false ]


]
Methods Char

    digitValue
        " return an integer representing our value "
        self isDigit ifTrue: [ ^ value - $0 asInteger ].
        self isUppercase ifTrue: [ ^ value - $A asInteger + 10 ].
        ^ smalltalk error: 'illegal conversion, char to digit'


]
Methods Char

    asInteger
        ^ value


]
Methods Char

    value: aValue       " private - used for initialization "
        value <- aValue


]
Methods Char

    isLowercase
        ^ value between: $a asInteger and: $z asInteger


]
Methods Char

    isAlphaNumeric
        ^ (self isAlphabetic) or: [ self isDigit ]


]
Methods VBox
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupVbox.
    super handle: (result at: 1).
    ^ self


]
Methods MetaTestCase

        failedCheckSignal
          ^ failedCheckSignal


]
Methods MetaTestCase

        suite
            | sel suite |
            suite <- TestSuite named: (self asString).
            self methods binaryDo: [ :selector :m |
              sel <- selector asString copyFrom: 1 to: 4.
              sel = 'test' ifTrue: [
                suite addTestCase: (self selector: selector)
              ]
            ].
            ^ suite


]
Methods MetaTestCase
        selector: aSymbol
        ^ self new setSelector: aSymbol


]
Methods MetaTestCase

        doInit
          failedCheckSignal <- FailedCheckSignal new


]
Methods ArrayTest
    testSort
        self should: [#(1 5 3 2 4) sort asArray = #(1 2 3 4 5)] description: '#(1 5 3 2 4) sort asArray = #(1 2 3 4 5)'


]
Methods List

    reject: aBlock          
        ^ self select: [:x | (aBlock value: x) not ]


]
Methods List

    addFirst: aValue
        links <- Link new; value: aValue; link: links


]
Methods List

    remove: value
        (links notNil)
            ifTrue: [ links <- links removeValue: value ]


]
Methods List
    add: aValue
        ^ self addLast: aValue


]
Methods List

    select: aBlock          
        ^ self inject: self class new
               into: [:x :y | (aBlock value: y) 
                    ifTrue: [x add: y]. x]


]
Methods List

    do: aBlock
        (links notNil)
            ifTrue: [ links binaryDo: [:x :y | aBlock value: y]]


]
Methods List

    removeFirst
        self remove: self first


]
Methods List

    links
        ^ links  "used to walk two lists in parallel "


]
Methods List

    addAll: aValue
        aValue do: [:x | self add: x ]


]
Methods List

    collect: aBlock
        ^ self inject: self class new
               into: [:x :y | x add: (aBlock value: y). x ]


]
Methods List

    first
        ^ (links notNil)
            ifTrue: links
            ifFalse: [ smalltalk error: 'first on empty list']


]
Methods List

    addLast: aValue
        (links isNil)
            ifTrue: [ self addFirst: aValue ]
            ifFalse: [ links add: aValue whenFalse: [ :x :y | true ] ]


]
Methods List

    add: aValue ordered: aBlock
        (links isNil)
            ifTrue: [ self addFirst: aValue]
            ifFalse: [ links <- links add: aValue 
                    whenFalse: aBlock ]


]
Methods List

    reverseDo: aBlock
        (links notNil)
            ifTrue: [ links reverseDo: aBlock ]


]
Methods List

    size
        (links isNil)
            ifTrue: [ ^ 0 ]
            ifFalse: [ ^ links size ]


]
Methods Process

    context
        ^ stack at: 3


]
Methods Process
    new
        stack <- Array new: 50.
        stackTop <- 10.
        linkPointer <- 2.
        stack at: 4 put: 1. "return point"
        stack at: 6 put: 1. "bytecode counter"


]
Methods Process

    startAt: x
        stack at: 6 put: x. "starting bytecode value"


]
Methods Process

    trace       | link m h r s |
        " first yield scheduler, forceing store of linkPointer"
        scheduler yield.
        link <- linkPointer.
        link <- stack at: link+1.
        " then trace back chain "
        [ link notNil ] whileTrue:
            [ 
              m <- stack at: link+3. 
              m notNil 
                ifTrue: [ 
                      h <- stack at: link+5.
                      r <- stack at: link+2.
                      s <- '{', link, ' ', r, '}', m signature, ' ('.
                      (r to: link-1) do: 
                        [:x | s <- s, ' ', 
                            (stack at: x) class asString].
                      (s, ') [', h, ' ', link, ']') printNoReturn.
                      h notNil
                        ifTrue: [
                            s <- ' exceptions('.
                            s <- s, h handledException class asString.
                            (s, ')') print
                        ] ifFalse: [
                            ' no exceptions' print
                        ]
                ].
              link <- stack at: link ]


]
Methods Process

    method: x 
        stack at: 5 put: x.


]
Methods Process

    resume
        " resume current process "
        scheduler addProcess: self.


]
Methods Process

    terminate
        " kill current process "
        scheduler removeProcess: self. scheduler yield.




]
Methods Process

    context: ctx
        stack at: 3 put: ctx.


]
Methods Process

    recordHandler: aBlock for: anException 
        | link handler |
        " first yield scheduler, forceing store of linkPointer"
        scheduler yield.
        link <- linkPointer.
        link <- stack at: link+1.
        "No point recording the handler in our current frame, as it'll disappear as
         soon as we're done"
        link <- stack at: link.
        handler <- ExceptionHandler for: anException with: aBlock.
        stack at: link+5 put: handler


]
Methods Process
    execute 
        " execute for time slice, terminating if all over "
        (stack size > 1500)
            ifTrue: [ smalltalk error:
                'process stack overflow, probable loop'].
        <19 self> ifTrue: [] ifFalse: [ self terminate ].


]
Methods Process

    exceptionHandlerFor: anException 
        | link handler returnpoint result |
        " first yield scheduler, forceing store of linkPointer"
        scheduler yield.
        link <- linkPointer.
        link <- stack at: link+1.
        returnpoint <- nil.
        " then trace back chain "
        [ link notNil ] whileTrue:
            [ handler <- stack at: link+5.
              handler notNil
                ifTrue: [
                    (handler handles: anException)
                        ifTrue: [
                            result <- Array new: 2.
                            result at: 1 put: handler; at: 2 put: returnpoint.
                            ^ result
                        ]
                ].
              returnpoint <- stack at: link + 2.
              link <- stack at: link
            ].
        ^ nil


]
Methods Symbol

    printString
        ^ '#' , self asString


]
Methods Symbol

    assign: value
        <27 self value>. ^ value


]
Methods Symbol

        apply: args ifError: aBlock
        ^ smalltalk perform: self withArguments: args ifError: aBlock


]
Methods Symbol
asSymbol
  ^ self


]
Methods Symbol

    value
        ^ <87 self>


]
Methods Symbol

    copy
        ^ self


]
Methods Symbol
        apply: args
        ^ self apply: args ifError: [ 'does not apply' ]


]
Methods Symbol

    asString
        " catenation makes string and copy automatically "
        ^ <24 self ''>


]
Methods Symbol

    respondsTo
        ^ classes inject: Set new
            into: [:x :y | ((y methodNamed: self) notNil)
                        ifTrue: [ x add: y]. x]


]
Methods String

    uppercase
        | ret |
        ret <- ByteArray new: (self size).
        (1 to: self size) do: [ :x | ret at: x put: (self at: x) uppercase asInteger ].
        ^ ret asString


]
Methods String

    lowercase
        | ret |
        ret <- ByteArray new: (self size).
        (1 to: self size) do: [ :x | ret at: x put: (self at: x) lowercase asInteger ].
        ^ ret asString


]
Methods String

    printString
        ^ '''' , self, ''''


]
Methods String
    , value
        (value isMemberOf: String)
            ifTrue: [ ^ <24 self value> ]
            ifFalse: [ ^ self , value asString ]



]
Methods String

    asByteArray | newArray i |
        newArray <- ByteArray new: self size.
        i <- 0.
        self do: [:x | i <- i + 1. newArray at: i put: x asInteger].
        ^ newArray


]
Methods String

    asSymbol
        ^ <83 self>


]
Methods String

    unixCommand
        ^ <88 self>


]
Methods String

    value
        " evaluate self as an expression "
        ^ ( '^ [ ', self, ' ] value' ) execute


]
Methods String

    copy
        " catenation makes copy automatically "
        ^ '',self


]
Methods String

    basicAt: index put: aValue
        (aValue isMemberOf: Char)
            ifTrue: [ super basicAt: index put: aValue asInteger ]
            ifFalse: [ smalltalk error:
                'cannot put non Char into string' ]


]
Methods String

    print
        stdout print: self


]
Methods String

    printNoReturn
        stdout printNoReturn: self


]
Methods String

    < value
        (value isKindOf: String)
            ifTrue: [ ^ super < value ]
            ifFalse: [ ^ false ]


]
Methods String
    edit    | file text |
        file <- File new; 
            scratchFile;
            open: 'w';
            print: self;
            close.
        (editor, ' ', file name) unixCommand.
        file open: 'r'.
        text <- file asString.
        file close; delete.
        ^ text


]
Methods String

    asInteger
        ^ self inject: 0 into: [:x :y | x * 10 + y digitValue ]


]
Methods String

    execute | meth |
        " execute self as body of a method "
        meth <- Method new; text: 'compile ', self.
        (meth compileWithClass: Object)
            ifTrue: [ ^ meth executeWith: #(0) ].
        ^ nil


]
Methods String

    hash
        ^ <82 self>


]
Methods String

    copyFrom: position1 to: position2
        ^ <33 self position1 position2>


]
Methods String

    words: aBlock   | text index list |
        list <- List new.
        text <- self.
        [ text <- text copyFrom: 
            (text indexOf: aBlock ifAbsent: [ text size + 1])
                to: text size.
          text size > 0 ] whileTrue:
            [ index <- text 
                indexOf: [:x | (aBlock value: x) not ]
                ifAbsent: [ text size + 1].
              list addLast: (text copyFrom: 1 to: index - 1).
              text <- text copyFrom: index to: text size ].
        ^ list asArray


]
Methods String

    = value
        (value isKindOf: String)
            ifTrue: [ ^ super = value ]
            ifFalse: [ ^ false ]


]
Methods String

    basicAt: index
        ^  (super basicAt: index) asCharacter


]
Methods String

    size
        ^ <81 self>


]
Methods MetaWidget
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^newObj


]
Methods Browser

  show
    dialog show.


]
Methods Browser

  showMethod: method on: class
    | defPnl |
    defPnl <- panels at: 3.
    defPnl showMethod: method on: class


]
Methods Browser

  clearMethod
    | defPnl |
    defPnl <- panels at: 3.
    defPnl clearMethod


]
Methods Browser

  panels: thePanels
    panels <- thePanels.
    ^self


]
Methods Browser

  buttonPanel
    ^ ButtonPanel new: self


]
Methods Browser

  updateClassList
    | navPnl |
    navPnl <- panels at: 1.
    navPnl updateClassList


]
Methods Browser

  definitionPanel
    ^ DefinitionPanel new: self


]
Methods Browser
  init
    | container |
    container <- Split with: ((panels at: 1) widget) and: ((panels at: 3) widget); at: 'ORIENTATION' put: 'HORIZONTAL'. 
    dialog <- Dialog new: container; at: 'SIZE' put: '400x200'; at: 'SHRINK' put: 'YES'; at: 'TITLE' put: 'Tumbleweed Browser v0.1'.
    ^ self


]
Methods Browser

  panels
    ^ Array new: 3; at: 1 put: (self navigationPanel); 
                    at: 2 put: (self buttonPanel); 
                    at: 3 put: (self definitionPanel)


]
Methods Browser

  navigationPanel
    ^ NavigationPanel new: self


]
Methods Browser

  showClass: class
    | defPnl |
    defPnl <- panels at: 3.
    defPnl showClass: class


]
Methods NavigationPanel
  init: browser
    | protocolList methodList currentClass currentProtocol currentMethod |
    super init: browser.
    hbox <- HBox new; at: 'HOMOGENEOUS' put: 'YES'.
    classList <- self listControl: 'Classes' on: hbox.
    protocolList <- self listControl: 'Protocols' on: hbox.
    methodList <- self listControl: 'Methods' on: hbox.
    widget <- VBox new; at: 'NMARGIN' put: '5x5'; addChild: hbox.
    classList setActionCallback: [ :h :className :item :state | 
      (state = 1) ifTrue: [
        currentClass <- (className asSymbol) value.
        self clearList: methodList.
        self updateProtocolList: protocolList on: currentClass.
        browser showClass: currentClass
      ]
    ].
    protocolList setActionCallback: [ :h :protocolName :item :state | 
      (state = 1) ifTrue: [
        currentProtocol <- protocolName asSymbol.
        self clearList: methodList.
        self updateMethodList: methodList with: currentProtocol on: currentClass 
      ]
    ].
    methodList setActionCallback: [ :h :methodName :item :state | 
      (state = 1) ifTrue: [
        currentMethod <- (currentClass methodNamed: (methodName asSymbol)).
        browser showMethod: currentMethod on: currentClass
      ]
    ].
    self updateClassList.
    ^ self


]
Methods NavigationPanel

  listControl: name on: container
    | list vbox |
    list <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '0x50'.
    vbox <- VBox new.
    vbox addChild: (LabelControl new: (name asString)).
    vbox addChild: list.
    container addChild: vbox.
    ^ list


]
Methods NavigationPanel

  updateClassList
    | index |
    " Initialise the class list "
    self clearList: classList.
    index <- 1.
    ((classes select: [ :x | x isMeta not ]) sort: [ :x :y | (x asString) < (y asString) ] ) do: [ :x |
      classList at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self


]
Methods NavigationPanel

  updateProtocolList: list on: class
    | index |
    self clearList: list.
    browser clearMethod.
    index <- 1.
    (class protocols) do: [ :x |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self


]
Methods NavigationPanel

  updateMethodList: list with: protocol on: class
    | index |
    self clearList: list.
    browser clearMethod.
    index <- 1.
    (class methodsFor: (protocol asString)) binaryDo: [ :x :y |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self


]
Methods NavigationPanel

  clearList: list
    list at: 'REMOVEITEM' put: (cPointer new).
    ^ self


]
Methods EmployeeRecord
salary: aFloat
  salary <- aFloat


]
Methods EmployeeRecord
upDate
  name <- self promptString: 'name' currentValue: name.
  position <- (self promptString: 'position' currentValue: position) asSymbol.
  salary <- (self promptString: 'salary' currentValue: salary) asInteger



]
Methods EmployeeRecord
position
  ^ position


]
Methods EmployeeRecord
position: aSymbol
  position <- aSymbol


]
Methods EmployeeRecord
printString
  ^ 'name: ', name,
    ' position: ', position,
    ' salary: ', salary



]
Methods EmployeeRecord
new
  self initialize


]
Methods EmployeeRecord
idNumber
  ^ idNumber


]
Methods EmployeeRecord
promptString: aString currentValue: aValue
  | reply |
  reply <- smalltalk getPrompt: (aString, '(', aValue, ') : ').
  (reply size = 0)
    ifTrue: [ ^ aValue ]
    ifFalse: [ ^ reply ]



]
Methods EmployeeRecord
idNumber: anInteger
  idNumber <- anInteger



]
Methods EmployeeRecord
initialize
  name <- smalltalk getPrompt: 'name: '.
  position <- (smalltalk getPrompt: 'position: ') asSymbol.
  salary <- (smalltalk getPrompt: 'salary: ') asInteger





]
Methods EmployeeRecord
name: aString
  name <- aString


]
Methods EmployeeRecord
salary
  ^ salary


]
Methods EmployeeRecord
name
  ^ name


]
Methods False

    printString
        ^ 'false'


]
Methods False

    xor: aBoolean
        ^ aBoolean


]
Methods False

    not
        ^ true


]
Methods False
    ifTrue: trueBlock ifFalse: falseBlock
        ^ falseBlock value


]
Methods Object

    isNil
        ^ false


]
Methods Object

    broadcast: aSymbol with: anObject
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [nil].
        dependencies notNil ifTrue: [
            dependencies do: [:dep | dep perform: aSymbol with: anObject]
        ]


]
Methods Object

    respondsTo: message
        self class upSuperclassChain: 
            [:c | (c methodNamed: message) notNil
                    ifTrue: [ ^ true ]].
        ^ false


]
Methods Object

    isFraction
        ^ false


]
Methods Object

    class
        ^ <11 self>


]
Methods Object
    isNumber
        ^ false


]
Methods Object

    printString
        ^ self class printString


]
Methods Object
    isPoint
        ^ false


]
Methods Object

    new
        " default initialization protocol"
        ^ self


]
Methods Object

    isFloat
        ^ false


]
Methods Object

    isShortInteger
        ^ false


]
Methods Object

    changed: aParameter
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [nil].
        dependencies notNil ifTrue: [
            dependencies do: [ :dep | dep update: aParameter ]
        ]


]
Methods Object

    isKindOf: aClass
        self class upSuperclassChain:
            [:x | (x == aClass) ifTrue: [ ^ true ] ].
        ^ false


]
Methods Object

    isLongInteger
        ^ false


]
Methods Object

    display
        ('(Class ', self class, ') ' , self printString ) print


]
Methods Object

    isMemberOf: aClass
        ^ self class == aClass


]
Methods Object

    copy
        ^ self shallowCopy


]
Methods Object

    changed
        self changed: self


]
Methods Object

    deepCopy    | newObj |
        newObj <- self class new.
        (1 to: self basicSize) do: 
            [:i | newObj basicAt: i put: (self basicAt: i) copy].
        ^ newObj


]
Methods Object

    basicAt: index put: value
        ^ <31 self index value>


]
Methods Object

    shallowCopy | newObj |
        newObj <- self class new.
        (1 to: self basicSize) do: 
            [:i | newObj basicAt: i put: (self basicAt: i) ].
        ^ newObj


]
Methods Object

    print
        self printString print 


]
Methods Object

    perform: selector with: arg
        | args |
        (selector isMemberOf: Symbol) ifTrue: [
            (self respondsTo: selector) ifTrue: [
                args <- Array new: 2; at: 1 put: self; at: 2 put: arg.
                smalltalk perform: selector withArguments: args
            ]
        ] 


]
Methods Object

    asString
        ^ self printString


]
Methods Object
    message: m notRecognizedWithArguments: a
        | msg |
        msg <- 'Message ', (m printString), ' not recognized on ', (self class printString), ' with arguments [', (a printString), ']'.
        Error with: msg; signal


]
Methods Object

    == aValue
        ^ <21 self aValue>


]
Methods Object

    removeDependent: anObject
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [^anObject].
        dependencies removeKey: anObject ifAbsent: [].
        dependencies size < 1 ifTrue: [
            Object dependencies removeKey: self ifAbsent: []
        ].
        ^anObject


]
Methods Object
    dependents
    | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [^ List new].
        ^ dependencies 


]
Methods Object
    assign: name value: val
        ^ name assign: val


]
Methods Object

    notNil
        ^ true


]
Methods Object

    update: aParameter
        nil


]
Methods Object

    broadcast: aSymbol
        | dependencies |
        dependencies <- Object dependencies at: self ifAbsent: [nil].
        dependencies notNil ifTrue: [
            dependencies do: [:dep | dep perform: aSymbol]
        ]


]
Methods Object

    isInteger
        ^ false


]
Methods Object

    ~= avalue
        ^ self ~~ aValue


]
Methods Object

    basicSize
        ^ <12 self>


]
Methods Object

    addDependent: anObject
        (Object dependencies at: self ifAbsentPut: [List new])
            add: anObject


]
Methods Object

    hash
        ^ <13 self>


]
Methods Object

    perform: selector
        | args |
        (selector isMemberOf: Symbol) ifTrue: [
            (self respondsTo: selector) ifTrue: [
                args <- Array new: 1; at: 1 put: self.
                smalltalk perform: selector withArguments: args
            ]
        ] 


]
Methods Object

    ~~ aValue
        ^ (self == aValue) not


]
Methods Object

    = aValue
        ^ self == aValue


]
Methods Object

    basicAt: index
        ^ <25 self index>


]
Methods Semaphore

    signal
        (processList size = 0)
            ifTrue: [ count <- count + 1]
            ifFalse: [ scheduler critical:
                [ processList first resume.
                    processList removeFirst ]]


]
Methods Semaphore

    wait        | process |
        (count = 0)
            ifTrue: [ scheduler critical:
                    [ process <- scheduler currentProcess.
                      processList add: process.
                      scheduler removeProcess: process].
                  scheduler yield ]
            ifFalse: [ count <- count - 1]


]
Methods Semaphore
    new
        count <- 0.
        processList <- List new


]
Methods Semaphore

    set: aNumber
        count <- aNumber


]
Methods Semaphore

    critical: aBlock
        self wait.
        aBlock value.
        self signal


]
Methods MetaNode

  new
    smalltalk error: 'Cannot create Node without a name'


]
Methods MetaNode

  new: aString
    | newObj |
    newObj <- super new.
    newObj init: aString.
    ^ newObj


]
Methods MetaNode
  doInit
    errorException <- NodeException new


]
Methods True

    printString
        ^ 'true'


]
Methods True

    xor: aBoolean
        ^ aBoolean not


]
Methods True

    not
        ^ false


]
Methods True
    ifTrue: trueBlock ifFalse: falseBlock
        ^ trueBlock value


]
Methods Split

  left: child1 right: child2
    | gripper |
    iup <- IUP instance.
    iup IupAppend: (child1 handle) to: (self handle).
    iup IupAppend: (child2 handle) to: (self handle).


]
Methods Split
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupSplitWith: (CPointer new) and: (CPointer new).
    super handle: (result at: 1).
    ^ self


]
Methods Rectangle
    + v
        ^ Rectangle new; bottomRight: right@bottom + v;
            upperLeft: left@top + v


]
Methods Rectangle

    printString
        ^ ((left@top) printString) , ':', ((right@bottom) printString)


]
Methods Rectangle

    height
        ^ bottom - top


]
Methods Rectangle

    width
        ^ right - left


]
Methods Rectangle

    bottom
        ^ bottom


]
Methods Rectangle

    upperLeft: aPoint
        left <- aPoint x.
        top <- aPoint y.


]
Methods Rectangle

    right
        ^ right


]
Methods Rectangle

    inset: aPoint
        self upperLeft: left@top + aPoint.
        self bottomRight: right@bottom - aPoint


]
Methods Rectangle

    bottomRight: aPoint
        right <- aPoint x.
        bottom <- aPoint y.


]
Methods Rectangle

    top
        ^ top


]
Methods Rectangle

    - v
        ^ Rectangle new; bottomRight: right@bottom - v;
            upperLeft: left@top - v


]
Methods Rectangle

    contains: aPoint
        ^ aPoint between: left@top and: right@bottom


]
Methods Rectangle

    left
        ^ left


]
Methods IUP

  IupInsert: child after: ref on: parent 
    | args |
    args <- Array new: 3.
    args at: 1 put: parent.
    args at: 2 put: ref.
    args at: 3 put: child.
    ^ iup call: 'IupInsert' returning: #cObject taking: #(cObject cObject cObect) as: args 


]
Methods IUP

  IupGetColorRed: r Green: g Blue: b
    ^ iup call: 'IupGetColor' returning: #int taking: #(int int charOut charOut charOut) as: #(10 10 r g b)


]
Methods IUP

  IupShow: dialog
    | args |
    args <- Array new: 1.
    args at: 1 put: dialog.
    ^ iup call: #IupShow returning: #int taking: #(cObject) as: args


]
Methods IUP

  IupSplitWith: child1 and: child2 
    | args |
    args <- Array new: 2.
    args at: 1 put: child1.
    args at: 2 put: child2.
    ^ iup call: 'IupSplit' returning: #cObject taking: #(cObject cObject) as: args


]
Methods IUP

  IupPopup: child at: x and: y
    | args |
    args <- Array new: 3.
    args at: 1 put: child.
    args at: 2 put: x.
    args at: 3 put: y.
    ^ iup call: 'IupPopup' returning: #int taking: #(cObject int int) as: args


]
Methods IUP

  IupDialog: child 
    | args |
    args <- Array new: 1.
    args at: 1 put: child.
    ^ iup call: 'IupDialog' returning: #cObject taking: #(cObject) as: args


]
Methods IUP

  IupLabel: title
    | args |
    args <- Array new: 1.
    args at: 1 put: title.
    ^ iup call: #IupLabel returning: #cObject taking: #(string) as: args


]
Methods IUP

  IupGetAttribute: name on: handle 
    | args |
    args <- Array new: 2.
    args at: 1 put: handle.
    args at: 2 put: name.
    ^ iup call: #IupGetAttribute returning: #string taking: #(cObject string) as: args


]
Methods IUP

  IupHide: dialog
    | args |
    args <- Array new: 1.
    args at: 1 put: dialog.
    ^ iup call: #IupHide returning: #int taking: #(cObject) as: args


]
Methods IUP

  IupAppend: child to: parent 
    | args |
    args <- Array new: 2.
    args at: 1 put: parent.
    args at: 2 put: child.
    ^ iup call: 'IupAppend' returning: #cObject taking: #(cObject cObject) as: args 


]
Methods IUP

  IupVbox 
    | args |
    args <- Array new: 2.
    args at: 1 put: (CPointer new).
    args at: 2 put: (CPointer new).
    ^ iup call: 'IupVbox' returning: #cObject taking: #(cObject cObject) as: args


]
Methods IUP
  init
    iup <- FFI new: 'iup'.
    iup call: 'IupOpen' returning: #int taking: #(int int) as: #(0 0).
    ^self


]
Methods IUP

  IupHbox 
    | args |
    args <- Array new: 2.
    args at: 1 put: (CPointer new).
    args at: 2 put: (CPointer new).
    ^ iup call: 'IupHbox' returning: #cObject taking: #(cObject cObject) as: args


]
Methods IUP

  IupButton: title 
    | args |
    args <- Array new: 2.
    args at: 1 put: title.
    args at: 2 put: 'DefaultButtonAction'.
    ^ iup call: 'IupButton' returning: #cObject taking: #(string string) as: args 


]
Methods IUP

  IupFill 
    | args |
    ^ iup call: 'IupFill' returning: #cObject taking: #() as: #()


]
Methods IUP

  IupStoreAttribute: name on: handle as: value
    | args |
    args <- Array new: 3.
    args at: 1 put: handle.
    args at: 2 put: name.
    args at: 3 put: value.
    ^ iup call: #IupStoreAttribute returning: #void taking: #(cObject string string) as: args


]
Methods IUP

  IupMenu
    | args |
    args <- Array new: 2.
    args at: 1 put: (CPointer new).
    args at: 2 put: (CPointer new).
    ^ iup call: 'IupMenu' returning: #cObject taking: #(cObject cObject) as: args


]
Methods IUP

  IupLoopStep
    | result |
    result <- iup call: #IupLoopStep returning: #int taking: #() as: #().
    ^ result at: 1


]
Methods IUP

  IupMessage: m title: t
    | args |
    args <- Array new: 2.
    args at: 1 put: t.
    args at: 2 put: m.
    ^ iup call: 'IupMessage' returning: #void taking: #(string string) as: args 


]
Methods IUP

  IupGetChildAt: pos on: parent
    | args |
    args <- Array new: 2.
    args at: 1 put: parent.
    args at: 2 put: pos.
    ^ iup call: 'IupGetChild' returning: #cObject taking: #(cObject int) as: args


]
Methods IUP

  IupSetCallbackOn: handle for: action as: callback
    | args |
    args <- Array new: 3.
    args at: 1 put: handle.
    args at: 2 put: action.
    args at: 3 put: callback.
    ^ iup call: #IupSetCallback returning: #cObject taking: #(cObject string cObject) as: args


]
Methods IUP

  IupText
    ^ iup call: #IupText returning: #cObject taking: #(string) as: #('DefaultListAction')


]
Methods IUP

  IupList
    ^ iup call: #IupList returning: #cObject taking: #(string) as: #('DefaultListAction')


]
Methods IUP

  IupMainLoop
    ^ iup call: #IupMainLoop returning: #int taking: #() as: #()


]
Methods IUP

  IupItem: title
    | args |
    args <- Array new: 2.
    args at: 1 put: title.
    args at: 2 put: 'DefaultMenuAction'.
    ^ iup call: 'IupItem' returning: #cObject taking: #(string string) as: args 


]
Methods MetaDialog
  new: child
    | newDialog |
    newDialog <- super new.
    newDialog init: child.
    ^ newDialog


]
Methods Array
+ aValue        | size1 size2 newValue |
        " catenate two strings together "
        size1 <- self size.
        size2 <- aValue size.
        newValue <- self class new: (size1 + size2).
        newValue replaceFrom: 1 to: size1 with: self.
        newValue replaceFrom: size1+1 to: size1+size2 with: aValue.
        ^ newValue









]
Methods Array

    with: coll do: aBlock
        (1 to: (self size min: coll size))
            do: [:i | aBlock value: (self at: i) 
                    value: (coll at: i) ]


]
Methods Array

    with: coll ifAbsent: z do: aBlock   | xsize ysize |
        xsize <- self size.
        ysize <- coll size.
        (1 to: (xsize max: ysize))
            do: [:i | aBlock value:
              (i <= xsize ifTrue: [ self at: i ] ifFalse: [ z ])
              value:
              (i <= ysize ifTrue: [ coll at: i ] ifFalse: [ z ])]


]
Methods Array

    new
        ^ smalltalk error: 'arrays and strings cannot be created using new'



]
Methods Array

    with: newElement    | s newArray |
        s <- self size.
        newArray <- Array new: (s + 1).
        (1 to: s) do: [:i | newArray at: i put: (self at: i) ].
        newArray at: s+1 put: newElement.
        ^ newArray


]
Methods Array
replaceFrom: start to: stop with: replacement
        ^ self replaceFrom: start to: stop with: replacement startingAt: 1





]
Methods Array

    deepCopy
        ^ self deepCopyFrom: 1 to: self size


]
Methods Array

    select: aCond   | newList |
        newList <- List new.
        self do: [:i | (aCond value: i) ifTrue: [newList addLast: i]].
        ^ newList asArray


]
Methods Array

    shallowCopy
        ^ self copyFrom: 1 to: self size


]
Methods Array

  combineWith: anArray
    | newArray |
    (anArray class = self class) 
        ifFalse: [ smalltalk error: ('Error, cannot combine different collection types: ', (self class), ' : ', (anArray class)) ].
    newArray <- self class new: (self size + anArray size).
    (1 to: (self size))
        do: [:i |  newArray basicAt: i put: (self at: i) ].
    (1 to: (anArray size))
        do: [:i |  newArray basicAt: (i + self size) put: (anArray at: i) ].
    ^ newArray


]
Methods Array

    exchange: a and: b  | temp |
        temp <- self at: a.
        self at: a put: (self at: b).
        self at: b put: temp


]
Methods Array
    < coll
        (coll isKindOf: Array)
            ifTrue: [ self with: coll 
                   do: [:x :y | (x = y) ifFalse: 
                          [ ^ x < y ]].
                  ^ self size < coll size ]
            ifFalse: [ ^ super < coll ]


]
Methods Array

    deepCopyFrom: low to: high  | newArray newlow newhigh |
        newlow <- low max: 1.
        newhigh <- high min: self size.
        newArray <- self class new: (0 max: newhigh - newlow + 1).
        (newlow to: newhigh)
            do: [:i |  newArray at: ((i - newlow) + 1)
                    put: (self at: i) copy ].
        ^ newArray


]
Methods Array

    do: aBlock
        (1 to: self size) do:
            [:i | aBlock value: (self at: i) ]


]
Methods Array

    at: index put: value
        (self includesKey: index)
            ifTrue: [ self basicAt: index put: value ]
            ifFalse: [ smalltalk error: 
                ('illegal index to at:put: for array: ', index, '(', (self size), ')') ]


]
Methods Array

    includesKey: index
        ^ index between: 1 and: self size


]
Methods Array
replaceFrom: start to: stop with: replacement startingAt: repStart | base |
        base <- repStart-1.
        (0 to: (stop - start)) do: [:idx|
                self basicAt: (idx + start) put:
                        (replacement at: (idx + repStart))
        ]




]
Methods Array

    grow: aValue    | s newArray |
        s <- self size.
        newArray <- Array new: s + 1.
        (1 to: s) do: [:i | newArray at: i put: (self at: i)].
        newArray at: s+1 put: aValue.
        ^ newArray


]
Methods Array

    collect: aBlock     | s newArray |
        s <- self size.
        newArray <- Array new: s.
        (1 to: s) do: [:i | newArray at: i put: 
            (aBlock value: (self at: i))].
        ^ newArray


]
Methods Array

  from: low to: high 
    | start stop size obj |
    start <- low max: 1.
    stop <- high min: self size.
    size <- (stop + 1 - start) max: 1.
    obj <- (self class) new: size.
    (1 to: size) do: [ :i |
      obj basicAt: i put: (self at: start).
      start <- start + 1 ].
    ^ obj


]
Methods Array

    binaryDo: aBlock
        (1 to: self size) do:
            [:i | aBlock value: i value: (self at: i) ]


]
Methods Array

    copyFrom: low to: high  | newArray newlow newhigh |
        newlow <- low max: 1.
        newhigh <- high min: self size.
        newArray <- self class new: (0 max: newhigh - newlow + 1).
        (newlow to: newhigh)
            do: [:i |  newArray at: ((i - newlow) + 1)
                    put: (self at: i) ].
        ^ newArray


]
Methods Array

    reverseDo: aBlock
        (self size to: 1 by: -1) do:
            [:i | aBlock value: (self at: i) ]


]
Methods Array

    = coll
        (coll isKindOf: Array)
            ifTrue: [ (self size = coll size)
                    ifFalse: [ ^ false ].
                  self with: coll
                    do: [:x :y | (x = y) 
                        ifFalse: [ ^ false ] ]. 
                 ^ true ]
            ifFalse: [ ^ super = coll ]


]
Methods Array

    size
        ^ self basicSize


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


]
Methods Exception

    signalWith: aString
        message <- aString.
        self signal


]
Methods Exception

    message: aString
        message <- aString


]
Methods Exception

    message
        ^ message


]
Methods Interval

    lower: aValue
        lower <- aValue


]
Methods Interval

    step: aValue
        step <- aValue


]
Methods Interval
    do: aBlock      | current |
        current <- lower.
        (step > 0) 
            ifTrue: [ [ current <= upper ] whileTrue:
                    [ aBlock value: current.
                    current <- current + step ] ]
            ifFalse: [ [ current >= upper ] whileTrue:
                    [ aBlock value: current.
                    current <- current + step ] ]


]
Methods Interval

    upper: aValue
        upper <- aValue


]
Methods TextControl
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupText.
    super handle: (result at: 1).
    ^self


]
Methods TextControl

  insert: aString
    ^ super at: 'INSERT' put: (aString asString)


]
Methods MetaTestSuite
        named: aString
            ^ self new setName: aString


]
Methods LongInteger

    + n     | newDigits z carry |
        n isLongInteger
            ifFalse: [ ^ super + n ].
        negative ifTrue: [ ^ n - self negated ].
        n negative ifTrue: [ ^ self - n negated ].
        " reduced to positive + positive case "
        newDigits <- List new.  carry <- 0.
        self with: n bitDo:
            [:x :y | z <- x + y + carry.
                (z >= 100) ifTrue: [ carry <- 1. z <- z - 100]
                     ifFalse: [ carry <- 0 ].
                newDigits addLast: z ].
        carry > 0 ifTrue: [ newDigits addLast: carry ].
        ^ LongInteger new; sign: false digits: newDigits asArray


]
Methods LongInteger

    negative
        ^ negative


]
Methods LongInteger

    negated
        ^ LongInteger new; sign: negative not digits: digits


]
Methods LongInteger

    printString | str |
        str <- negative ifTrue: [ '-' ] ifFalse: [ '' ].
        digits reverseDo: [:x | str <- str , 
            (x quo: 10) printString , (x rem: 10) printString ].
        ^ str


]
Methods LongInteger

    timesShort: value   | y z carry newDigits |
        y <- value abs.
        carry <- 0.
        newDigits <- digits collect:
            [:x | z <- x * y + carry. 
                carry <- z quo: 100. 
                z - (carry * 100)].
        (carry > 0) ifTrue: [ newDigits <- newDigits grow: carry ].
        ^ LongInteger new; sign: (negative xor: value negative) 
                    digits: newDigits


]
Methods LongInteger

    new
        "override restriction from class Integer"
        ^ self


]
Methods LongInteger

    abs
        negative ifTrue: [ ^ self negated] 


]
Methods LongInteger

    isShortInteger
        " override method in class Integer "
        ^ false


]
Methods LongInteger

    coerce: n
        ^ n asLongInteger


]
Methods LongInteger

    sign: s digits: d
        negative <- s.
        digits <- d.


]
Methods LongInteger

    isLongInteger
        ^ true


]
Methods LongInteger

    asFloat     | r |
        r <- 0.0 .
        digits reverseDo: [ :x | r <- r * 100.0 + x asFloat].
        negative ifTrue: [ r <- r negated ].
        ^ r.


]
Methods LongInteger

    quo: value  | a b quo result |
        result <- 0.
        a <- self abs. b <- value abs.
        [a > b] whileTrue:
            [ quo <- (a asFloat quo: b). result <- result + quo.
                a <- a - (b * quo) ].
        ^ result


]
Methods LongInteger
    < n     | result |
        n isLongInteger
            ifFalse: [ ^ super < n ].
        (negative == n negative) ifFalse: [ ^ negative ].
        " now either both positive or both negative "
        result <- false.
        self with: n bitDo: 
            [:x :y | (x ~= y) ifTrue: [ result <- x < y]].
        negative ifTrue: [ result <- result not ].
        ^ result


]
Methods LongInteger

    generality
        ^ 4 "generality value - used in mixed type arithmetic "


]
Methods LongInteger

    * n     | result |
        n isShortInteger ifTrue: [ ^ self timesShort: n ].
        n isLongInteger  ifFalse: [ ^ super * n ].
        result <- 0 asLongInteger.
        digits reverseDo: 
            [:x | result <- (result timesShort: 100) +
                (n timesShort: x)].
        negative ifTrue: [ result <- result negated ].
        ^ result


]
Methods LongInteger

    with: n bitDo: aBlock   | d di dj |
        " run down two digits lists in parallel doing block "
        di <- digits size.
        d <- n digits.
        dj <- d size.
        (1 to: (di max: dj)) do: [:i |
            aBlock value: 
               ((i <= di) ifTrue: [ digits at: i] ifFalse: [0])
                value:
               ((i <= dj) ifTrue: [ d at: i] ifFalse: [0]) ]


]
Methods LongInteger

    digits
        ^ digits


]
Methods LongInteger

    bitShift: n
        (n >= 0)
            ifTrue: [ ^ self * (2 raisedTo: n) ]
            ifFalse: [ ^ self quo: (2 raisedTo: n negated)]


]
Methods LongInteger

    - n     | result newDigits z borrow |
        n isLongInteger
            ifFalse: [ ^ super - n ].
        negative ifTrue: [ ^ (self negated + n) negated ].
        n negative ifTrue: [ ^ self + n negated ].
        (self < n) ifTrue: [ ^ (n - self) negated ].
        " reduced to positive - smaller positive "
        newDigits <- List new. borrow <- 0.
        self with: n bitDo:
            [:x :y | z <- (x - borrow) - y.
                (z >= 0) ifTrue: [ borrow <- 0]
                ifFalse: [ z <- z + 100. borrow <- 1].
                newDigits addLast: z ].
        result <- 0. "now normalize result by multiplication "
        newDigits reverseDo: [:x | result <- result * 100 + x ].
        ^ result


]
Methods LongInteger

    = n
        n isLongInteger
            ifFalse: [ ^ super = n ].
        (negative == n negative) ifFalse: [ ^ false ].
        ^ digits = n digits


]
Methods NodeTest
  setUp
    nodeA <- Node new: 'A'.
    nodeB <- Node new: 'B'


]
Methods NodeTest
  testAddRemoveChild
    "Test adding a child in good and bad conditions"
    self should: [ nodeA addChild: nodeB. nodeB parent = nodeA ] description: 'addChild should set parent on child.'.
    self should: [ (nodeA children) size = 1 ] description: 'After adding a child, count should be one'.
    self should: [ [ nodeA removeChild: nodeA ] on: Exception do: [ :ex | false ]. true ] description: 'Should not except when removing valid child'.
    self should: [ (nodeA removeChild: nodeB) = nodeA ] description: 'Should not except when removing valid child'.
    self should: [ (nodeA children) size = 0 ] description: 'After removing a child, count should be zero'.


]
Methods Block

    on: anException do: aBlock
        scheduler currentProcess recordHandler: aBlock for: anException.
        ^ self valueAndResumeOnUnwind


]
Methods Block

    forkWith: args
        (self newProcessWith: args) resume


]
Methods Block

    valueAndResumeOnUnwind
        ^ self value


]
Methods Block
    checkArgumentCount: count
        ^ (argCount = count)
            ifTrue: [ true ]
            ifFalse: [ smalltalk error:
                'wrong number of arguments passed to block'.
                false ]


]
Methods Block

    value
        ^ (self checkArgumentCount: 0)
            ifTrue: [ context returnToBlock: bytePointer ]


]
Methods Block

    whileTrue: aBlock
        ( self value ) ifTrue:
            [ aBlock value. 
                self whileTrue: aBlock ]


]
Methods Block

    whileFalse: aBlock
        [ self value not ] whileTrue: aBlock


]
Methods Block

    blockContext: ctx
        context <- ctx


]
Methods Block

    value: x value: y
        ^ (self checkArgumentCount: 2)
            ifTrue: [ context at: argLoc put: x.
                  context at: argLoc + 1 put: y.
                  context returnToBlock: bytePointer ]


]
Methods Block
    newProcess 
        " create a new process to execute block "
        ^ Process new; context: context; startAt: bytePointer.


]
Methods Block

    value: x
        ^ (self checkArgumentCount:  1)
            ifTrue: [ context at: argLoc put: x.
                  context returnToBlock: bytePointer ]


]
Methods Block

    whileTrue
        self whileTrue: []


]
Methods Block

    newProcessWith: args
        (self checkArgumentCount: args size)
            ifTrue: [ (1 to: args size) do: [:i |
                   context at: (argLoc + i - 1) 
                    put: (args at: i)]].
        ^ self newProcess


]
Methods Block

    fork
        self newProcess resume


]
Methods Block

    value: x value: y value: z
        ^ (self checkArgumentCount:  3)
            ifTrue: [ context at: argLoc put: x.
                  context at: argLoc + 1 put: y.
                  context at: argLoc + 2 put: z.
                  context returnToBlock: bytePointer ]


]
Methods MetaExternalData
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^ newObj


]
Methods EDTest
  fields
    ^ #((string string)
        (number int))


]
Methods Integer
    + value     | r |
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ r <- <60 self value>.
                  "primitive will return nil on overflow"
                  r notNil ifTrue: [ r ]
                ifFalse: [ self asLongInteger + value asLongInteger ]]
            ifFalse: [ super + value ]


]
Methods Integer

    asLongInteger   | newList i |
        newList <- List new.
    i <- self.
        i = 0 ifTrue: [ newList add: 0 ]
            ifFalse: [ i <- self abs.
                   [ i ~= 0 ] whileTrue: 
                    [ newList addLast: (i rem: 100).
                    i <- i quo: 100 ] ].
        ^ LongInteger new; sign: i negative digits: newList asArray


]
Methods Integer

    printString
        ^ self asString


]
Methods Integer

    odd
        ^ (self rem: 2) ~= 0


]
Methods Integer

    truncated
        ^ self


]
Methods Integer

    , value
        " used to make long integer constants "
        ^ self * 1000 + value


]
Methods Integer

    new
        ^ smalltalk error: 'cannot create integers with new'


]
Methods Integer

    asFraction
        ^ Fraction new ; with: self over: 1


]
Methods Integer

    isShortInteger
        ^ true


]
Methods Integer

    bitInvert
        "invert all bits in self"
        ^ self bitXor: -1


]
Methods Integer

    radix: base     | sa text |
        " return a printed representation of self in given base"
        sa <- self abs.
        text <- (sa \\ base) asDigit asString.
        ^ (sa < base)
            ifTrue: [ (self negative)
                    ifTrue: [ '-' , text ]
                    ifFalse: [ text ]]
            ifFalse: [ ((self quo: base) radix: base), text ]


]
Methods Integer
 
    benchmark
        "Handy bytecode-heavy benchmark -- approx 500000 bytecodes per run:
        (500000 * times ran // secs to run) = approx bytecodes per second"
 
        | size flags prime k count |
        size <- 1.
        flags <- Array new: size.
        self timesRepeat: [
            count <- 0.
            flags atAllPut: true.
            (1 to: size) do:
            [:i | (flags at: i) ifTrue:
                 [ prime <- i+1.
                 k <- i + prime.
                 [k <= size] whileTrue:
                     [ flags at: k put: false.
                     k <- k + prime ].
                 count <- count + 1]]].
     ^count


]
Methods Integer

    factorial
        ^ (2 to: self) inject: 1 into: [:x :y | x * y ]


]
Methods Integer

    / value     | t b |
        value = 0 ifTrue: [ ^ smalltalk error: 'division by zero'].

        value isInteger
            ifTrue: [ b <- self gcd: value .
                  t <- self quo: b.
                  b <- value quo: b.
                  b negative
                    ifTrue: [ t <- t negated. 
                          b <- b negated ].
                  (b = 1) ifTrue: [ ^ t ].
                  ^ Fraction new; with: t over: b ]
            ifFalse: [ ^ super / value ]


]
Methods Integer

    allMask: value
        " see if all bits in argument are on"
        ^ value = (self bitAnd: value)


]
Methods Integer

    gcd: value
        (value = 0) ifTrue: [ ^ self ].
        (self negative) ifTrue: [ ^ self negated gcd: value ].
        (value negative) ifTrue: [ ^ self gcd: value negated ].
        (value > self) ifTrue: [ ^ value gcd: self ].
        ^ value gcd: (self rem: value)


]
Methods Integer

    > value
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ <63 self value> ]
            ifFalse: [ super > value ]


]
Methods Integer
 
    benchFib
        "Handy send-heavy benchmark -- result is number of sends:
        (result // seconds to run) = approx calls per second"
        ^self < 2
            ifTrue: [1] 
            ifFalse: [(self - 1) benchFib + (self - 2) benchFib + 1]


]
Methods Integer

    asFloat
        " should be redefined by any subclasses "
        self isShortInteger ifTrue: [ ^ <51 self> ]


]
Methods Integer

    bitAt: value
        ^ (self bitShift: 1 - value) bitAnd: 1


]
Methods Integer

    quo: value  | r |
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ r <- <69 self value>.
                (r isNil)
                    ifTrue: [ smalltalk error:
                        'quo: or rem: with argument 0']
                    ifFalse: [ r ]]
            ifFalse: [ ^ super quo: value ]


]
Methods Integer

    asDigit
        " return as character digit "
        (self >= 0)
            ifTrue: [ (self <= 9) ifTrue: 
                    [ ^ (self + $0 asInteger) asCharacter ].
                  (self < 36) ifTrue:
                    [ ^ (self + $A asInteger - 10) asCharacter ] ].
        ^ smalltalk error: ('illegal conversion, integer to digit [', self, ']')


]
Methods Integer

    asString
        ^ self radix: 10


]
Methods Integer

    asCharacter
        ^ Char new; value: self


]
Methods Integer

    < value
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ <62 self value> ]
            ifFalse: [ super < value ]


]
Methods Integer

    anyMask: value
        " see if any bits in argument are on"
        ^ 0 ~= (self bitAnd: value)


]
Methods Integer

    bitOr: value
        ^ (self bitXor: value) bitXor: (self bitAnd: value)


]
Methods Integer

    generality
        " generality value - used in mixed class arithmetic "
        ^ 2


]
Methods Integer

    * value     | r |
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ r <- <68 self value>.
                  "primitive will return nil on overflow"
                  r notNil ifTrue: [ r ]
                  ifFalse: [ self asLongInteger * value asLongInteger ]]
            ifFalse: [ super * value ]


]
Methods Integer

    tinyBenchmarks
        | t1 t2 r n1 n2 |
        n1 <- 1 bitShift: self.
        "[t1 <- Time millisecondsToRun: [n1 benchmark].
            t1 < 5000] whileTrue:[ n1 <- n1 * 2 ]."

        n2 <- 24 + self.
        [t2 <- Time millisecondsToRun: [r <- n2 benchFib].
            t2 < 5000] whileTrue:[ n2 <- n2 + 1 ].

        ^"((n1 * 500000 * 1000) // t1) printString, ' bytecodes/sec; ',"
            ((r * 1000) // t2) printString, ' sends/sec'


]
Methods Integer

    timesRepeat: aBlock | i |
        " use while, which is optimized, not to:, which is not"
        i <- 0.
        [ i < self ] whileTrue:
            [ aBlock value. i <- i + 1]


]
Methods Integer

    bitXor: value
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ <72 self value > ]
            ifFalse: [ smalltalk error: 
                'argument to bit operation must be integer']


]
Methods Integer

    bitShift: value
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ <79 self value > ]
            ifFalse: [ smalltalk error: 
                'argument to bit operation must be integer']


]
Methods Integer

    even
        ^ (self rem: 2) = 0


]
Methods Integer

    lcm: value
        ^ (self quo: (self gcd: value)) * value


]
Methods Integer

    - value     | r |
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ r <- <61 self value>.
                  "primitive will return nil on overflow"
                r notNil ifTrue: [ r ]
                ifFalse: [ self asLongInteger - value asLongInteger ]]
            ifFalse: [ super - value ]


]
Methods Integer

    bitAnd: value
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ <71 self value > ]
            ifFalse: [ smalltalk error: 
                'arguments to bit operation must be short integer']


]
Methods Integer

    = value
        ^ (self isShortInteger and: [value isShortInteger])
            ifTrue: [ self == value ]
            ifFalse: [ super = value ]


]
Methods MetaAddSubClassDialog
  new: parent
    | newDialog |
    newDialog <- super new.
    newDialog init: parent.
    ^ newDialog


]
Methods MetaItemControl
  new: aString
    | newItem |
    newItem <- super new.
    newItem init: aString.
    ^ newItem


]
Methods MetaIUP

  test
    | btn vbox dialog done e |
    iup <- IUP instance.
    btn <- ButtonControl new: 'Test'.
    stderr print: (btn handle asString).
    vbox <- VBox new.
    vbox addChild: btn.
    btn setActionCallback: [ :x | stderr print: ('Hello: ', (x asString)). -3 ].
    dialog <- Dialog new: vbox.
    dialog show.
    done <- false.
    [ done ] whileFalse: [ 
      e <- iup IupLoopStep.
      [ e = -3 ] whileFalse: [ e <- iup IupLoopStep ].
      stderr print: (e asString).
      done <- true
    ]


]
Methods MetaIUP
  new
    ^ self instance.


]
Methods MetaIUP

  instance
    theInstance isNil ifTrue: [ stderr print: 'Loading IUP'].
    theInstance isNil ifTrue: [ theInstance <- super new. theInstance init ].
    ^ theInstance


]
Methods File

    exists
        number notNil ifTrue: [ self close ].
        self getNumber.
        <120 number name mode> isNil
            ifTrue: [ ^ false ]
            ifFalse: [ ^ true ]


]
Methods File

    getString
        ^ (number notNil)
            ifTrue: [<125 number>]


]
Methods File

    open
        number notNil ifTrue: [ self close ].
        self getNumber.
        <120 number name mode> isNil
            ifTrue: [ smalltalk error: 
                 'open failed: ', name. ^ false].
        ^ true


]
Methods File

    saveImage
        " subtle problem - when we read in image don't want
          image file to be open for writing, so we remove it's
          number from files array temporarily "
        (number notNil)
            ifTrue: [ files at: number put: nil. 
                  <127 number>.
                  files at: number put: self]
            ifFalse: [smalltalk error: 'saveImage: file not open']


]
Methods File

    print: aString
        (number notNil)
            ifTrue: [<129 number aString>]
            ifFalse: [smalltalk error: 'file not open']


]
Methods File

    fileIn
        " read code into the system "
        (number notNil) ifTrue: [<123 number>]


]
Methods File

    printNoReturn: aString
        (number notNil)
            ifTrue: [<128 number aString>]
            ifFalse: [smalltalk error: 'file not open']


]
Methods File
    asString    | text line |
        text <- ''.
        [ (line <- self getString) notNil ]
            whileTrue: [ text <- text , line ].
        ^ text


]
Methods File

    fileIn: name
        self name: name.
        self open: 'r'.
        self fileIn.
        self close.


]
Methods File

    getNumber
        " get a file number - called only by open"
        (1 to: 15) do: [:i | (files at: i) isNil
            ifTrue: [ files at: i put: self. number <- i. ^ nil]]


]
Methods File

    readUntil: conditionBlock doing: actionBlock    | line |
        [ line <- self getString. line notNil]
            whileTrue: [ (conditionBlock value: line)
                    ifTrue: [ ^ line ].
                    actionBlock value: line ].
        ^ nil


]
Methods File

    name: string
        name <- string


]
Methods File

    mode: m
        mode <- m


]
Methods File

    name
        ^ name


]
Methods File

    open: m
        self mode: m.
        self open


]
Methods File
    scratchFile
        name <- 'junk.tmp'


]
Methods File

    delete
        ('rm ', name) unixCommand


]
Methods File

    close
        " close file, take entry out of global variable "
        number isNil ifTrue: [ ^ nil ].
        files at: number put: nil.
        <121 number>.
        number <- nil.


]
Methods MetaBrowser
  new
    | newObj |
    newObj <- super new.
    newObj panels: newObj panels.
    newObj init.
    ^ newObj


]
Methods MetaBrowser

  open
    self openOnClass: nil


]
Methods MetaBrowser

  openOnClass: class showing: method
    Browser new; show. IUP instance IupMainLoop


]
Methods MetaBrowser

  openOnClass: class
    self openOnClass: class showing: nil


]
Methods ButtonPanel
  init: browser
    | button |
    super init: browser.
    widget <- HBox new.
    #(browse hierarchy variables implementors inheritance senders versions view) do: [ :name |
      button <- ButtonControl new: (name asString).
      widget addChild: button
    ].
    ^ self


]
Methods Morph
  drawOn: aSurface
  (shape isNil) ifTrue: [
    shape <- SFMLRectangleShape new
  ].
  shape setPositionX: (bounds left) Y: (bounds top).
  shape setSizeX: (bounds width) Y: (bounds height).
  aSurface draw: shape


]
Methods Morph
  init: aString
    super init: aString.
    bounds <- Rectangle new.
    self setPositionX: 0 Y: 0; setWidth: 0 height: 0


]
Methods Morph

  setWidth: width height: height
    bounds bottomRight: ((self left+width)@(self top+height)) 


]
Methods Morph

  bottom
    ^ bounds bottom


]
Methods Morph

  right
    ^ bounds right


]
Methods Morph

  setPositionX: xpos Y: ypos
    bounds upperLeft: (xpos@ypos) 


]
Methods Morph

  top
    ^ bounds top


]
Methods Morph
  left
    ^ bounds left


]
Methods Test
    queen       | lastQueen |
        lastQueen <- NullQueen new.
        (1 to: 8) do: [:i | lastQueen <- Queen new;
                    setColumn: i neighbor: lastQueen ].
        lastQueen first.
        (lastQueen result asArray = #(1 5 8 6 3 7 2 4) )
            ifTrue: ['8 queens test passed' print]
            ifFalse: [smalltalk error: '8queen test failed']


]
Methods Context

    blockReturn
        <18 self>
            ifFalse: [ ^ smalltalk error: 
                'incorrect context for block return']


]
Methods Context

    returnToBlock: bytePtr
        " change the location we will return to, to execute a block"
        <28 self bytePtr>


]
Methods Context

    method: m
        method <- m


]
Methods Context

    copy
        ^ super copy temporaries: temporaries copy


]
Methods Context

    arguments: a
        arguments <- a


]
Methods Context
    at: key put: value
        temporaries at: key put: value


]
Methods Context

    temporaries: t
        temporaries <- t


]
Methods MetaArray
    new: size   " hack out block the right size and class "
        "create a new block, set its class"
        ^ < 22 < 58 size > self >


]
Methods Circle
    center: c
        center <- c


]
Methods Circle

    radius: r
        radius <- r


]
Methods MetaSplit
  with: child1 and: child2
    | newControl |
    newControl <- super new.
    newControl left: child1 right: child2.
    ^ newControl


]
Methods MetaExceptionHandler
    for: anException with: aBlock
    | handler |
    handler <- super new.
    handler handledException: anException.
    handler handlerBlock: aBlock.
    ^ handler


]
Methods Scheduler

    processCount
        ^ processList size


]
Methods Scheduler
    new
        "create a new scheduler with empty process list "
        notdone <- true.
        processList <- Set new.


]
Methods Scheduler

    run
        " run as long as process list is non empty "
        [ notdone ] whileTrue:
            [ processList size = 0 ifTrue: 
                [ self initialize ].
              processList do: 
                [ :x | currentProcess <- x.
                    x execute  ] ]


]
Methods Scheduler
    initialize  | string |
        [ [ true ] whileTrue: [ 
            string <- nil.
            string <- smalltalk getPrompt: '> '.
            string isNil
                ifTrue: [ notdone <- false ]
                ifFalse: [ (string size > 0)
                    ifTrue: [ 
                        echoInput ifTrue:
                            [ string print ].
                        [ string value print ] fork ] 
                ].
            self yield ] 
        ] fork








]
Methods Scheduler

    addProcess: aProcess
        " add a process to the process list "
        processList add: aProcess


]
Methods Scheduler

    yield
        " set time slice counter to zero, thereby
        yielding to next process "
        <53 0>


]
Methods Scheduler

    currentProcess
        " return the currently executing process "
        ^ currentProcess


]
Methods Scheduler

    critical: aBlock
        "set time slice counter high to insure bytecodes are
        executed before continuing "
        <53 10000>.
        aBlock value.
        "then yield processor "
        <53 0>.


]
Methods Scheduler

    removeProcess: aProcess
        " remove a given process from the process list "
        processList remove: aProcess.


]
Methods Link

    link: aLink
        nextLink <- aLink


]
Methods Link

    removeKey: aKey
        (aKey = key)
            ifTrue: [ ^ nextLink ]
            ifFalse: [ (nextLink notNil)
                ifTrue: [ nextLink <- nextLink removeKey: aKey]]


]
Methods Link

    value
        ^ value


]
Methods Link

    next
        ^ nextLink


]
Methods Link
    add: newValue whenFalse: aBlock
        (aBlock value: value value: newValue)
            ifTrue: [ (nextLink notNil)
                ifTrue: [ nextLink <- nextLink add: newValue 
                    whenFalse: aBlock ]
            ifFalse: [ nextLink <- Link new; value: newValue] ]
            ifFalse: [ ^ Link new; value: newValue; link: self ]


]
Methods Link

    removeValue: aValue
        (aValue = value)
            ifTrue: [ ^ nextLink ]
            ifFalse: [ (nextLink notNil)
                ifTrue: [ nextLink <- nextLink removeValue: aValue]]


]
Methods Link

    at: aKey ifAbsent: exceptionBlock
        (aKey = key)
            ifTrue: [ ^value ]
            ifFalse: [ ^ (nextLink notNil)
                    ifTrue: [ nextLink at: aKey
                            ifAbsent: exceptionBlock ]
                    ifFalse: exceptionBlock ]


]
Methods Link

    at: aKey put: aValue
        (aKey = key)
            ifTrue: [ value <- aValue ]
            ifFalse: [ (nextLink notNil)
                ifTrue: [ nextLink at: aKey put: aValue]
                ifFalse: [ nextLink <- Link new;
                        key: aKey; value: aValue] ]


]
Methods Link

    includesKey: aKey
        (key = aKey)
            ifTrue: [ ^ true ].
        (nextLink notNil)
            ifTrue: [ ^ nextLink includesKey: aKey ]
            ifFalse: [ ^ false ]


]
Methods Link

    key: aKey
        key <- aKey


]
Methods Link

    value: aValue
        value <- aValue


]
Methods Link

    binaryDo: aBlock
        aBlock value: key value: value.
        (nextLink notNil)
            ifTrue: [ nextLink binaryDo: aBlock ]


]
Methods Link

    reverseDo: aBlock
        (nextLink notNil)
            ifTrue: [ nextLink reverseDo: aBlock ].
        aBlock value: value


]
Methods Link

    size
        (nextLink notNil)
            ifTrue: [ ^ 1 + nextLink size]
            ifFalse: [ ^ 1 ]


]
Methods EmployeeDatabase
newEmployee
  | newNumber |
  newNumber <- (smalltalk getPrompt: 'id: ') asInteger.
  records at: newNumber put: EmployeeRecord new




]
Methods EmployeeDatabase
list: condition
  records do: [:x | (condition value: x) ifTrue: [ x print ]]


]
Methods EmployeeDatabase
new
  records <- Dictionary new


]
Methods EmployeeDatabase
upDate: idNumber
  (records at: idNumber) upDate


]
Methods EmployeeDatabase
removeEmployee: aNumber
  records removeKey: aNumber




]
Methods EmployeeDatabase
records
  ^ records


]
Methods Module
  printString
    ^('Module ', name)


]
Methods Module

  name: aString
    name <- aString


]
Methods Module

  setRoot: aString
    root <- aString


]
Methods Queen

    checkRow: testRow column: testColumn | columnDifference |
        columnDifference <- testColumn - column.
        (((row = testRow) or: 
            [ row + columnDifference = testRow]) or:
            [ row - columnDifference = testRow])
                ifTrue: [ ^ true ].
        ^ neighbor checkRow: testRow column: testColumn


]
Methods Queen

    advance
        (row = 8)
            ifTrue: [ (neighbor next) ifFalse: [ ^ false ].
                  row <- 0 ].
        row <- row + 1.
        ^ true


]
Methods Queen
    setColumn: aNumber neighbor: aQueen
        column <- aNumber.
        neighbor <- aQueen



]
Methods Queen

    next
        ^ (self advance) and: [ self testPosition ]


]
Methods Queen

    result
        ^ neighbor result addLast: row


]
Methods Queen

    testPosition
        [neighbor checkRow: row column: column]
            whileTrue: [ (self advance) ifFalse: [ ^ false ]].
        ^ true


]
Methods Queen

    first
        neighbor first.
        row <- 1.
        ^ self testPosition


]
Methods Workspace

show
  dialog show.


]
Methods Workspace

runSelectedText: text
  | selection |
  selection <- text at: 'SELECTEDTEXT'.
  (selection size > 0)
    ifTrue: [ 
      selection value print ]


]
Methods Workspace

processCommand: c
  ^ (commands includesKey: c) 
    ifTrue: [
      (commands at: c) value.
      -1
    ] 
    ifFalse: [ 
      -2
    ]


]
Methods Workspace
init
  | container text doit buttons |
  commands <- Dictionary new.
  commands at: 833 put: [ 
    self saveMethod: method on: class 
  ].
  text <- TextControl new; at: 'EXPAND' put: 'YES'; at: 'MULTILINE' put: 'YES'; at: 'TABSIZE' put: '4'.
  doit <- ButtonControl new: 'doit'; setActionCallback: [ :x | self runSelectedText: text ]. 
  buttons <- HBox new; addChild: doit.
  container <- VBox new; addChild: text; addChild: buttons.
  dialog <- Dialog new: container; at: 'SIZE' put: '400x200'; at: 'SHRINK' put: 'YES'; at: 'TITLE' put: 'Tumbleweed Workspace v0.1'.
  ^ self


]
Methods Point

    + v
        v isPoint
            ifTrue: [ ^ Point new; x: x + v x; y: y + v y ]
            ifFalse: [ ^ Point new; x: x + v; y: y + v]


]
Methods Point

    printString
        ^ x printString , '@', y printString


]
Methods Point

    y: v
        y <- v


]
Methods Point

    x
        ^ x


]
Methods Point

    isPoint
        ^ true


]
Methods Point
    <= aPoint
        ^ (x <= aPoint x) and: [y <= aPoint y]


]
Methods Point

    x: v
        x <- v


]
Methods Point

    * v
        ^ Point new; x: x * v; y: y * v


]
Methods Point

    size: aPoint
        ^ self to: self + aPoint


]
Methods Point

    y
        ^ y


]
Methods Point

    - v
        v isPoint
            ifTrue: [ ^ Point new; x: x - v x; y: y - v y ]
            ifFalse: [ ^ Point new; x: x - v; y: y - v]


]
Methods Point

    radius: n
        ^ Circle new; center: self; radius: n


]
Methods Point

    = aPoint
        aPoint isPoint
            ifTrue: [ ^ (x = aPoint x) and: [y = aPoint y] ]
            ifFalse: [ ^ false ]


]
Methods Point

    to: aPoint
        " return a rectangle with the given dimensions "
        ^ Rectangle new; upperLeft: self; bottomRight: aPoint


]
Methods FLTKWindow
handle: aCPointer
  handle <- aCPointer

]
Methods FLTKWindow
handle
  ^ handle

]
