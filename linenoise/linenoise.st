Class LineNoise Object lnFFI

Methods MetaLineNoise 'misc'
  loadFFI
    (lnFFI isNil) ifTrue: [ lnFFI <- FFI new: 'modules/linenoise/linenoise' ].
|
  getPrompt: aString
    | args |
    self loadFFI.
    args <- Array new: 1.
    args at: 1 put: aString.
    ^ (lnFFI call: 'linenoise' returning: #string taking: #(string) as: args) at: 1
|
  addHistory: aString
    | args |
    self loadFFI.
    args <- Array new: 1.
    args at: 1 put: aString.
    ^ (lnFFI call: 'linenoiseHistoryAdd' returning: #int taking: #(string) as: args) at: 1
]
