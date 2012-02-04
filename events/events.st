Class EventLoop Object
RawClass MetaEvent Class MetaObject evFFI
Class Event Object handle
Class TimerEvent Event
Class IOEvent Event

Methods MetaEvent 'initialisation'
  evFFI
    (evFFI isNil) ifTrue: [
      evFFI <- FFI new: 'modules/events/events'
    ].
    ^ evFFI
|
  new: aBlock
    | newobj |
    newobj <- super new.
    newobj init: aBlock.
    ^ newobj
]
Methods Event 'access'
  handle
    ^ handle
]
*
Methods MetaTimerEvent 'creation'
  new: aBlock
    ^ super new: aBlock
]
*
Methods TimerEvent 'creation'
  init: aBlock
    | args callback |
    callback <- <183 #void #(cObject cObject int) aBlock>.
    args <- Array new: 1.
    args at: 1 put: callback.
    handle <- ((Event evFFI) call: 'create_timer' returning: #cObject taking: #(cObject) as: args) at: 1
|
  start
    | args |
    args <- Array new: 1.
    args at: 1 put: handle.
    (Event evFFI) call: 'start_timer' returning: #void taking: #(cObject) as: args
|
  stop
    | args |
    args <- Array new: 1.
    args at: 1 put: handle.
    (Event evFFI) call: 'stop_timer' returning: #void taking: #(cObject) as: args
]
*
Methods MetaIOEvent 'creation'
  new: fd as: aBlock
    | neobj |
    newobj <- super new.
    newobj init: fd as: aBlock.
    ^ newobj
]
*
Methods IOEvent 'creation'
  init: fd as: aBlock
    | args callback |
    callback <- <183 #void #(cObject cObject int) aBlock>.
    args <- Array new: 2.
    args at: 1 put: callback; at: 2 put: fd.
    handle <- ((Event evFFI) call: 'create_io' returning: #cObject taking: #(cObject int) as: args) at: 1
|
  start
    | args |
    args <- Array new: 1.
    args at: 1 put: handle.
    (Event evFFI) call: 'start_io' returning: #void taking: #(cObject) as: args
|
  stop
    | args |
    args <- Array new: 1.
    args at: 1 put: handle.
    (Event evFFI) call: 'stop_io' returning: #void taking: #(cObject) as: args
]
*
Methods MetaEventLoop 'running'
  run: flags
    | args |
    args <- Array new: 1.
    args at: 1 put: flags.
    (Event evFFI) call: 'run' returning: #void taking: #(int) as: args    
]
*
Methods MetaEventLoop 'testing'
  test
    | event result client clientsock socket doit |
    socket <- Socket new.
    socket bindTo: 8090.
    socket listen.
    event <- IOEvent new: (socket socketID) as: [ :a :b :c | 
      a print.
      b print.
      c print.
      client <- socket accept.
      data <- ByteArray new: 255.
      request <- String new: 0.
      doit <- true.
      [ doit ] whileTrue: [
          result <- socket read: data.
          read <- result at: 1.
          ( read >= 0 ) ifTrue: [
              ( read < 255 ) ifTrue: [ 
                  "Make sure String size gets the right length"
                  data at: (read + 1) put: 0 
              ] ifFalse: [
                  "If less than 255 read, we're done"
                  doit <- false
              ].
              request <- request combineWith: (data asString) 
            ]
        ]
    ].
    event start.
    EventLoop run: 0
]

    
