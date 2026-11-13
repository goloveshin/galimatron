parser grammar g_parser;

options { tokenVocab = g_lexer; }

@members
{
    bool seenComment = false;
    bool seenHash = false;
}

//------------------------------------------------------------------------------

productionEntryPoint
  : chunk EOF;

testStringEntryPoint
  : STRING EOF;

//------------------------------------------------------------------------------
// Chunk
//------------------------------------------------------------------------------

// testing: yes (unit)
//
// wizard commands stand alone. a file that has aggregates has no wizard
chunk
  : wizardStatement+
  | content?
  ;

// testing: yes (unit)
//
content
  : contentRootAggregateOnly | contentAggregateOnly
  ;

// testing: yes (unit)
//
contentRootAggregateOnly
  : rootAggregate
    rootAggregateVersionDefinitions?
  ;

// testing: yes (unit)
//
contentAggregateOnly
  : aggregate
    aggregateVersionDefinitions?
  ;

//------------------------------------------------------------------------------
// Root Aggregate
//------------------------------------------------------------------------------

// testing: yes (unit)
//
rootAggregate
  : ROOT AGGREGATE name=IDENTIFIER
    LC
      rootAggregateVersionDeclarations
    RC
  ;

// testing: yes (unit)
//
rootAggregateVersionDeclarations
  : VERSIONS
    LC
      rootAggregateVersionDeclaration*
    RC
  ;

// testing: yes (unit)
//
rootAggregateVersionDeclaration
  : version_index=INTEGER (EQ rootAggregateVersionDeclarationHeader)? SEMI
  ;

rootAggregateVersionDeclarationHeader
  @init {
    seenComment = false;
    seenHash = false;
  }
  : version_status=aggregateVersionStatus?
    (
      LC
        rootAggregateVersionDeclarationHeaderBody? (COMMA rootAggregateVersionDeclarationHeaderBody)?
      RC
    )?
  ;

rootAggregateVersionDeclarationHeaderBody
  : {!seenComment}? versionComment {seenComment = true;}
    | {!seenHash}? versionHash {seenHash = true;}
  ;

// testing: yes (unit)
//
rootAggregateVersionDefinitions
  : rootAggregateVersionDefinition+
  ;

// a numbered body, or a parked body owned by the version named after before
//
// testing: yes (unit)
//
rootAggregateVersionDefinition
  : VERSION versionMode? (version_index=INTEGER | BEFORE owner_index=INTEGER)
    LC
      rootVersionDefinitionBody
    RC
  ;

//------------------------------------------------------------------------------
// Aggregate
//------------------------------------------------------------------------------

// testing: yes (unit)
//
aggregate
  : AGGREGATE name=IDENTIFIER
    LC
      aggregateVersionDeclarations
    RC
  ;

// testing: yes (unit)
//
aggregateVersionDeclarations
  : VERSIONS
    LC
      aggregateVersionDeclaration*
    RC
  ;

// testing: yes (unit)
//
aggregateVersionDeclaration
  : version_index=INTEGER (EQ aggregateVersionDeclarationHeader)? SEMI
  ;

aggregateVersionDeclarationHeader
  @init {
    seenComment = false;
    seenHash = false;
  }
  : version_status=aggregateVersionStatus?
    (
      LC
        aggregateVersionDeclarationHeaderBody? (COMMA aggregateVersionDeclarationHeaderBody)?
      RC
    )?
  ;

aggregateVersionDeclarationHeaderBody
  : {!seenComment}? versionComment {seenComment = true;}
    | {!seenHash}? versionHash {seenHash = true;}
  ;

// testing: yes (unit)
//
aggregateVersionDefinitions
  : aggregateVersionDefinition+
  ;

// a numbered body, or a parked body owned by the version named after before
//
// testing: yes (unit)
//
aggregateVersionDefinition
  : VERSION versionMode? (version_index=INTEGER | BEFORE owner_index=INTEGER)
    LC
      versionDefinitionBody
    RC
  ;

aggregateVersionStatus
  : DELETED
  ;

//------------------------------------------------------------------------------
// Aggregate's common
//------------------------------------------------------------------------------

// testing: yes (unit)
//
rootVersionDefinitionBody
  : (tagStatement | labelStatement | orderingStatement | operationStatement)*
  ;

