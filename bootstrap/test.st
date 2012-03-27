*
*
* Little Smalltalk, version 2
* Written by Tim Budd, Oregon State University, July 1987
*
*  a few test cases.
*
* to use, first file in this file, then pas all to an instance of
*   class Test, for example using the messages
*
*   File new; fileIn: 'test.st'
*   Test new all
*
Class Test Object
Class One Object
Class Two One
Class Three Two
Class Four Three

Methods One 'all'
        test
                ^ 1
|
    result1
                ^ self test
]
Methods Two 'all'
        test
                ^ 2
]
Methods Three 'all'
        result2
                ^ self result1
|
    result3
                ^ super test
]
Methods Four 'all'
        test
                ^ 4
]
Methods Test 'all'
    all
        self super.
        self conversions.
        self collections.
        self factorial.
        self filein.
        'all tests completed' print
|
    conversions
        " test a few conversion routines "
        ( (#abc == #abc asString asSymbol) and: [
        ($A == $A asInteger asCharacter) and: [
        (12 = 12 asDigit digitValue) and: [
        (237 = 237 asString asInteger) and: [
        (43 = 43 asFloat truncated) and: [
        $A == ($A asString at: 1) ] ] ] ] ] )
            ifFalse: [^ smalltalk error: 'conversion failure'].
        'conversion test passed' print.
|
    collections
        " test the collection classes a little"
        ( (#(1 2 3 3 2 4 2) asSet = #(1 2 3 4) asSet) and: [
        (#(1 5 3 2 4) sort asArray = #(1 2 3 4 5)) and: [
        (1 "(#+ respondsTo occurrencesOf: Float)" = 1) and: [
        ('First' < 'last') ] ] ] )
            ifFalse: [^smalltalk error: 'collection failure'].
        'collection test passed' print.
|
    factorial   | t |
        t <- [:x | (x = 1) ifTrue: [ 1 ] 
                ifFalse: [ x * (t value: x - 1) ] ].
        ((t value: 5) = 5 factorial)
            ifFalse: [ smalltalk error: 'factorial failure'].
        'factorial test passed' print
|
    filein
        File new; name: '../tumbleweed/bootstrap/queen.st'; open: 'r'; fileIn.
        "(globalNames includesKey: #Queen )
            ifFalse: [ smalltalk error: 'fileIn failure']."
        'file in test passed' print.
        self queen
|
    super2       | x1 x2 x3 x4 |
                x1 <- One new.
                x2 <- Two new.
                x3 <- Three new.
                x4 <- Four new.
        ^ List new; addLast: x1 test;
            addLast: x1 result1;
            addLast: x2 test;
            addLast: x2 result1;
            addLast: x3 test;
                    addLast: x4 result1;
                    addLast: x3 result2;
            addLast: x4 result2;
                    addLast: x3 result3;
                    addLast: x4 result3
|
    super
        (self super2 asArray = #(1 1 2 2 2 4 2 4 2 2) )
            ifTrue: ['super test passed' print]
            ifFalse: [ smalltalk error: 'super test failed']
]

*
* New test framework based on the original Kent Beck paper
*

RawClass MetaTestCase Class MetaObject 
Class TestCase Object selector
Class TestSuite Object name testCases
Class TestResult Object startTime stopTime Time testName failures errors
Class FailedCheckSignal Exception

Methods MetaTestCase 'creation'
            selector: aSymbol
            ^ self new setSelector: aSymbol
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
            [ 
                [ self performTest ] on: FailedCheckSignal do: [:ex | aTestResult failure: (ex message) in: self]
            ] on: Exception do: [:ex | aTestResult error: 'Error' in: self]
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
        should: aBlock or: aString
            aBlock value ifFalse: [FailedCheckSignal signal: aString]
|
        shouldnt: aBlock
            self shouldnt: aBlock or: 'No message supplied'
|
        shouldnt: aBlock or: aString
            aBlock value ifTrue: [FailedCheckSignal signal: aString]
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
        name
            ^ name
|
        addTestCase: aTestCase
            testCases add: aTestCase
|
        addTestCases: aCollection
            aCollection do: [:each | self addTestCase: each]
]

Methods TestSuite 'run'
        run
            | result |
            result <- self defaultTestResult.
            result start.
            self run: result.
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
            startTime <- <4>
|
        stop
            stopTime <- <4>
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

Class TestTestCase TestCase anumber

Methods TestTestCase 'testing'
        setUp
            anumber <- 5.
|
        testAdd
            self should: [anumber + 5 = 10] or: '5+5 should equal 10!'
|
        testFail
            self should: [anumber + 3 = 10] or: '5+3 should equal 10!'
]


Methods MetaTestTestCase 'all'
        testSuite1
            | suite |
            suite <- TestSuite named: 'TestTestCase'.
            suite addTestCase: (TestTestCase selector: #testAdd).
            suite addTestCase: (TestTestCase selector: #testFail).
            ^ suite
]
