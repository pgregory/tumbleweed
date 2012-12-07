*
*
* Little Smalltalk, version 2
* Written by Tim Budd, Oregon State University, July 1987
*
*
* New test framework based on the original Kent Beck paper
* 
* use:
*
*    TestRunner all
*

RawClass MetaTestCase Class MetaObject failedCheckSignal
Class TestCase Object selector
Class TestSuite Object name testCases
Class TestResult Object startTime stopTime testName failures errors
Class TestRunner Object
Class FailedCheckSignal Exception

Methods MetaTestCase 'creation'
        selector: aSymbol
        ^ self new setSelector: aSymbol
|
        doInit
          failedCheckSignal <- FailedCheckSignal new
|
        failedCheckSignal
          ^ failedCheckSignal
|
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

Methods TestCase 'all'
            setSelector: aSymbol
            selector <- aSymbol
| 
        selector
            ^ selector
]

Methods TestCase 'fixture'
        setUp
            "Run whatever code you need to get ready for the test to run."
            nil
|
        tearDown
            "Release whatever resources you used for the test."
            nil
]

Methods TestCase 'execution'
        run
            self setUp.
            self performTest.
            "Should really do valueNowOrOnUnwindDo: here"
            self tearDown
|
        run: aTestResult
            self setUp.
            [ self performTest ] on: FailedCheckSignal do: [:ex | aTestResult failure: (ex message) in: self].
            self tearDown
|
        performTest
            | args |
            ^ [self respondsTo: selector] ifTrue: 
                [   args <- Array new: 1.
                    args at: 1 put: self.
                    smalltalk perform: selector withArguments: args
                ] ifFalse: [ false ]
|
        should: aBlock
            self should: aBlock or: 'No message supplied'
|
        should: aBlock description: aString
            aBlock value ifFalse: [self failedCheckSignal signalWith: aString]
|
        shouldnt: aBlock
            self shouldnt: aBlock or: 'No message supplied'
|
        shouldnt: aBlock description: aString
            aBlock value ifTrue: [self failedCheckSignal signalWith: aString]
]

Methods MetaTestSuite 'creation'
        named: aString
            ^ self new setName: aString
]

Methods TestSuite 'creation'
        setName: aString
            name <- aString.
            testCases <- List new 
|
        addTestCase: aTestCase
            testCases add: aTestCase
|
        addTestCases: aCollection
            aCollection do: [:each | self addTestCase: each]
]

Methods TestSuite 'accessing'
    name
        ^ name
|
    testCases
        ^ testCases
]

Methods TestSuite 'run'
        run
            | result |
            result <- self defaultTestResult.
            result start.
            [ self run: result ] on: Exception do: [ :ex | result error: 'Error' in: self ].
            result stop.
            ^ result
|
        run: aTestResult
            testCases do: [:each | each run: aTestResult]
|
        defaultTestResult
            ^ self defaultTestResultClass test: self
|
        defaultTestResultClass
            ^ TestResult
]

Methods MetaTestResult 'creation'
        test: aTest
            ^ self new setTest: aTest
]

Methods TestResult 'creation'
        setTest: aTest
          testName <- aTest name.
          failures <- List new.
          errors <- List new
]

Methods TestResult 'run'
        start
            startTime <- Time seconds
|
        stop
            stopTime <- Time seconds
|
        run: aTestResult
            testCases do: [:each | each run: aTestResult]
]

Methods TestResult 'errors'
        makeRecord: aString in: aTestCase
            | record |
            record <- Array new: 2.
            record at: 1 put: aTestCase.
            record at: 2 put: aString.
            ^ record
|
        error: aString in: aTestCase
            errors add: (self makeRecord: aString in: aTestCase)
|
        failure: aString in: aTestCase
            failures add: (self makeRecord: aString in: aTestCase)
|
        errors
            ^ errors
|
        failures
            ^ failures
]


Methods MetaTestRunner 'running'
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

Class TestConversions TestCase 

Methods TestConversions 'tests'
    testConversions
        " test a few conversion routines "
        self should: [#abc == #abc asString asSymbol] description: '#abc == #abc asString asSymbol'.
        self should: [$A == $A asInteger asCharacter] description: '$A == $A asInteger asCharacter'.
        self should: [12 = 12 asDigit digitValue] description: '12 = 12 asDigit digitValue'.
        self should: [237 = 237 asString asInteger] description: '237 = 237 asString asInteger'.
        self should: [43 = 43 asFloat truncated] description: '43 = 43 asFloat truncated'.
        self should: [$A == ($A asString at: 1)] description: '$A == ($A asString at: 1)'
]
