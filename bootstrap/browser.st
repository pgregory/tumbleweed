Class Panel Object browser widget
Class   NavigationPanel Panel classList
Class   ButtonPanel Panel buttons
Class   DefinitionPanel Panel text commands
Class Browser Object panels dialog currentClass currentMethod
Class AddSubClassDialog Object dialog parentClass nameControl ivnamesControl
*
Methods MetaPanel 'all'
  new: browser
    | newPanel |
    newPanel <- super new.
    newPanel init: browser.
    ^ newPanel
]
*
Methods Panel 'all'
  init: browser
    browser <- browser.
|
  widget
    ^ widget
]
*
Methods NavigationPanel 'all'
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
|
  listControl: name on: container
    | list vbox |
    list <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '0x50'.
    vbox <- VBox new.
    vbox addChild: (LabelControl new: (name asString)).
    vbox addChild: list.
    container addChild: vbox.
    ^ list
|
  updateProtocolList: list on: class
    | index |
    self clearList: list.
    browser clearMethod.
    index <- 1.
    (class protocols) do: [ :x |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  updateMethodList: list with: protocol on: class
    | index |
    self clearList: list.
    browser clearMethod.
    index <- 1.
    (class methodsFor: (protocol asString)) binaryDo: [ :x :y |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  updateClassList
    | index |
    " Initialise the class list "
    self clearList: classList.
    index <- 1.
    ((classes select: [ :x | x isMeta not ]) sort: [ :x :y | (x asString) < (y asString) ] ) do: [ :x |
      classList at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  clearList: list
    list at: 'REMOVEITEM' put: (cPointer new).
    ^ self
]
*
Methods ButtonPanel 'all'
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
Methods DefinitionPanel 'all'
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
|
  acceptButton
    | button |
    button <- ButtonControl new: 'Accept'; at: 'EXPAND' put: 'HORIZONTAL'.
    button setActionCallback: [ :x | self processCommand: 833 ]. 
    ^ button
|
  cancelButton
    | button |
    button <- ButtonControl new: 'Cancel'; at: 'EXPAND' put: 'HORIZONTAL'.
    button setActionCallback: [ :x | self processCommand: 835 ].
    ^ button
|
  buttons
    | hbox |
    hbox <- HBox new.
    #(subclass) do: [ :name |
      hbox addChild: ( ButtonControl new: (name asString); at: 'EXPAND' put: 'HORIZONTAL'; setActionCallback: [ :x | self processCommand: 837 ])
    ].
    ^ hbox
|
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
|
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
|
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
|
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
|
  cancelMethod: method on: class
    self showMethod: method on: class.
    ^ true
|
  subclassOn: class
    | dialog |
    dialog <- AddSubClassDialog new: class.
    dialog show. 
|
  showClass: class
    commands at: 837 put: [ 
      self subclassOn: class 
    ].
]
*
Methods MetaBrowser 'all'
  new
    | newObj |
    newObj <- super new.
    newObj panels: newObj panels.
    newObj init.
    ^ newObj
|
  open
    self openOnClass: nil
|
  openOnClass: class
    self openOnClass: class showing: nil
|
  openOnClass: class showing: method
    Browser new; show. IUP instance IupMainLoop
]
*
Methods Browser 'all'
  init
    | container |
    container <- Split with: ((panels at: 1) widget) and: ((panels at: 3) widget); at: 'ORIENTATION' put: 'HORIZONTAL'. 
    dialog <- Dialog new: container; at: 'SIZE' put: '400x200'; at: 'SHRINK' put: 'YES'; at: 'TITLE' put: 'Tumbleweed Browser v0.1'.
    ^ self
|
  panels
    ^ Array new: 3; at: 1 put: (self navigationPanel); 
                    at: 2 put: (self buttonPanel); 
                    at: 3 put: (self definitionPanel)
|
  panels: thePanels
    panels <- thePanels.
    ^self
|
  navigationPanel
    ^ NavigationPanel new: self
|
  buttonPanel
    ^ ButtonPanel new: self
|
  definitionPanel
    ^ DefinitionPanel new: self
|
  clearMethod
    | defPnl |
    defPnl <- panels at: 3.
    defPnl clearMethod
|
  showMethod: method on: class
    | defPnl |
    defPnl <- panels at: 3.
    defPnl showMethod: method on: class
|
  showClass: class
    | defPnl |
    defPnl <- panels at: 3.
    defPnl showClass: class
|
  updateClassList
    | navPnl |
    navPnl <- panels at: 1.
    navPnl updateClassList
|
  show
    dialog show.
]
*
Methods MetaAddSubClassDialog 'all'
  new: parent
    | newDialog |
    newDialog <- super new.
    newDialog init: parent.
    ^ newDialog
]

*
Methods AddSubClassDialog 'all'
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
|
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
|
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
|
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
|
  show
    dialog show.
]

