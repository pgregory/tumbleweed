RawClass MetaModule Class MetaObject modules
Class Module Object name root

Methods MetaModule 'creation'
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
|
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
*
Methods MetaModule 'misc'
  modules
    ^ modules
]
*
Methods Module 'all'
  printString
    ^('Module ', name)
|
  name: aString
    name <- aString
|
  setRoot: aString
    root <- aString
]
