*
* Little Smalltalk, version 3
* Written by Tim Budd, Oregon State University, July 1988
*
* Classes dealing with objects having Magnitude
*
RawClass MetaTime Class MetaMagnitude ClockAtStartup 
Class Time Magnitude
*
Methods MetaTime 'clocks'
  seconds
    ^ <seconds>
|
  millisecondClock
    ^ <milliseconds>
|
  millisecondsToRun: timedBlock
    | startTime |
    startTime <- self millisecondClock.
    timedBlock value.
    ^self millisecondClock - startTime
|
  doInit
    ObjectMemory addDependent: self
|
  update: aspect
    (aspect == #returnFromSnapshot) ifTrue: [
      ClockAtStartup <- <seconds>
    ]
|
  clockAtStartup
    ^ ClockAtStartup
]
*
Methods MetaTime 'triggers'
  every: seconds do: block
  | current |
  current <- Time millisecondClock.
  [true] whileTrue: [
    ((Time seconds - current) >= seconds) ifTrue: [
      current <- Time seconds.
      block value
    ].
    scheduler yield
  ]
]
  

