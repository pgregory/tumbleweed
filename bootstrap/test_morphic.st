Class NodeTest TestCase nodeA nodeB

Methods NodeTest 'initialisation'
  setUp
    nodeA <- Node new: 'A'.
    nodeB <- Node new: 'B'
]
*
Methods NodeTest 'tests - hierarchy'
  testAddRemoveChild
    "Test adding a child in good and bad conditions"
    self should: [ nodeA addChild: nodeB. nodeB parent = nodeA ] description: 'addChild should set parent on child.'.
    self should: [ (nodeA children) size = 1 ] description: 'After adding a child, count should be one'.
    self should: [ [ nodeA removeChild: nodeA ] on: Exception do: [ :ex | false ]. true ] description: 'Should not except when removing valid child'.
    self should: [ (nodeA removeChild: nodeB) = nodeA ] description: 'Should not except when removing valid child'.
    self should: [ (nodeA children) size = 0 ] description: 'After removing a child, count should be zero'.
]
