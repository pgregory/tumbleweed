Class SetTest TestCase
Class StringTest TestCase string emptyString subcollection3ElementsSorted nonEmpty5ElementsSorted unsortedCollection indexInNonEmptyArray arrayWithCharacters nonEmpty1element withoutEqualElements sameAtEndAndBegining elementInNonEmpty collectionNotIncluded notIn 
Class ArrayTest TestCase

Methods SetTest 'tests'
    testEliminateDuplicates
        " test the collection classes a little"
        self should: [#(1 2 3 3 2 4 2) asSet = #(1 2 3 4) asSet] description: '#(1 2 3 3 2 4 2) asSet = #(1 2 3 4) asSet'
]

Methods ArrayTest 'tests'
    testSort
        self should: [#(1 5 3 2 4) sort asArray = #(1 2 3 4 5)] description: '#(1 5 3 2 4) sort asArray = #(1 2 3 4 5)'
]

Methods StringTest 'requirements'
    empty
        ^emptyString
|
    nonEmpty
        ^nonEmpty5ElementsSorted
]

Methods StringTest 'initialisation'
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

Methods StringTest 'test - comparing'
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

Methods StringTest 'test - equality'
    testEqualSignIsTrueForEmptyButNonIdenticalCollections
        self should: [self empty = self empty copy] description: 'self empty = self empty copy'.
        self should: [self empty copy = self empty] description: 'self empty copy = self empty'.
        self should: [self empty copy = self empty copy] description: 'self empty copy = self empty copy'
|
    testEqualSignIsTrueForNonIdenticalButEqualCollections
        self should: [self nonEmpty = self nonEmpty copy] description: 'self nonEmpty = self nonEmpty copy'.
        self should: [self nonEmpty copy = self nonEmpty] description: 'self nonEmpty copy = self nonEmpty'.
        self should: [self nonEmpty copy = self nonEmpty copy] description: 'self nonEmpty copy = self nonEmpty copy'
|
    testEqualSignOfIdenticalCollectionObjects
        self should: [self empty = self empty] description: 'self empty = self empty'.
        self should: [self nonEmpty = self nonEmpty] description: 'self nonEmpty = self nonEmpty'
]