// testing: yes (unit)
//
versionDefinitionBody
  : (tagStatement | labelStatement | operationStatement)*
  ;

// testing: yes (unit)
//
versionHash
  : HASH EQ version_hash=STRING
  ;

versionComment
  : COMMENT EQ version_comment=STRING
  ;

//------------------------------------------------------------------------------
// Tag
//------------------------------------------------------------------------------

// testing: yes (unit)
//
tagMode
  : ONCE
  | LOCAL
  ;

// once drops earlier versions of this aggregate. local also drops the root version.
// own keeps earlier versions of this aggregate and drops the root version.
// local and own on the root are errors
//
// testing: yes (unit)
//
versionMode
  : ONCE
  | LOCAL
  | OWN
  ;

// testing: yes (unit)
//
namedTag
  : name=IDENTIFIER EQ tagMode? tagLabels tagValues
  ;

// testing: yes (unit)
//
tagDefinition
  : namedTag
  | tagMode? tagLabels tagValues
  ;

// testing: yes (unit)
//
tagStatement
  : tagDefinition SEMI
  ;

// testing: yes (unit)
//
tagLabels
  : IDENTIFIER*
  ;

// testing: yes (unit)
//
tagValues
  // essentially this is tagJSONArray, only through LC/RC
  : LC RC
  | LC
      tagJSONValue (COMMA tagJSONValue)*
    RC
  ;

// testing: yes (unit)
//
tagJSONValue
  : tagJSONString
  | tagJSONRecord
  | tagJSONArray
  ;

// testing: yes (unit)
//
tagJSONRecord
  : LC
      tagJSONKeyValue (COMMA tagJSONKeyValue)*
    RC
  ;

// testing: yes (unit)
//
tagJSONArray
  : LS RS
  | LS
      tagJSONValue (COMMA tagJSONValue)*
    RS
  ;

// testing: yes (unit)
//
tagJSONKeyValue
  : (key=STRING COL)? tagJSONValue
  ;

// testing: yes (unit)
//
tagJSONString
  : string=STRING TEMPLATE_ENGINE?
  ;

//------------------------------------------------------------------------------
// Label
//------------------------------------------------------------------------------

// testing: yes (unit)
//
labelDefinition
  : name=IDENTIFIER EQ (labelStatic | labelDynamic)
  ;

// testing: yes (unit)
//
labelStatement
  : LABEL labelDefinition SEMI
  ;

// testing: yes (unit)
//
labelStatic
  : STATIC
    (
      LC
        static_values+=IDENTIFIER (COMMA static_values+=IDENTIFIER)*
      RC
      (
        COMMA
        TAG_NAME_FROM
          LC
            tag_name_from=IDENTIFIER
          RC
      )?
    )?
  ;

// testing: yes (unit)
//
labelDynamic
  : DYNAMIC
    LC
      value=IDENTIFIER
    RC
  ;

//------------------------------------------------------------------------------
// Ordering
//------------------------------------------------------------------------------

// testing: yes (unit)
//
orderingDefinition
  : name=IDENTIFIER EQ orderingPart (COMMA orderingPart)*
  ;

// testing: yes (unit)
//
orderingStatement
  : ORDERING orderingDefinition SEMI
  ;

// testing: yes (unit)
//
orderingPart
  : LC
      IDENTIFIER COMMA IDENTIFIER (COMMA IDENTIFIER)*
    RC
  ;

//------------------------------------------------------------------------------
// Operation
//------------------------------------------------------------------------------

// testing: yes (unit)
//
operationStatement
  : versionDeleteStatement
  ;

// testing: yes (unit)
//
tagName
  : name=IDENTIFIER
  ;

// testing: yes (unit)
//
versionDeleteStatement
  : DELETE tagName? SEMI
  ;

//------------------------------------------------------------------------------
// Wizard
//------------------------------------------------------------------------------

// testing: yes (unit)
//
wizardStatement
  : WIZARD wizardCommand SEMI            # wizardSingle
  | WIZARD LC (wizardCommand SEMI)+ RC   # wizardBatch
  ;

wizardCommand
  : wizardCommandCreateRootAggregate
  | wizardCommandCreateAggregate
  | wizardCommandCopyAggregate
  | wizardCommandContinueAggregate
  | wizardCommandDeleteAggregate
  | wizardCommandCreateVersion
  | wizardCommandDeleteVersion
  | wizardCommandCollapseVersion
  | wizardCommandCreateHash
  | wizardCommandDeleteHash
  | wizardCommandCreateLabel
  | wizardCommandCreateTag
  | wizardCommandDeleteTag
  | wizardCommandCreateOrdering
  ;

