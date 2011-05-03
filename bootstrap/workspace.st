Class Workspace Object dialog commands

Methods MetaWorkspace 'all'
new
  | newWorkspace |
  newWorkspace <- super new.
  newWorkspace init.
  ^ newWorkspace
|
open
  Workspace new; show.
]
*
Methods Workspace 'creation'
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
|
show
  dialog show.
|
runSelectedText: text
  | selection |
  selection <- text at: 'SELECTEDTEXT'.
  (selection size > 0)
    ifTrue: [ 
      selection value print ]
|
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

