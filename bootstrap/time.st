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
  millisecondClock
    ^ <7>
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
      ClockAtStartup <- <7>
    ]
|
  clockAtStartup
    ^ ClockAtStartup
]
  