// testing: yes (unit)
//
wizardCommandCreateRootAggregate
  : CREATE ROOT AGGREGATE name=IDENTIFIER
  ;

// in names the directory under the command file.
// create aggregate item in payload writes payload/item.gal
//
// testing: yes (unit)
//
wizardCommandCreateAggregate
  : CREATE AGGREGATE versionMode? names+=IDENTIFIER (COMMA names+=IDENTIFIER)* (IN directory=IDENTIFIER)?
  ;

// one source, then one or more new names.
// in names the directory under the command file.
// copy aggregate ping to pong in payload writes payload/pong.gal
//
// testing: yes (unit)
//
wizardCommandCopyAggregate
  : COPY AGGREGATE source=IDENTIFIER TO names+=IDENTIFIER (COMMA names+=IDENTIFIER)* (IN directory=IDENTIFIER)?
  ;

// one source, then one or more new names.
// the source ends in a new version. each new name continues there.
// in names the directory under the command file.
//
// testing: yes (unit)
//
wizardCommandContinueAggregate
  : CONTINUE AGGREGATE source=IDENTIFIER TO names+=IDENTIFIER (COMMA names+=IDENTIFIER)* (IN directory=IDENTIFIER)?
  ;

// no name removes every aggregate whose versions are all deleted.
// physically removes the named chunks. the no-name form has no mode
//
// testing: yes (unit)
//
wizardCommandDeleteAggregate
  : DELETE AGGREGATE (names+=IDENTIFIER (COMMA names+=IDENTIFIER)* (physically=PHYSICALLY)?)?
  ;

// testing: yes (unit)
//
wizardCommandCreateVersion
  : CREATE VERSION versionMode?
  ;

// no version means the last version
//
// testing: yes (unit)
//
wizardCommandDeleteVersion
  : DELETE VERSION (version_indexes+=INTEGER (COMMA version_indexes+=INTEGER)*)?
  ;

// one inclusive range. the version after the range owns the anonymous body
//
// testing: yes (unit)
//
wizardCommandCollapseVersion
  : COLLAPSE VERSION first=INTEGER MINUS last=INTEGER
  ;

// no version means the last version.
// force stores the hash as the important tags are now.
// the validation before the wizard does not check that version's stored hash
//
// testing: yes (unit)
//
wizardCommandCreateHash
  : CREATE HASH (version_indexes+=INTEGER (COMMA version_indexes+=INTEGER)*)? (force=FORCE)?
  ;

// no version means the last version.
// force removes the hash when the stored hash does not match.
// the validation before the wizard does not check that version's stored hash
//
// testing: yes (unit)
//
wizardCommandDeleteHash
  : DELETE HASH (version_indexes+=INTEGER (COMMA version_indexes+=INTEGER)*)? (force=FORCE)?
  ;

// testing: yes (unit)
//
wizardCommandCreateLabel
  : CREATE LABEL
    (
      aggregate_name=IDENTIFIER labelDefinition
      | labelDefinition
    )
  ;

// testing: yes (unit)
//
wizardCommandCreateTag
  : CREATE TAG
    (
      aggregate_name=IDENTIFIER namedTag
      | aggregate_name=IDENTIFIER unnamedOnAggregate
      | tagDefinition
    )
  ;

// an aggregate and an unnamed tag. one label is enough, and tag-name-from can name it
//   dictionary entry { "uuid", "0", "stable id of a message" }
// more labels stay an unnamed tag
//   hello field important { "std::string", "message" }
//
// testing: yes (unit)
//
unnamedOnAggregate
  : tagMode? first=IDENTIFIER rest+=IDENTIFIER* tagValues
  ;

// testing: yes (unit)
//
wizardCommandDeleteTag
  : DELETE TAG
    (
      aggregate_name=IDENTIFIER tagName
      | tagName
    )
  ;

// testing: yes (unit)
//
wizardCommandCreateOrdering
  : CREATE ORDERING
    (
      aggregate_name=IDENTIFIER orderingDefinition
      | orderingDefinition
    )
  ;
